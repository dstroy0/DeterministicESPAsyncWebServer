// 09.InterfaceForward - bridge frames between interfaces, DMA-driven.
//
// The full v5 ingest pipeline end to end: a frame arrives on interface A (a DMA channel
// RX completion), the DMA-complete callback posts it onto the internal FORWARD lane of
// the preempting queue, that lane's task calls the forwarding plane, and the plane applies
// the rules and hands the bytes to interface B's egress. So the device BRIDGES traffic
// between its interfaces instead of only terminating it.
//
//   interface A --DMA RX--> callback --post--> FORWARD lane --> det_forward_ingress()
//                                                                     |
//                                                    (rule A->B allow, rate-capped)
//                                                                     |
//                                                          interface B egress send
//
// The forwarding plane is default-deny and fail-closed. Here interface A is a DMA channel
// fed by the simulator (no wire needed) and interface B's "egress" just counts the bytes;
// a real build would send B out Wi-Fi / Ethernet / a bus (or another DMA channel).
//
// Build flags (whole build):
//   DETWS_ENABLE_DMA=1 DETWS_ENABLE_PREEMPT_QUEUE=1 DETWS_ENABLE_FORWARD=1 DETWS_DMA_SIMULATE=1

#include "DeterministicESPAsyncWebServer.h" // discovers the library (adds src/ to the include path)
#include "services/dma/det_dma.h"
#include "services/forward/det_forward.h"
#include "services/preempt_queue/preempt_queue.h"
#include <Arduino.h>
#include <string.h>

static const uint8_t IF_A = 0; // ingress interface (DMA channel 0)
static const uint8_t IF_B = 1; // egress interface

static uint32_t g_out_frames = 0; // frames sent out interface B
static uint32_t g_out_bytes = 0;

// Interface B egress: a real build would send on Wi-Fi / Ethernet / a bus / a DMA channel.
static bool if_b_send(uint8_t, const uint8_t *data, uint16_t len, void *)
{
    g_out_frames++;
    g_out_bytes += len;
    Serial.printf("  -> IF_B egress %u bytes: %02X %02X ...\n", len, data[0], len > 1 ? data[1] : 0);
    return true;
}

// FORWARD-lane item: a self-contained copy of the frame (see 07.DmaIngest for why we copy).
struct fwd_msg
{
    uint16_t len;
    uint8_t src;
    uint8_t bytes[16];
};
union pq_item {
    fwd_msg msg;
    uint8_t raw[DETWS_PQ_ITEM_SIZE];
};

// Runs on the FORWARD lane (high priority): drive the forwarding plane off the "ISR".
static void on_forward(const void *item, void *)
{
    const fwd_msg *m = &((const pq_item *)item)->msg;
    det_forward_ingress(m->src, m->bytes, m->len);
}

// DMA-complete on interface A: copy the frame and post it onto the FORWARD lane.
static void on_dma_complete(const det_dma_event *ev, void *)
{
    if (ev->dir != DET_DMA_RX)
        return;
    pq_item it = {};
    it.msg.src = IF_A;
    it.msg.len = ev->len;
    uint16_t n = (ev->len < sizeof(it.msg.bytes)) ? ev->len : sizeof(it.msg.bytes);
    memcpy(it.msg.bytes, ev->data, n);
    detws_pq_post_lane_from_isr(DETWS_PQ_LANE_FORWARD, &it);
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    // FORWARD lane task (internal, high priority): runs the forwarding plane.
    DetwsPqConfig fwd = {};
    fwd.handler = on_forward;
    fwd.priority = 0; // 0 -> the FORWARD lane default (above the user lane)
    fwd.core = 1;
    fwd.name = "forward";
    detws_pq_start_lane(DETWS_PQ_LANE_FORWARD, &fwd);

    // Interface A ingress: a DMA channel fed by the simulator.
    det_dma_config a = {};
    a.channel = IF_A;
    a.periph = DET_DMA_UART;
    a.on_complete = on_dma_complete;
    det_dma_open(&a);

    // Forwarding rule: A -> B allowed (default-deny otherwise), no rate cap.
    det_forward_reset();
    det_forward_add_if(IF_B, DET_IF_WIFI_STA, if_b_send, nullptr);
    det_forward_add_rule(IF_A, IF_B, DET_FWD_ALLOW, 0);

    // Ingress ACL: drop frames whose first byte is 0xFF (a "bad" marker) before forwarding.
    uint8_t bad_pat[1] = {0xFF}, bad_mask[1] = {0xFF};
    det_forward_acl_add(IF_A, 0, bad_pat, bad_mask, 1, DET_FWD_DENY);

    Serial.println("forwarding: IF_A (DMA) -> FORWARD lane -> ACL + plane -> IF_B egress");
}

static uint8_t g_seq = 0;

void loop()
{
    // A frame arrives on interface A. det_dma_poll() completes the RX, which fires the
    // callback -> FORWARD lane -> forwarding plane -> IF_B egress. Every 5th frame is a
    // "bad" one (first byte 0xFF) that the ingress ACL should drop.
    uint8_t frame[8] = {0xBB, g_seq, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    if ((g_seq % 5) == 4)
        frame[0] = 0xFF;
    det_dma_sim_feed(IF_A, frame, sizeof(frame));
    det_dma_poll();
    g_seq++;

    if ((g_seq & 0x07) == 0)
    {
        det_forward_stats st;
        det_forward_get_stats(&st);
        Serial.printf("stats: in=%lu forwarded=%lu acl_denied=%lu (IF_B frames=%lu)\n", (unsigned long)st.frames_in,
                      (unsigned long)st.forwarded, (unsigned long)st.acl_denied, (unsigned long)g_out_frames);
    }
    delay(1000);
}
