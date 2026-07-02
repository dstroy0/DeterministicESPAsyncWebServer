# 11.LoRaGateway - a real LoRa radio bridged to the gateway

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_LORA`, `DETWS_ENABLE_GATEWAY`

## What this example teaches

The radio-plugin half of [10.RadioGateway](../10.RadioGateway/README.md): instead of a
simulated feed, this drives an actual **Semtech SX127x / RFM95-96** over SPI and bridges
its frames to the [gateway](../10.RadioGateway/README.md).

```
RFM95 RX --SPI--> lora_recv() --> lora_frame_parse() --> det_gw_uplink()
                                                              |
                                           envelope + topic  lora/0/<from>
                                                              |
                                                     northbound publish (MQTT/HTTP/WS)
```

The `services/lora` module has two layers, and only the first is hardware-specific:

- **Driver** - the SX127x register protocol over a `lora_bus` (two callbacks that read /
  write a chip register). Here they are a few SPI transfers; that is the _only_ code tied
  to your board.
- **Codec** - `lora_frame_parse` / `lora_frame_build` handle the RadioHead 4-byte header
  (`to` / `from` / `id` / `flags`) that sits on top of the header-less LoRa PHY.

```cpp
lora_config cfg = {}; cfg.freq_hz = 915000000; cfg.spreading = 7; cfg.bandwidth = 7;
cfg.coding_rate = 1; cfg.sync_word = 0x12; cfg.tx_power = 17;
lora_init(&bus, &cfg);           // verifies the chip id, applies the config
lora_set_rx(&bus);               // listen

int n = lora_recv(&bus, buf, sizeof(buf), &rssi);   // -> a frame, or -1
lora_header h; const uint8_t *payload; uint16_t plen;
lora_frame_parse(buf, n, &h, &payload, &plen);
det_gw_uplink(0, h.from, payload, plen, rssi);      // bridge northbound
```

A **downlink** (a northbound command) is the mirror: `lora_frame_build()` then
`lora_send()`, driven by the gateway port's transmit callback.

## Wiring (ESP32 <-> RFM95)

| RFM95 | ESP32   |
| ----- | ------- |
| SCK   | GPIO 18 |
| MISO  | GPIO 19 |
| MOSI  | GPIO 23 |
| NSS   | GPIO 5  |
| RST   | GPIO 14 |
| DIO0  | GPIO 26 |
| VCC   | 3V3     |
| GND   | GND     |

Change the `PIN_*` constants for your board. A production build triggers RX off the DIO0
interrupt and rides the DMA + FORWARD-lane path (10.RadioGateway); this sketch polls to
stay simple. It needs the module wired to actually receive - the codec and the register
protocol are host-tested in `test/test_lora`.

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_LORA=1 -DDETWS_ENABLE_GATEWAY=1" \
  --lib="." examples/Foundation/11.LoRaGateway/11.LoRaGateway.ino
```
