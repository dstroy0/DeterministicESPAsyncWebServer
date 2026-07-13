# 23.EthernetW5500 - run the server over a W5500 SPI Ethernet module

**Layer:** Foundation · **Build flags:** `DETWS_ENABLE_ETHERNET`, `DETWS_ETH_W5500`, `DETWS_ETH_W5500_*`

## What this example teaches

The RMII path ([19.Ethernet](../19.Ethernet)) needs a chip with an on-chip Ethernet MAC. The
**ESP32-S3 has no RMII MAC**, so a wired link there uses an **SPI Ethernet controller** - the
WIZnet **W5500** - over the HSPI bus. With `DETWS_ETH_W5500=1`, `init_eth_physical()` calls the
arduino-esp32 3.x ETH SPI API (`ETH.begin(ETH_PHY_W5500, ...)`); once the link has a DHCP IP the
server accepts on it with no other change:

```cpp
init_eth_physical();          // ETH.begin(ETH_PHY_W5500, ...) with the DETWS_ETH_W5500_* pins
while (!eth_ready()) delay(250);
// ... server.begin(80) - now serving over W5500 Ethernet
```

Nothing else changes: the egress reporting already classifies the wired route as
`DetIface::DETIFACE_ETH`, so `det_net_egress()`, per-route STA/AP/ETH interface filters, and every
protocol work over the link the moment it has an IP.

## Wiring (ESP32-S3-DevKitC, HSPI)

The pins come from the `DETWS_ETH_W5500_*` build flags in [build_opt.h](build_opt.h):

| Signal | GPIO | Flag                   |
| ------ | ---- | ---------------------- |
| `CS`   | `7`  | `DETWS_ETH_W5500_CS`   |
| `RST`  | `6`  | `DETWS_ETH_W5500_RST`  |
| `INT`  | `5`  | `DETWS_ETH_W5500_INT`  |
| `SCLK` | `12` | `DETWS_ETH_W5500_SCK`  |
| `MOSI` | `11` | `DETWS_ETH_W5500_MOSI` |
| `MISO` | `13` | `DETWS_ETH_W5500_MISO` |

Plus `VCC 3V3` and `GND`. Set the flags for your own wiring; see the
[hardware hookup guide](../../../docs/HARDWARE_HOOKUP.md) for the full pinout.

## Core requirement

W5500 SPI Ethernet is **arduino-esp32 3.x only** - the 2.x ETH library has no W5500 support
(`ETH_PHY_W5500` is undefined there). Build this with the arduino-cli / IDF-5.x core.

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32-s3-devkitc-1 --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_ETHERNET=1 -DDETWS_ETH_W5500=1 -DDETWS_ETH_W5500_CS=7 -DDETWS_ETH_W5500_RST=6 -DDETWS_ETH_W5500_INT=5 -DDETWS_ETH_W5500_SCK=12 -DDETWS_ETH_W5500_MISO=13 -DDETWS_ETH_W5500_MOSI=11" \
  --lib="." examples/Foundation/23.EthernetW5500/23.EthernetW5500.ino
```

(The Arduino IDE reads the flags from `build_opt.h` beside the sketch automatically.)
