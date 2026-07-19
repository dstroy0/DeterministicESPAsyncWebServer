# Ethernet - run the server over a wired Ethernet PHY

**Layer:** Foundation · **Build flags:** `DWS_ENABLE_ETHERNET`, `ETH_PHY_*`

## What this example teaches

Some deployments want a **wired** uplink - PoE cameras, panel-mount controllers, RF-noisy
factory floors, anything that can't rely on Wi-Fi. With `DWS_ENABLE_ETHERNET` the physical
layer gains `init_eth_physical()` alongside `init_wifi_physical()`:

```cpp
init_eth_physical();          // ETH.begin() with the ETH_PHY_* build-flag config
while (!eth_ready()) delay(250);
// ... server.begin(80) - now serving over Ethernet
```

It is a thin wrapper over the Arduino **ETH** library for an RMII PHY (LAN8720 / TLK110 /
RTL8201 / DP83848). Nothing else changes: the egress reporting already classifies a wired
route as `DETIFACE_ETH`, so `dws_net_egress()`, per-route STA/AP/ETH interface filters, and
every protocol work over the wired link the moment it has an IP. Wi-Fi and Ethernet can also
run together (dual-homed) - the stack picks the default route.

## PHY configuration

The PHY pins, address, type, and clock source come from the standard `ETH_PHY_*` build
flags (they vary by board). This example is tuned for a generic LAN8720:

| Flag            | Value                |
| --------------- | -------------------- |
| `ETH_PHY_TYPE`  | `ETH_PHY_LAN8720`    |
| `ETH_PHY_ADDR`  | `1`                  |
| `ETH_PHY_POWER` | `-1` (none)          |
| `ETH_PHY_MDC`   | `23`                 |
| `ETH_PHY_MDIO`  | `18`                 |
| `ETH_CLK_MODE`  | `ETH_CLOCK_GPIO0_IN` |

Common boards differ - e.g. **WT32-ETH01** uses `ETH_PHY_POWER=16`, **Olimex ESP32-POE**
uses `ETH_PHY_ADDR=0 ETH_PHY_POWER=12 ETH_CLK_MODE=ETH_CLOCK_GPIO17_OUT`. Set the flags for
your hardware.

## Build-flag note

The flags must reach the library build, so pass them as build flags:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_ETHERNET=1 -DETH_PHY_TYPE=ETH_PHY_LAN8720 -DETH_PHY_ADDR=1 -DETH_PHY_POWER=-1 -DETH_PHY_MDC=23 -DETH_PHY_MDIO=18 -DETH_CLK_MODE=ETH_CLOCK_GPIO0_IN" \
  --lib="." examples/Peripherals/Ethernet/Ethernet.ino
```
