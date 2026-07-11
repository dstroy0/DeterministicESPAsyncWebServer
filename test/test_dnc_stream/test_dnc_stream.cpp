// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DNC drip-feed engine (services/dnc/dnc_stream): stream a G-code program over a
// send/recv seam and verify it round-trips through a scripted mock controller that decodes the wire
// bytes back to lines (ISO + EIA tape codes), that XON/XOFF flow control pauses and resumes the
// feed, and that an untranslatable line fails closed.

#include "services/dnc/dnc.h"
#include "services/dnc/dnc_stream.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A mock controller: mock_send decodes the streamed bytes back into lines + program markers;
// mock_recv is the reverse channel, optionally injecting an XOFF (then XON) once the feed passes a
// byte threshold, to exercise the pause/resume path.
struct MockCtrl
{
    DncDecoder dec;
    char lines[16][DETWS_DNC_LINE_MAX + 1];
    int nlines;
    int prog_start, prog_end;
    size_t bytes_sent;
    // reverse-channel scripting
    bool xoff_pending;
    size_t xoff_threshold;
    int xon_countdown;
    bool paused_seen;
    bool fail_send;
};

static void mock_init(MockCtrl *m, DncCode code)
{
    memset(m, 0, sizeof(*m));
    dnc_decode_init(&m->dec, code);
}

static int mock_send(void *c, const uint8_t *d, size_t len)
{
    MockCtrl *m = (MockCtrl *)c;
    if (m->fail_send)
        return -1;
    m->bytes_sent += len;
    for (size_t k = 0; k < len; k++)
    {
        DncEvent ev = dnc_decode_feed(&m->dec, d[k]);
        if (ev == DncEvent::DNC_EV_LINE && m->nlines < 16)
        {
            strcpy(m->lines[m->nlines], m->dec.line);
            m->nlines++;
        }
        else if (ev == DncEvent::DNC_EV_PROG_START)
            m->prog_start++;
        else if (ev == DncEvent::DNC_EV_PROG_END)
            m->prog_end++;
    }
    return (int)len;
}

static int mock_recv(void *c, uint8_t *buf, size_t cap)
{
    MockCtrl *m = (MockCtrl *)c;
    if (cap == 0)
        return 0;
    if (m->xoff_pending && m->bytes_sent >= m->xoff_threshold)
    {
        m->xoff_pending = false;
        m->paused_seen = true;
        m->xon_countdown = 3;
        buf[0] = (uint8_t)DncFlowByte::DNC_XOFF;
        return 1;
    }
    if (m->xon_countdown > 0)
    {
        if (--m->xon_countdown == 0)
        {
            buf[0] = (uint8_t)DncFlowByte::DNC_XON;
            return 1;
        }
        return 0; // an empty poll tick while "paused"
    }
    return 0;
}

static DncCfg iso_cfg()
{
    DncCfg c;
    memset(&c, 0, sizeof(c));
    c.code = DncCode::DNC_CODE_ISO;
    return c;
}

void test_iso_roundtrip()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    DncCfg cfg = iso_cfg();
    const char *prog = "N10 G0 X1 Y2\nN20 G1 X3 F100\nM30";

    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK,
                          dnc_stream(&cfg, prog, strlen(prog), mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(1, m.prog_start);
    TEST_ASSERT_EQUAL_INT(1, m.prog_end);
    TEST_ASSERT_EQUAL_INT(3, m.nlines);
    TEST_ASSERT_EQUAL_STRING("N10 G0 X1 Y2", m.lines[0]);
    TEST_ASSERT_EQUAL_STRING("N20 G1 X3 F100", m.lines[1]);
    TEST_ASSERT_EQUAL_STRING("M30", m.lines[2]);
}

void test_eia_roundtrip()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_EIA);
    DncCfg cfg = iso_cfg();
    cfg.code = DncCode::DNC_CODE_EIA;
    const char *prog = "N10 G0 X1\nN20 M30"; // uppercase + digits only (EIA has no lowercase)

    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK,
                          dnc_stream(&cfg, prog, strlen(prog), mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(1, m.prog_start);
    TEST_ASSERT_EQUAL_INT(1, m.prog_end);
    TEST_ASSERT_EQUAL_INT(2, m.nlines);
    TEST_ASSERT_EQUAL_STRING("N10 G0 X1", m.lines[0]);
    TEST_ASSERT_EQUAL_STRING("N20 M30", m.lines[1]);
}

void test_crlf_and_parity()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    DncCfg cfg = iso_cfg();
    cfg.crlf = true;
    cfg.even_parity = true;
    const char *prog = "G90\nG0 X0"; // decoder strips parity + the CR

    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK,
                          dnc_stream(&cfg, prog, strlen(prog), mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(2, m.nlines);
    TEST_ASSERT_EQUAL_STRING("G90", m.lines[0]);
    TEST_ASSERT_EQUAL_STRING("G0 X0", m.lines[1]);
}

void test_xoff_pacing()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    m.xoff_pending = true;
    m.xoff_threshold = 8; // pause partway through
    DncCfg cfg = iso_cfg();
    const char *prog = "N10 G0 X1\nN20 G1 X2\nN30 M30";

    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK,
                          dnc_stream(&cfg, prog, strlen(prog), mock_send, mock_recv, &m));
    TEST_ASSERT_TRUE(m.paused_seen); // the engine actually paused on XOFF
    TEST_ASSERT_EQUAL_INT(3, m.nlines);
    TEST_ASSERT_EQUAL_STRING("N30 M30", m.lines[2]); // and resumed to completion
}

void test_leader_trailer()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    DncCfg cfg = iso_cfg();
    cfg.leader_len = 8;
    const char *prog = "M30";

    size_t plen = strlen(prog);
    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK, dnc_stream(&cfg, prog, plen, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(1, m.nlines);
    TEST_ASSERT_EQUAL_STRING("M30", m.lines[0]);
    // 8 leader + 8 trailer NUL runout bytes were emitted (skipped by the decoder)
    TEST_ASSERT_GREATER_OR_EQUAL(16u, m.bytes_sent);
}

void test_empty_program()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    DncCfg cfg = iso_cfg();
    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_OK, dnc_stream(&cfg, "", 0, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(1, m.prog_start);
    TEST_ASSERT_EQUAL_INT(1, m.prog_end);
    TEST_ASSERT_EQUAL_INT(0, m.nlines);
}

void test_encode_error()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_EIA);
    DncCfg cfg = iso_cfg();
    cfg.code = DncCode::DNC_CODE_EIA;
    const char *prog = "N10 x1"; // lowercase 'x' has no EIA representation -> fail closed
    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_ERR_ENCODE,
                          dnc_stream(&cfg, prog, strlen(prog), mock_send, mock_recv, &m));
}

void test_io_error_and_args()
{
    MockCtrl m;
    mock_init(&m, DncCode::DNC_CODE_ISO);
    m.fail_send = true;
    DncCfg cfg = iso_cfg();
    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_ERR_IO, dnc_stream(&cfg, "M30", 3, mock_send, mock_recv, &m));

    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_ERR_ARG, dnc_stream(nullptr, "M30", 3, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_INT(DncStreamResult::DNC_STREAM_ERR_ARG, dnc_stream(&cfg, nullptr, 3, mock_send, mock_recv, &m));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_iso_roundtrip);
    RUN_TEST(test_eia_roundtrip);
    RUN_TEST(test_crlf_and_parity);
    RUN_TEST(test_xoff_pacing);
    RUN_TEST(test_leader_trailer);
    RUN_TEST(test_empty_program);
    RUN_TEST(test_encode_error);
    RUN_TEST(test_io_error_and_args);
    return UNITY_END();
}
