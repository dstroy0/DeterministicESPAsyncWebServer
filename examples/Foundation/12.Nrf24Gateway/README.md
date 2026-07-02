# 12.Nrf24Gateway - a real nRF24L01+ radio bridged to the gateway

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_NRF24`, `DETWS_ENABLE_GATEWAY`

## What this example teaches

A second radio driver plugged into the [gateway](../10.RadioGateway/README.md) - the
cheap, ubiquitous **Nordic nRF24L01+** 2.4 GHz module. It shows that a different bus
shape drops into the same gateway: where the [SX127x](../11.LoRaGateway/README.md) is
plain register read/write, the nRF24 uses an **SPI command protocol** and a separate
**CE** pin, so its `nrf_bus` carries an SPI transfer plus a CE callback.

```
nRF24 RX --SPI--> nrf24_recv() -> pipe + payload -> det_gw_uplink(port, pipe, ...)
                                                           |
                                        envelope + topic  nrf24/0/<pipe>
                                                           |
                                                  northbound publish (MQTT/HTTP/WS)
```

The nRF24 does its own **hardware addressing**: a received frame's source is the **pipe
number** it arrived on, so there is no in-payload header (no codec) - the pipe is the
address handed to `det_gw_uplink()`. Payloads are a **static width**
(`DETWS_NRF24_PAYLOAD`, default 32); a short send is zero-padded.

```cpp
nrf_config cfg = {}; cfg.address = addr5; cfg.channel = 76; cfg.data_rate = 0; cfg.tx_power = 3;
nrf24_init(&bus, &cfg);
nrf24_set_rx(&bus);

uint8_t buf[DETWS_NRF24_PAYLOAD]; uint8_t pipe;
int n = nrf24_recv(&bus, buf, sizeof(buf), &pipe);   // -> a frame, or -1
det_gw_uplink(0, pipe, buf, n, 0);                   // pipe = source address
```

## Wiring (ESP32 <-> nRF24L01+)

| nRF24 | ESP32   |
| ----- | ------- |
| SCK   | GPIO 18 |
| MISO  | GPIO 19 |
| MOSI  | GPIO 23 |
| CSN   | GPIO 5  |
| CE    | GPIO 4  |
| VCC   | 3V3     |
| GND   | GND     |

Change the `PIN_*` constants for your board (a 10 uF cap across VCC/GND helps the
module's TX current spikes). It needs the module wired to actually receive - the SPI
command protocol is host-tested in `test/test_nrf24`.

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_NRF24=1 -DDETWS_ENABLE_GATEWAY=1" \
  --lib="." examples/Foundation/12.Nrf24Gateway/12.Nrf24Gateway.ino
```
