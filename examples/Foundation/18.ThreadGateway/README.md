# 18.ThreadGateway - an OpenThread RCP bridged to the gateway

**Layer:** Foundation · **Build flags:** `DWS_ENABLE_THREAD`, `DWS_ENABLE_GATEWAY`

## What this example teaches

A **Thread** mesh plugged into the [gateway](../10.RadioGateway/README.md) through an
OpenThread **radio co-processor** (RCP - an nRF52840 / EFR32), which speaks **spinel** over
**HDLC-lite** framing on a UART. This decodes the HDLC frames and bridges each spinel
payload northbound - the basis of a Thread / Matter border router.

```
Thread RCP --UART--> spinel_frame_decode() --> spinel payload -> dws_gateway_uplink()
                                                                      |
                                               envelope + topic  thread/0/<tid>
                                                                      |
                                                       northbound publish (MQTT/HTTP/WS)
```

HDLC-lite wraps each spinel frame with an **FCS** (CRC-16/X-25), byte-stuffs the reserved
bytes, and terminates with a Flag `0x7E`. `services/thread` does the framing:

```cpp
uint8_t payload[256]; uint16_t plen;
int n = spinel_frame_decode(buf, len, payload, sizeof(payload), &plen);  // >0 / need-more / -1
if (n > 0)
    dws_gateway_uplink(0, tid, payload, plen, 0);
```

`spinel_frame_encode()` builds a frame the same way (this sketch sends a spinel RESET at
boot). Note the FCS is CRC-16/**X-25** (reflected, final XOR), transmitted low byte first -
distinct from Zigbee's ASH CRC. The codec is host-tested against the X-25 catalog check
value (`0x906E`) and the byte-stuffing in `test/test_thread`.

Interpreting the spinel command _inside_ a frame - the property get/set, an inbound IPv6
stream - is application work; this sketch bridges the raw spinel payload to show the
transport path.

## Wiring (ESP32 <-> Thread RCP)

| RCP | ESP32        |
| --- | ------------ |
| TX  | GPIO 16 (RX) |
| RX  | GPIO 17 (TX) |
| VCC | 3V3          |
| GND | GND          |

115200 8N1 (an nRF52840 dongle flashed with the OpenThread RCP firmware).

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_THREAD=1 -DDWS_ENABLE_GATEWAY=1" \
  --lib="." examples/Foundation/18.ThreadGateway/18.ThreadGateway.ino
```
