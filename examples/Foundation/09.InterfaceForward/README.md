# 09.InterfaceForward - bridge frames between interfaces, DMA-driven

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_DMA`,
`DETWS_ENABLE_PREEMPT_QUEUE`, `DETWS_ENABLE_FORWARD`, `DETWS_DMA_SIMULATE`

## What this example teaches

The whole v5 ingest pipeline, end to end. A frame arriving on one interface is
**forwarded** to another, so the device bridges / routes between its interfaces
instead of only terminating traffic:

```
interface A --DMA RX--> callback --post--> FORWARD lane --> det_forward_ingress()
                                                                  |
                                                 (rule A->B allow, rate-capped)
                                                                  |
                                                       interface B egress send
```

Three pieces snap together, each already its own feature:

- **[DMA](../07.DmaIngest/README.md)** moves the bytes off interface A into a static
  buffer and fires a completion.
- The **[FORWARD lane](../08.PreemptLanes/README.md)** (an internal, high-priority
  preempting lane) carries the frame off the interrupt to a task.
- The **forwarding plane** (`services/forward`) applies the rules and sends the frame
  out interface B.

## The forwarding plane

Register interfaces (each with an egress send callback), then add `src -> dst` rules:

```cpp
det_forward_add_if(IF_B, DET_IF_WIFI_STA, if_b_send, nullptr);
det_forward_add_rule(IF_A, IF_B, DET_FWD_ALLOW, 0); // 0 = no rate cap

// when a frame arrives on interface A:
det_forward_ingress(IF_A, bytes, len); // -> forwards to every allowed destination
```

- **Default-deny**: a `(src, dst)` pair forwards only with an ALLOW rule and no DENY
  (a DENY always wins). A frame is never reflected to its source interface.
- **Multi-destination**: add several ALLOW rules for one source to fan a frame out
  (a hub / bridge).
- **Rate cap**: the fourth argument caps an ALLOW rule at N frames/second; excess is
  dropped, not blocked.
- **Fail-closed**: a full destination (send returns false) or an exceeded cap drops
  and is counted (`det_forward_get_stats()`), never blocks.

### Ingress ACL

Before any forwarding rule runs, an optional **access-control list** filters frames by
content. Each entry matches on the source interface (or `DET_FWD_IF_ANY`) and a byte
pattern under a mask; entries are evaluated in add order, **first match wins**, and a
frame matching none takes the ACL default (permit by default, so the ACL is opt-in):

```cpp
// drop frames whose first byte is 0xFF, on interface A, before forwarding:
uint8_t pat[1] = {0xFF}, mask[1] = {0xFF};
det_forward_acl_add(IF_A, /*offset*/ 0, pat, mask, /*patlen*/ 1, DET_FWD_DENY);

// allowlist instead (only explicitly permitted frames pass):
det_forward_acl_set_default(DET_FWD_DENY);
det_forward_acl_add(IF_A, 0, allowed_hdr, hdr_mask, 2, DET_FWD_ALLOW);
```

This sketch drops every 5th frame (a `0xFF` marker) at the ACL; watch `acl_denied`
climb in the serial stats while `forwarded` counts the rest.

Storage is static (zero heap): `DETWS_FWD_MAX_IFACES` interfaces,
`DETWS_FWD_MAX_RULES` rules, `DETWS_FWD_MAX_ACL` ACL entries.

Here interface A is a DMA channel fed by the simulator (no wire needed) and interface
B's egress just counts the bytes; a real build sends B out Wi-Fi / Ethernet / a bus or
another DMA channel.

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_DMA=1 -DDETWS_ENABLE_PREEMPT_QUEUE=1 -DDETWS_ENABLE_FORWARD=1 -DDETWS_DMA_SIMULATE=1" \
  --lib="." examples/Foundation/09.InterfaceForward/09.InterfaceForward.ino
```
