# 08.PreemptLanes - internal preempting lanes vs the user lane

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_PREEMPT_QUEUE`

## What this example teaches

The preempting work queue ([06.PreemptQueue](../06.PreemptQueue/README.md)) is not one
queue but several named **lanes**, each with its own task at its own priority:

| Lane                    | Role                                          | Priority |
| ----------------------- | --------------------------------------------- | -------- |
| `DETWS_PQ_LANE_DMA`     | internal: DMA peripheral transfers            | highest  |
| `DETWS_PQ_LANE_FORWARD` | internal: interface forwarding                | high     |
| `DETWS_PQ_LANE_DEVICE`  | internal: device access                       | high     |
| `DETWS_PQ_LANE_USER`    | **exposed to your app** (no-arg `detws_pq_*`) | lowest   |

The internal lanes run **above** the user lane (base `DETWS_PQ_INTERNAL_PRIORITY`, DMA
highest) and below the lwIP tcpip / WiFi tasks. So the library's own real-time ingest
(a DMA transfer, a forwarded frame, a device event) always preempts user "process now"
work, and neither starves networking.

```cpp
// internal lane (priority 0 -> the lane default, above the user lane):
DetwsPqConfig dma = {}; dma.handler = on_critical; dma.core = 1;
detws_pq_start_lane(DETWS_PQ_LANE_DMA, &dma);
detws_pq_post_lane(DETWS_PQ_LANE_DMA, &item, 0);

// user lane - the no-arg API is unchanged (it drives DETWS_PQ_LANE_USER):
DetwsPqConfig user = {}; user.handler = on_background; user.priority = 5;
detws_pq_start(&user);
detws_pq_post(&item, 0);
```

`detws_pq_lane_priority(lane)` returns a lane's default priority, so you can confirm the
ordering. Each lane is independent: separate queue, separate task, separate
`detws_pq_high_water_lane()`. Queue storage is static (zero heap); a lane's task stack is
created only when you start it, so lanes you never start cost only their queue storage.

## Zero heap, fail-closed

Same guarantees as one lane: `DETWS_PQ_DEPTH` x `DETWS_PQ_ITEM_SIZE` static storage per
lane, fail-closed on a full queue, no hot-path locks.

## Build-flag note

`DETWS_ENABLE_PREEMPT_QUEUE` must reach the library build, so pass it as a build flag:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_PREEMPT_QUEUE=1" \
  --lib="." examples/Foundation/08.PreemptLanes/08.PreemptLanes.ino
```
