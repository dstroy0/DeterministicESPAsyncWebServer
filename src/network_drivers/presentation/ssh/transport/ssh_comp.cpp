// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_comp.cpp
 * @brief SSH server-to-client compression owner - per-connection state + activation.
 */

#include "network_drivers/presentation/ssh/transport/ssh_comp.h"

#if DETWS_ENABLE_SSH_ZLIB

#include "network_drivers/presentation/ssh/transport/ssh_zlib.h"
#include <string.h>

// The per-connection compressor holds a window-sized work buffer + a window-sized hash chain (tens
// of KB); the pool does not fit internal DRAM alongside the SSH crypto stack, so it lives in PSRAM
// (DETWS_SSH_ZLIB_IN_PSRAM). Same mechanism/caveat as the TLS arena and HTTP/2 pool: it needs a
// framework built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y (the stock arduino-esp32 core
// ships it OFF, so EXT_RAM_BSS_ATTR would silently no-op); see tools/psram/README.md.
#if DETWS_SSH_ZLIB_IN_PSRAM && defined(ARDUINO)
#include <esp_attr.h> // pulls in sdkconfig.h -> CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY
#if !defined(CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY)
#error                                                                                                                 \
    "DETWS_SSH_ZLIB_IN_PSRAM needs a framework built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y. The stock arduino-esp32 core ships it OFF, so EXT_RAM_BSS_ATTR silently no-ops and the compressor pool would overflow internal DRAM. Rebuild the core (tools/psram/README.md) or unset DETWS_SSH_ZLIB_IN_PSRAM."
#endif
#if defined(EXT_RAM_BSS_ATTR)
#define DETWS_SSH_COMP_ATTR EXT_RAM_BSS_ATTR // IDF v5 / arduino-esp32 3.x
#elif defined(EXT_RAM_ATTR)
#define DETWS_SSH_COMP_ATTR EXT_RAM_ATTR // IDF v4 / arduino-esp32 2.x
#else
#define DETWS_SSH_COMP_ATTR
#endif
#else
#define DETWS_SSH_COMP_ATTR
#endif

// Per-connection compression state (large buffers -> PSRAM; the flags are trivial).
struct SshCompState
{
    SshDeflate z;                      ///< streaming compressor (bound to the buffers below).
    uint8_t work[SSH_ZLIB_WORK_SIZE];  ///< history + input work buffer.
    uint16_t head[SSH_ZLIB_HASH_SIZE]; ///< hash bucket heads.
    uint16_t prev[SSH_ZLIB_WORK_SIZE]; ///< hash chain (absolute-position indexed).
    uint16_t ll_code[288];             ///< fixed lit/length Huffman codes.
    uint8_t ll_len[288];               ///< their bit lengths.
    uint16_t d_code[30];               ///< fixed distance Huffman codes.
    uint8_t d_len[30];                 ///< their bit lengths.
    uint8_t s2c_alg;                   ///< negotiated ::SshCompAlg.
    bool s2c_active;                   ///< true once the stream has started.
};

// All SSH compression state, owned by one instance (internal linkage): the per-connection
// deflate stream table. One named owner, unreachable from any other translation unit.
struct SshCompCtx
{
    SshCompState comp[MAX_SSH_CONNS];
};
static DETWS_SSH_COMP_ATTR SshCompCtx s_ssh_comp;

static void start_stream(SshCompState *c)
{
    ssh_deflate_init(&c->z, c->work, c->head, c->prev, c->ll_code, c->ll_len, c->d_code, c->d_len);
    c->s2c_active = true;
}

void ssh_comp_reset(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    s_ssh_comp.comp[i].s2c_alg = SSH_COMP_NONE;
    s_ssh_comp.comp[i].s2c_active = false;
}

void ssh_comp_set_s2c(uint8_t i, uint8_t alg)
{
    if (i >= MAX_SSH_CONNS)
        return;
    s_ssh_comp.comp[i].s2c_alg = alg;
}

void ssh_comp_on_newkeys(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    SshCompState *c = &s_ssh_comp.comp[i];
    if (c->s2c_alg == SSH_COMP_ZLIB && !c->s2c_active)
        start_stream(c);
}

void ssh_comp_on_auth_success(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    SshCompState *c = &s_ssh_comp.comp[i];
    if (c->s2c_alg == SSH_COMP_ZLIB_DELAYED && !c->s2c_active)
        start_stream(c);
}

bool ssh_comp_s2c_active(uint8_t i)
{
    return i < MAX_SSH_CONNS && s_ssh_comp.comp[i].s2c_active;
}

int ssh_comp_s2c(uint8_t i, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_cap, size_t *out_len)
{
    if (i >= MAX_SSH_CONNS || !s_ssh_comp.comp[i].s2c_active)
        return -1;
    return ssh_deflate_packet(&s_ssh_comp.comp[i].z, src, src_len, dst, dst_cap, out_len);
}

#endif // DETWS_ENABLE_SSH_ZLIB
