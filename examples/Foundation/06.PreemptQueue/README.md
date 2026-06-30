# 06.PreemptQueue - real-time ingest with a preempting task queue

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_PREEMPT_QUEUE`

## What this example teaches

Hardware events (a DMA-complete interrupt, a GPIO edge, a bus byte) arrive in ISR
context, where you must do as little as possible. The **preempting work queue**
is the clean hand-off: the ISR posts a small fixed-size item and the scheduler
**preempts straight to a dedicated high-priority task** that does the real work -
so the heavy processing runs off the interrupt, immediately, not on the next loop
tick.

It is one queue feeding one core-pinned task, with static (zero-heap) storage:

```cpp
DetwsPqConfig cfg = {};
cfg.handler  = on_reading; // runs in the high-priority task, once per item
cfg.priority = 6;          // above loop(): a post preempts into the handler
cfg.core     = 1;          // pin the task
detws_pq_start(&cfg);
```

**Post from an ISR** - interrupt-safe, asks for an immediate context switch:

```cpp
void IRAM_ATTR sample_isr() {
    Reading r{ esp_timer_get_time(), g_seq++ };
    detws_pq_post_from_isr(&r);   // xQueueSendFromISR + portYIELD_FROM_ISR
}
```

**Post from a task** - back, urgent (front), each with a wait timeout, all
fail-closed (they return `false` and drop rather than block forever) when the
queue is full:

```cpp
detws_pq_post(&item, 0);          // to the back
detws_pq_post_urgent(&item, 0);   // jump the queue
```

On hardware the high-priority task drains the queue automatically. Measured on a
DevKitV1: an ISR post reaches the handler in ~12 us, and the queue never backs up
past one item (each post is processed before the next arrives).

**Sizing.** `DETWS_PQ_DEPTH` (items), `DETWS_PQ_ITEM_SIZE` (bytes per item), and
`DETWS_PQ_STACK` (task stack) are compile-time because the storage is static;
`detws_pq_high_water()` reports the peak depth so you can size `DETWS_PQ_DEPTH`.

**Build-flag note.** `DETWS_ENABLE_PREEMPT_QUEUE` must be set for the whole build;
an in-sketch `#define` does not reach the separately compiled library, so pass it
in `platformio.ini` (`build_flags = -DDETWS_ENABLE_PREEMPT_QUEUE=1`).
