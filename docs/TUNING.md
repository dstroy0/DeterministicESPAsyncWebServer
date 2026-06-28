# Performance tuning

How to size the worker model for a given workload without giving up the library's
determinism guarantees (no heap after `begin()`, fixed buffers, bounded latency).
Every knob here is compile-time; the default build is the tested-deterministic
path, so you only change these when you have a specific reason.

## The execution model in one paragraph

The server runs in one or more dedicated FreeRTOS worker tasks, not the user's
`loop()`. Each worker owns a disjoint partition of connection slots (slot `i` ->
worker `i % DETWS_WORKER_COUNT`) plus its own event queue and scratch arena, so no
two workers ever touch the same state: there are no hot-path locks, which is what
keeps latency bounded (= deterministic) while cores run disjoint connections in
parallel. A worker blocks on its FreeRTOS task notification and is woken the moment
an event or a deferred callback is queued, so event latency is independent of the
idle-sweep cadence. `DETWS_WORKER_COUNT == 1` (the default) is byte-for-byte the
original single-pipeline behavior. The lwIP callbacks that run on the tcpip thread
(shared with WiFi on Core 0) are kept minimal: a received segment is bulk-copied
into the slot's ring with a single SPSC publish, then one event is posted.

## Knobs

| Macro                        | Default | What it does                                                                                                                                             |
| ---------------------------- | ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `DETWS_WORKER_COUNT`         | 1       | Number of worker tasks. `> 1` partitions slots across cores. Must be `<= MAX_CONNS`.                                                                     |
| `DETWS_WORKER_CORE`          | 1       | Core that worker 0 pins to; worker `k` pins to `(DETWS_WORKER_CORE + k) % cores`.                                                                        |
| `DETWS_WORKER_TASK_PRIORITY` | 5       | FreeRTOS priority of the worker task(s).                                                                                                                 |
| `DETWS_WORKER_TASK_STACK`    | 8192    | Per-worker task stack (bytes).                                                                                                                           |
| `DETWS_WORKER_POLL_TICKS`    | 1       | Idle-sweep block timeout (ticks). Events wake the worker immediately regardless; this only sets how often an idle worker wakes to run the timeout sweep. |
| `EVT_QUEUE_DEPTH`            | 16      | Per-queue event slots. Raise to absorb larger connection bursts.                                                                                         |
| `MAX_CONNS`                  | (build) | Connection pool size. The hard ceiling on concurrent connections.                                                                                        |

## Measured behavior (ESP32, esp32dev, COM3)

**Event latency is decoupled from the idle-sweep cadence.** With WiFi power-save
off to isolate scheduling, `GET /health` over 15 requests:

| `DETWS_WORKER_POLL_TICKS` | avg     | min     | max     |
| ------------------------- | ------- | ------- | ------- |
| 1                         | 27.2 ms | 12.4 ms | 35.1 ms |
| 100                       | 28.0 ms | 12.5 ms | 42.2 ms |

Identical at a 100x longer idle sweep. The pre-notification poll would have added
up to one full sweep per request (~50 ms average at `POLL_TICKS=100`).

**Idle worker wakeups scale as `tick_rate / DETWS_WORKER_POLL_TICKS`.** At the
Arduino 1 kHz tick that is `1000 / POLL_TICKS` wakeups per second with no traffic
(1000/s at the default 1, 10/s at 100), each wakeup being one context switch plus
one service pass over idle slots. Raising the knob cuts that idle service-loop CPU
proportionally at no latency cost. Absolute idle CPU on a real build is dominated
by WiFi/IDF housekeeping, so the headline benefit of a high value is fewer wakeups
(CPU/power headroom), not a change to request latency.

**Parallel throughput (existing benchmark).** A CPU-bound handler (~0.2 s) under
4-way concurrency: `DETWS_WORKER_COUNT=1` ~5.9 req/s vs `=2` ~9.1 req/s (~1.5x).
It is not a full 2x because worker 1 shares Core 0 with WiFi/lwIP; single-request
latency is unchanged.

## Recipes

- **Default / low latency (most builds).** Leave everything at default. One worker
  on Core 1, `loop()` freed, events serviced immediately.
- **Battery / mostly idle.** Raise `DETWS_WORKER_POLL_TICKS` (e.g. 100 for a ~10 Hz
  idle sweep). Far fewer idle wakeups, and because events still wake the worker
  immediately there is no latency cost. Keep it well below your connection timeout
  so stale connections are still reaped promptly.
- **CPU-bound handlers / throughput.** Set `DETWS_WORKER_COUNT=2` to run handlers
  on both cores. Expect ~1.5x, not 2x (Core 0 also runs WiFi/lwIP). Ensure
  `MAX_CONNS >= DETWS_WORKER_COUNT` and that handlers touch only their own slot's
  state (the model already guarantees slot isolation).
- **Bursty connection load.** Raise `EVT_QUEUE_DEPTH` so a burst of accepts/data
  events cannot overflow a queue (an overflow is dropped, not blocked, to keep the
  tcpip thread non-blocking). Raise `MAX_CONNS` for more concurrent connections
  (BSS cost is fixed and linear).
- **Pin away from a busy core.** Set `DETWS_WORKER_CORE=0` only if your app keeps
  Core 1 busy; by default Core 1 is the right home (Core 0 carries WiFi/lwIP).

## Determinism notes

- Every knob above changes fixed-size BSS or scheduling, never introduces a heap
  allocation or an unbounded loop.
- `DETWS_WORKER_COUNT > 1` adds `DETWS_SCRATCH_ARENA_SIZE` of BSS per extra worker
  and one event queue per worker; all static.
- The internal time base stays 1000 Hz regardless of `DETWS_WORKER_POLL_TICKS`
  (see `services/det_clock.h`), so timeouts keep their tested 1 ms granularity.
