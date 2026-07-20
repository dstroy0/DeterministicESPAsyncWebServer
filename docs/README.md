# Documentation

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1 and HTTP/2 (with HTTP/3 over QUIC, host-tested), WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, SNMP, CoAP, Modbus TCP, MQTT, and OPC UA.

## Installation

**PlatformIO:**

```ini
lib_deps = https://github.com/dstroy0/DeterministicESPAsyncWebServer.git
```

**Arduino IDE:** Download the repository as a ZIP and use _Sketch → Include Library → Add .ZIP Library_.

## Quick Start

```cpp
#include <WiFi.h>
#include "dwserver.h"
#include "network_drivers/physical/physical.h"

DetWebServer server;

void handle_status(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "application/json", "{\"ok\":true}");
}

void setup()
{
    init_wifi_physical("SSID", "PASSWORD");
    while (!wifi_ready()) delay(250);

    server.on("/status", HTTP_GET, handle_status);
    server.set_cors("*");
    server.begin(80);
}

void loop()
{
    server.handle();
}
```

See `examples/Foundation/Configuration/Configuration.ino` for a full reference of every configurable flag and constant.

## Features

A compile-time menu grouped by the OSI layer each feature lives at, alphabetized within each layer: each cell is an optional `DETWS_ENABLE_*` subsystem (core HTTP/1.1, routing, middleware, JSON, templating, and chunked responses are always on). **Hover an entry for its summary; click through to [FEATURES.md](FEATURES.md) for the full description.** The tables are generated from [FEATURES.md](FEATURES.md) by `docs/utilities/gen_feature_tables.py`, so they never drift.

<!-- BEGIN GENERATED FEATURE TABLES (docs/utilities/gen_feature_tables.py) -->

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Foundation</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#config-io" title="Opt-in schema-driven config export / restore. Default off. Requires CONFIG_STORE. The app declares a fixed schema (key + type); services/config_io serializes the current values to a portable `key=value` text blob (backup / migrate) and parses one back into the store (restore / bulk template). Schema-driven rather than enumerating NVS, so it stays deterministic and zero-heap; the serialize / parse is host-tested.">Config IO</a></td>
  <td align="center"><a href="FEATURES.md#config-store" title="Typed NVS configuration store (WiFi creds, IP config,... as blobs). When set, src/services/config_store/config_store.h provides a typed key/value API (string / u32 / blob) that routes core settings into the ESP32's native NVS partition (via `Preferences`) instead of a JSON file on the filesystem - which survives FS corruption and is the corruption-resistant home for credentials. On host builds it is backed by a fixed in-memory table so the typed contract is unit-testable. Default off.">Config Store</a></td>
  <td align="center"><a href="FEATURES.md#device-id" title="Stable device UUID derived from the chip MAC (RFC 4122 v5). When set, src/services/device_id/device_id.h derives a deterministic v5 UUID from a MAC (via the library's SHA-1) - a storage-free, stable identity for mDNS hostnames, MQTT client IDs, etc. The MAC-&gt;UUID core is host-testable; dws_device_uuid() reads the ESP32 factory MAC. Default off.">Device ID</a></td>
  <td align="center"><a href="FEATURES.md#dma-peripheral-ingest" title="Opt-in DMA ingest / egress path (v5 milestone). Default off. services/dma moves peripheral bytes (UART / I2C / SPI) between the wire and a static buffer while the CPU is free, then a DMA-complete event carries the result up - the high-throughput field-bus ingest path (RS-485 UART, CAN over SPI, IO-Link). RX is double-buffered (ping-pong): the completed buffer is handed to a callback while the engine fills the other; the canonical callback posts the bytes into the preempting work queue so a high-priority task processes them off the interrupt (see Preempting Work Queue). Zero heap (static DWS_DMA_CHANNELS channels x DWS_DMA_BUF_SIZE buffers), fail-closed (one TX in flight per channel, feeds past the staging capacity are rejected). DWS_DMA_SIMULATE (default on) routes transfers through an in-memory ingress/egress simulator - feed bytes in, capture bytes out, optional TX-&gt;RX loopback - so the whole pipeline runs with no physical loopback wire, on the host bench and on-device; a real silicon driver plugs into the dws_dma_hw_* hooks when the flag is off. Host-tested (services/dma) and HW-verified on a DevKitV1: 2.2M+ frames ingested with zero integrity errors while an HTTP server was stress-loaded on the same core, no heap growth. See src/services/dma/dma.h.">DMA Peripheral Ingest</a></td>
  <td align="center"><a href="FEATURES.md#exception-decoder" title="Opt-in ESP32 panic / exception decoder for a live diagnostics panel, plus core-dump recovery. Default off. services/exc_decoder parses a captured Guru Meditation panic dump (the cause, the register PC + EXCVADDR, and the backtrace PC:SP frames) into a structured ExcInfo and serializes it as JSON for a &quot;/exception&quot; panel; the browser or a build server resolves the PCs to file:line against the firmware ELF (addr2line lives off-device). That decoder is pure, no heap/stdlib. **Core-dump partition recovery** closes the gap that a panic reboots the device and takes the console with it: ESP-IDF also writes a core dump to flash, which survives, so the next boot recovers it - `dws_exc_coredump_present` reports a stored image (checksum-verified) with its size/address, `dws_exc_coredump_summary` fills the _same_ ExcInfo so a crash recovered from flash renders through the same panel as a live capture, `dws_exc_coredump_save(fs, path)` streams the image out of flash to SD/LittleFS in `DWS_EXC_COREDUMP_CHUNK` pieces (no heap, any dump size), and `dws_exc_coredump_erase` clears the partition once the copy is safe. Offload is **transport-agnostic**: `dws_exc_coredump_read(offset, buf, len)` is the seam every consumer pulls through (a range past the image end is refused rather than returning trailing flash), so the same image goes to a filesystem, up an FTP data connection (`DWS_ENABLE_FTP_SESSION`), or into an HTTP body without this owner knowing about any of them - which matters because a dump that only ever reaches the device's own SD card is still lost with the device. The summary is **architecture-honest**: Xtensa's windowed ABI yields a real on-device backtrace, while RISC-V (C3/C6/H2/P4) carries only a stack dump - so there `frame_count` is 0 and the trap cause/value are reported rather than inventing frames, which is exactly why the raw offload matters on those parts. The saved file is the raw IDF flash image (24-byte header, then an ET_CORE ELF at offset 24, then a checksum), consumed as `esp-coredump ... -t raw`. **HW-verified on an ESP32-P4** saving to SD: a deliberate null dereference was recovered after reboot as `{&quot;cause&quot;:&quot;dws_worker&quot;,&quot;pc&quot;:&quot;0x4000004e&quot;,&quot;excvaddr&quot;:&quot;0x00000000&quot;,&quot;backtrace&quot;:[]}` - the fault address exactly the null pointer - a 12100-byte image was written to the card, and a following clean boot returned `{}`, proving the erase. **Also HW-verified on an ESP32-S3** (the Xtensa arm, and the FTP transport): the recovered summary carried a real 10-frame backtrace whose PCs `addr2line` resolves to the true crash path (`crash_handler` -&gt; `dispatch_matched_route` -&gt; `match_and_execute` -&gt; `http_poll_slot`), the 21540-byte image was streamed off the device to a live FTP server, and Espressif's own `esp-coredump info_corefile -t raw` then parsed that uploaded file and reported `exccause 0x1d (StoreProhibitedCause)`, `excvaddr 0x0`, `pc &lt;crash_handler(uint8_t, HttpReq*)+33&gt;` and crashed task `'dws_worker'` - independently confirming both the image format and the task name this decoder reports. Example CoreDump. See src/services/exc_decoder/exc_decoder.h.">Exception Decoder</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#failsafe-watchdog" title="Opt-in software watchdog: deadlock detection + fail-safe safe-state. When set, services/failsafe provides a fixed registry of &quot;lifelines&quot; (a task / worker / control loop that must check in within its deadline). dws_failsafe_check() detects one that stopped feeding (a hang / deadlock) and fires a breach callback once per episode so the app can enter a known-safe state. App-defined and per-lifeline, on top of the hardware task watchdog. Pure core, zero heap. Default off.">Failsafe Watchdog</a></td>
  <td align="center"><a href="FEATURES.md#gpio-map" title="Opt-in browser GPIO pin-mapper / diagnostics endpoint. Default off. When set, services/gpio_map serves a compile-time table of GPIO pins (number, label, direction, live level) as JSON for a browser diag panel, and accepts a control POST (`pin`, `level`) to drive an output. The live read / write uses the Arduino digital API on ESP32; the JSON serializer and the control parser are pure and host-testable.">GPIO Map</a></td>
  <td align="center"><a href="FEATURES.md#guardrails" title="Opt-in runtime heap/stack guardrails. Default off. When set, services/guardrails samples free heap, the heap low-water mark, the largest free block (fragmentation), and the calling task's remaining stack, and fires a callback when any crosses its threshold - a proactive fail-safe hook beyond the passive numbers in /metrics. The threshold evaluator and the JSON serializer are pure and host-tested; the sample reads esp_* / the FreeRTOS stack high-water on ESP32.">Guardrails</a></td>
  <td align="center"><a href="FEATURES.md#hardware-health" title="Opt-in hardware-health diagnostics. Four pure decision cores fed with samples the app reads from the hardware: a power-rail voltage-drop logger (dws_hwhealth_rail_sample tracks worst droop + sag/brownout counts), a SPI-bus CRC audit with hysteretic clock backoff (dws_hwhealth_spi_result halves/doubles the clock on fail/ok streaks), a GPIO short-circuit test (dws_hwhealth_gpio_short: driven vs readback), and a capacitor-leakage diag (dws_hwhealth_cap_leak: measured vs expected RC decay). No heap/stdlib. Default off.">Hardware Health</a></td>
  <td align="center"><a href="FEATURES.md#preempting-work-queue" title="Opt-in real-time ingest primitive (v5 milestone). Default off. Fixed-capacity queues, each feeding one dedicated core-pinned task: a producer posts a fixed-size item from a task (back or urgent-front, each with a wait timeout) or from an ISR (interrupt-safe via xQueueSendFromISR + portYIELD_FROM_ISR), and the scheduler preempts straight to the processing task so the work runs immediately instead of on the next tick - the clean ISR-to-&quot;process now&quot; hand-off for DMA-complete / GPIO / bus events. There are named lanes: one USER lane exposed to the app (the no-arg dws_pq_* API drives it) plus internal DMA / FORWARD / DEVICE lanes for the library's own real-time work. The internal lanes run above the user lane (base DWS_PQ_INTERNAL_PRIORITY, DMA highest) and below the lwIP tcpip / WiFi tasks, so internal ingest always preempts user work without starving networking; dws_pq_lane_priority() reports the ordering. Zero heap (static FreeRTOS queue storage per lane, compile-time DWS_PQ_DEPTH / DWS_PQ_ITEM_SIZE / DWS_PQ_STACK; a lane's task stack is created only when it starts, so unused lanes cost only their queue storage), fail-closed on a full queue, no hot-path locks so latency stays bounded. dws_pq_high_water_lane() reports the peak depth for sizing. HW-verified on a DevKitV1 (~12 us ISR-to-handler latency; the DMA and USER lanes ran continuously with zero errors under an HTTP flood); host-tested via the per-lane ring core (services/preempt_queue). See src/services/preempt_queue/preempt_queue.h.">Preempting Work Queue</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#psram-pool" title="Opt-in buffer placement policy (DRAM vs PSRAM) + SPI DMA ping-pong manager. Pure buffer-management decisions for a PSRAM-equipped ESP32: dws_psram_place picks DRAM vs PSRAM for a buffer by size, DMA requirement, and free-heap headroom (large/cold to PSRAM, small/hot + DMA to DRAM, always leaving an internal-DRAM reserve), and dws_pingpong_* keeps the classic SPI DMA double-buffer bookkeeping (CPU fills one buffer while DMA drains the other; swap flips their roles). The actual heap_caps_calloc is the app's. No heap/stdlib. Default off.">PSRAM Pool</a></td>
  <td align="center"><a href="FEATURES.md#rtc" title="I2C real-time-clock driver (DS1307 / DS3231). Default off. services/rtc reads and sets a battery-backed RTC over I2C (Wire, address 0x68), so the device knows the correct wall-clock time the instant it boots - offline, across power loss - with no network. It is the ideal middle of a time-source chain (GPS -&gt; RTC -&gt; upstream NTP), feeding `dws_time_now()` and the NTP server; `dws_rtc_time_source()` plugs straight into dws_time_source_add(). The BCD &lt;-&gt; Unix-epoch conversion of the seven time registers (12/24-hour, leap years, the clock-halt / century bit masks, range validation) is pure and host-tested across the chip's 2000-2099 range (that round-trip test caught a real 32-bit overflow past 2038); only the register read/write touches I2C. Zero heap. Example Rtc. See src/services/rtc/rtc.h.">RTC</a></td>
  <td align="center"><a href="FEATURES.md#sleep-scheduler" title="Opt-in dynamic sleep-cycle scheduler. When set, services/sleep_sched provides dws_sleep_next(): from the time since the last activity it returns how long a low-power device should sleep (0 = stay awake), ramping the window from a floor up to a ceiling the longer the idle streak runs. Pure decision core (the app applies the window via light / modem / deep sleep). Complements services/radio_power. Default off.">Sleep Scheduler</a></td>
  <td align="center"><a href="FEATURES.md#southbound" title="Opt-in southbound protocol-driver framework. The uniform seam every field-device driver plugs into so the app polls/drives any southbound device (a Modbus slave, a BACnet controller, a raw sensor over SPI/I2C/UART) through one facade: register a SouthboundDriver (a read/write/read_block/write_block vtable + its transport ctx), then address points by driver name via dws_southbound_read / _write / _read_block / _write_block. The block calls are the atomic multi-point (register-matrix) path. Bounded registry (DWS_SOUTHBOUND_MAX_DRIVERS, default 8), no heap; Modbus master is the one such driver today. Default off.">Southbound</a></td>
  <td align="center"><a href="FEATURES.md#time-source" title="Multi-source time fallback (NTP / RTC / GPS /... by priority). When set, src/services/time_source/time_source.h provides a small registry of user-defined time sources, each a callback returning Unix epoch seconds (0 when that source has no valid time). dws_time_now() queries them in priority order (lowest value first) and returns the first valid result, so the device falls back automatically when its preferred clock is unavailable. Pure and zero-heap (a fixed source table); host-testable. Default off.">Time Source</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#vfs" title="Unified virtual filesystem wrapper. Default off. services/vfs exposes one small file API (open/read/write/close, exists/size/remove/rename, whole-file helpers) over a pluggable backend, so a feature can target storage without knowing the medium. A built-in zero-heap RAM backend (fixed BSS pool - deterministic, host-identical) ships for scratch / tests; an Arduino-FS backend (ESP32) wraps a real fs::FS (LittleFS / SD / SPIFFS) for persistence. Mount one at startup; the API fails closed otherwise. Pool dimensions are tunable in ServerConfig.h (DWS_VFS_RAM_FILES, _RAM_FILE_SIZE, _MAX_OPEN, _NAME_MAX).">VFS</a></td>
  <td align="center"><a href="FEATURES.md#wear-leveling" title="Opt-in flash wear-leveling slot selector. When set, services/wearlevel provides dws_wearlevel_pick(): given per-slot write counts it returns the least-worn slot to write next, so repeated flash/NVS writes spread evenly and the region ages together instead of burning out one block. Pure core (the app owns the slots + persisted counts). Default off.">Wear Leveling</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Physical &amp; Data Link (L1-L2)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ads1115" title="TI ADS1115 4-channel 16-bit ADC with programmable gain (I2C). Default off. services/ads1115 builds the 16-bit config-register word for a single-shot single-ended reading (`dws_ads1115_config_single`: the OS start bit, the channel multiplexer, the programmable gain, single-shot mode, the data rate, and the disabled comparator - so `ch0, ±4.096 V, 128 SPS` is `0xC383`; out-of-range fields fall back sanely) and converts the signed 16-bit sample to microvolts for the selected gain's full-scale range (`dws_ads1115_raw_to_uv`: 125 µV/count at ±4.096 V). The config encoder + conversion are pure and host-tested (`native_ads1115`); only the config write / conversion read touches I2C. A cheap solder-and-test breakout for measuring batteries, potentiometers, and analog sensors with far more resolution than the ESP32 ADC. Example Ads1115 reads a potentiometer. See src/services/ads1115/ads1115.h.">ADS1115</a></td>
  <td align="center"><a href="FEATURES.md#ble-gatt" title="Opt-in Bluetooth ATT protocol codec + GATT characteristic bridge. The wire protocol under GATT for bridging the on-chip BLE radio to the web: services/ble_gatt builds and parses the common ATT PDUs (read / write / notify / error, Bluetooth Core Vol 3 Part F) and serializes a GATT characteristic table as JSON for the web stack (att_read_req / att_write_req / att_notify / att_error_rsp / att_parse / dws_gatt_char_json). The BLE stack owns the radio; this owns the ATT bytes + the northbound JSON. Pure, no heap/stdlib. Default off.">BLE GATT</a></td>
  <td align="center"><a href="FEATURES.md#bus-capture" title="Wired field-bus listen-only capture - the wired counterpart to Wi-Fi promiscuous capture. Default off. `bus_capture_begin(tx, rx, bitrate, sink)` installs the ESP32 CAN (TWAI) controller in listen-only mode - it receives and decodes every frame on the bus but never ACKs or transmits, so it stays invisible to the other nodes - and hands each decoded `CanFrame` to a sink; `bus_capture_poll()` drains the RX queue from `loop()`. Wire the sink into the forwarding plane (`DWS_ENABLE_FORWARD`) to bridge captured CAN frames to another interface - e.g. stream them to a wired collector over Ethernet. `can_to_socketcan()` (pure) formats a frame as a 16-byte Linux SocketCAN frame (big-endian `can_id` with the EFF / RTR flags), which with the libpcap `DLT_CAN_SOCKETCAN` link type is a capture Wireshark reads. The SocketCAN framing is pure and host-tested (`native_bus_capture`); the TWAI bring-up is ESP32-only and needs a CAN transceiver. Example CanCapture.">Bus Capture</a></td>
  <td align="center"><a href="FEATURES.md#cc1101" title="Opt-in CC1101 sub-GHz radio driver. A gateway radio plugin for the TI CC1101 300-928 MHz transceiver over SPI: services/cc1101 drives the chip's SPI header protocol (config registers, command strobes, status registers, TX/RX FIFO) - reset + apply a SmartRF register table + set channel + verify VERSION (dws_cc1101_init), send a variable-length packet (dws_cc1101_send), poll TX-done, enter RX, and read a packet with appended RSSI/LQI (dws_cc1101_recv), plus the RSSI-to-dBm decode. The huge modem config is a caller-supplied register table. Host-tested against a mock; the RF link needs the module. Default off.">CC1101</a></td>
  <td align="center"><a href="FEATURES.md#dshot" title="Opt-in DShot ESC throttle protocol codec. When set, services/dshot provides dws_dshot_encode() / _decode(): the 16-bit DShot frame (11-bit throttle/command + telemetry bit + 4-bit CRC), the bidirectional/extended inverted-CRC variant, and the per-rate bit timing for an RMT driver. Pure codec (the app clocks it out via RMT). Default off.">DShot</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#enocean" title="EnOcean ESP3 serial codec (v5 gateway plugin). Default off. services/enocean is the UART telegram codec for EnOcean Serial Protocol 3, the framing every USB / serial EnOcean gateway module (TCM 310 / USB 300) speaks for energy-harvesting 868 MHz switches and sensors. A telegram is `0x55 | data-len(2) | opt-len(1) | packet-type(1) | CRC8H | data | opt | CRC8D`. `dws_esp3_parse` frames one telegram out of a byte stream (returning the length consumed, 0 for &quot;need more&quot;, or -1 to drop a byte and resynchronise), verifying both CRC-8s (polynomial 0x07); `dws_esp3_build` assembles one; `dws_esp3_crc8` is the shared checksum. This is the radio half of an EnOcean-to-web bridge: pull the sender id + payload out of a RADIO_ERP1 telegram and bridge it northbound with `dws_gateway_uplink()`. Pure - you feed it the UART bytes, the module does the RF - and fully host-tested (CRC-8 known-answer values incl. the 0xF4 check, a build/parse round trip, malformed framing, and resynchronisation). Example EnOceanGateway reads a real module over UART. See src/services/enocean/enocean.h.">EnOcean</a></td>
  <td align="center"><a href="FEATURES.md#esp-now" title="ESP-NOW peer messaging. Default off. services/espnow wraps ESP-NOW connectionless peer-to-peer radio messaging in a 3-byte typed envelope (magic + type + length) so a receiver can demux by message type and reject a truncated frame, plus a bounded peer registry (DWS_ESPNOW_MAX_PEERS, no heap). The envelope codec + registry are pure and host-tested; the radio path (begin / add_peer / send / broadcast over esp_now, decoded frames to a callback) is ESP32-only and can bridge to WebSocket/SSE. No stdlib.">ESP-NOW</a></td>
  <td align="center"><a href="FEATURES.md#ethernet" title="Wired Ethernet bring-up. Default off (the ETH library is not linked). When set, the physical layer gains `init_eth_physical()` / `eth_ready()` alongside `init_wifi_physical()` - a thin wrapper over the Arduino ETH library for an RMII PHY (LAN8720 / TLK110 / RTL8201 / DP83848), so the server runs over a wired uplink (PoE, panel-mount, RF-noisy sites) instead of or alongside Wi-Fi. The PHY pins / address / type / clock come from the standard ETH_PHY_* build flags for your board. Nothing else changes: the egress reporting already classifies a wired default route as DETIFACE_ETH (`dws_net_egress`, host-tested classifier), so per-route STA/AP/ETH interface filters and every protocol work over the link the moment it has an IP; Wi-Fi and Ethernet can run dual-homed with the stack picking the default route. ESP32-only bring-up (the classifier is host-tested; the PHY needs the hardware). Example Ethernet. See src/network_drivers/physical/physical.h.">Ethernet</a></td>
  <td align="center"><a href="FEATURES.md#fdc2214" title="Opt-in FDC2114/2214 capacitance-to-digital field sensor. A field-perturbation sensing peripheral: services/fdc2214 decodes the FDC2x14's 28-bit conversion result (a capacitance shift moves the LC-tank frequency, giving contactless proximity / liquid-level / material sensing) - dws_fdc2214_data combines the register pair, dws_fdc2214_error pulls the flags, dws_fdc2214_sensor_freq_hz scales to frequency, and dws_fdc2214_build_config emits a single-channel bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec host-tested. Default off.">FDC2214</a></td>
  <td align="center"><a href="FEATURES.md#ina219" title="TI INA219 high-side current / power monitor (I2C). Default off. services/ina219 decodes the bus-voltage register (`dws_ina219_bus_mv`: value in bits [15:3], LSB 4 mV) and the shunt-voltage register (`dws_ina219_shunt_uv`: signed, LSB 10 µV), computes the calibration register from the current LSB and shunt resistance (`dws_ina219_calibration`: `40960000 / (current_lsb_ua * shunt_mohm)`, so 100 µA + 0.1 Ω -&gt; 4096), and scales the raw current / power registers to microamps / microwatts (`dws_ina219_current_ua`, `dws_ina219_power_uw`; the power LSB is 20x the current LSB). All the decode / calibration / scaling math is pure and host-tested (`native_ina219`); only the register read/write touches I2C. A cheap solder-and-test breakout for measuring how much current and power a circuit draws. Example Ina219 is a live power meter. See src/services/ina219/ina219.h.">INA219</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#interface-forwarding" title="Opt-in forwarding plane (v5 milestone). Default off. services/forward turns the device into a bridge / router: register interfaces (Wi-Fi STA / AP, Ethernet, a peripheral bus, a radio), each with an egress send callback, then add per-pair rules (`dws_forward_add_rule(src, dst, ALLOW/DENY, rate_cap)`). A frame arriving on one interface (`dws_forward_ingress()`, the canonical wiring being a DMA-complete event posted onto the internal FORWARD lane of the preempting queue) is forwarded to every allowed destination by calling that destination's send callback, so the device bridges / routes between its interfaces instead of only terminating traffic. Default-deny (a pair forwards only with an ALLOW rule and no DENY; a DENY always wins), never reflects a frame to its source, and fail-closed (an exceeded rate cap or a send callback returning false drops and is counted via `dws_forward_get_stats()`, never blocks). Multi-destination fan-out (several ALLOW rules for one source) gives hub behavior; a single ALLOW gives point-to-point routing. An optional ingress **ACL** filters frames by content before any forwarding rule runs: ordered entries (`dws_forward_acl_add()`) match on the source interface (or DWS_FWD_IF_ANY) and a byte pattern under a mask at an offset, first-match-wins, with a configurable default action (`dws_forward_acl_set_default()`) - permit-by-default for a denylist, or deny-by-default for an allowlist. **Policy routing** (route-by-tag) adds path selection on top: `dws_forward_route_add()` matches a frame by the same byte-pattern primitive (so it keys on any field at a known offset - EtherType, IP protocol, a port, an address prefix) and binds the match to a single **egress interface**, taking precedence over the src-&gt;dst fan-out (first matching route wins), so tagged traffic leaves a chosen NIC / radio; the same rate-cap / never-reflect / fail-closed guarantees apply, and `policy_routed` is counted. An optional **inspection hook** (build-time `DWS_FWD_INSPECT`, off by default for cost + privacy; runtime `dws_forward_set_inspector()`) runs a flexible app callback on every ingress frame after the ACL and before routing - to parse / observe / meter and optionally drop it (counted as `inspect_dropped`) - complementing the fast fixed-offset ACL. Static tables (zero heap): DWS_FWD_MAX_IFACES interfaces, DWS_FWD_MAX_RULES rules, DWS_FWD_MAX_ACL ACL entries, DWS_FWD_MAX_ROUTES policy routes. Host-tested (services/forward) + HW-verified on a DevKitV1: 800k+ frames ingested over DMA, ACL-filtered, and forwarded through the plane with exact accounting (forwarded + acl_denied == frames_in), zero loss, and zero integrity errors while an HTTP server was stress-loaded on the same core. This is the generic data path the post-v5 wireless gateway bridges sit on. See src/services/forward/forward.h.">Interface Forwarding</a></td>
  <td align="center"><a href="FEATURES.md#ld2410" title="HLK-LD2410 24 GHz mmWave presence / motion radar (UART). Default off. services/ld2410 syncs to the module's framed serial output (256000 baud: header `F4 F3 F2 F1`, little-endian length, payload, footer `F8 F7 F6 F5`) and decodes the target report - presence state (none / moving / stationary / both), the moving and stationary target distance (cm) and energy (0-100), the overall detection distance, and, in engineering mode, the per-gate energy of all nine range gates - plus encodes the config commands (enter / exit config, enable / disable engineering, restart). Unlike a PIR sensor it detects a perfectly still person (micro-motion / breathing), in the dark, through thin walls. The `Ld2410Stream` byte-by-byte reassembler is fixed-buffer, no-heap, and resyncs cleanly on dropped bytes or noise; the frame decoder + reassembler + command encoders are host-tested (`native_ld2410`), and only the UART read/write touches hardware. Example Ld2410 lights the onboard LED on presence. See src/services/ld2410/ld2410.h.">LD2410</a></td>
  <td align="center"><a href="FEATURES.md#ldc1614" title="Opt-in LDC1614 inductance-to-digital field sensor. A field-perturbation sensing peripheral: services/ldc1614 decodes the LDC1614's 28-bit conversion result (a nearby conductor changes the coil inductance via eddy currents, giving contactless metal proximity / displacement / EM-field sensing) - dws_ldc1614_data combines the register pair, dws_ldc1614_error pulls the flags, dws_ldc1614_sensor_freq_hz scales to frequency, and dws_ldc1614_build_config emits a single-channel bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec host-tested. Default off.">LDC1614</a></td>
  <td align="center"><a href="FEATURES.md#lora" title="Opt-in LoRa radio codec + driver (v5 gateway plugin). Default off. services/lora is the southbound-radio half of a LoRa-to-web bridge (see Radio Gateway), in two layers. **Codec**: `dws_lora_frame_parse` / `dws_lora_frame_build` handle the RadioHead-compatible 4-byte header (`to` / `from` / `id` / `flags`) that virtually every hobby / sensor LoRa deployment lays over the header-less LoRa PHY. **Driver**: the Semtech SX127x (SX1276-79 / RFM95-96) register protocol - `dws_lora_init` (verifies the chip id, switches to LoRa mode, programs the carrier frequency, spreading factor / bandwidth / coding rate, sync word, and PA power), `dws_lora_send` / `dws_lora_tx_done`, `dws_lora_set_rx`, and `dws_lora_recv` (reads the FIFO + packet RSSI on RxDone, drops on a CRC error) - all over a caller-supplied register-access bus (two callbacks that read/write a chip register), so the SPI + chip-select wiring is the integration's and the register sequence is portable. Bridge received frames northbound with `dws_gateway_uplink()`. The codec and the full register protocol are host-tested against a mock SX127x (a register file + a FIFO with the chip's auto-incrementing address pointer); the RF link itself needs the module. Example LoRaGateway drives a real RFM95 over SPI. See src/services/lora/lora.h.">LoRa</a></td>
  <td align="center"><a href="FEATURES.md#mpr121" title="NXP MPR121 12-channel capacitive-touch controller (I2C). Default off. services/mpr121 decodes the touch-status word (`dws_mpr121_touched` masks the 12 electrode bits out of the 16-bit status, which also carries the proximity electrode at bit 12 and the over-current flag at bit 15) and the chip's 10-bit filtered / baseline per-electrode data (`dws_mpr121_word10`), and builds the whole register bring-up as `(register, value)` byte pairs (`dws_mpr121_build_init`: soft reset, the NXP AN3944 rising/falling/touched filter defaults, per-electrode touch/release thresholds, CONFIG1/2, and the electrode-configuration register that starts it running with baseline tracking). The decode + init-sequence builder are pure and host-tested (`native_mpr121`); only the register read/write touches I2C. A cheap solder-and-test breakout for touch buttons / sliders. Example Mpr121 prints which pad you touch. See src/services/mpr121/mpr121.h.">MPR121</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#nrf24" title="Opt-in nRF24L01+ radio driver (v5 gateway plugin). Default off. services/nrf24 is a driver for the Nordic nRF24L01+ 2.4 GHz module - cheap point-to-multipoint sensor links bridged to the web stack. Unlike the SX127x (plain register read/write), the nRF24 speaks an SPI command protocol (each transaction is a command byte + data with the STATUS register shifted out first) and needs a separate CE pin, so the driver runs over an `nrf_bus` that carries a full-duplex SPI transfer plus a CE-set callback - the only board-specific code. `dws_nrf24_init` verifies the chip via a register read-back and programs the channel, data rate, power, 5-byte address, and static payload width; `dws_nrf24_send` (zero-padded to the width) / `dws_nrf24_tx_done`, `dws_nrf24_set_rx`, and `dws_nrf24_recv` (reports the receiving pipe). The chip's hardware pipe addressing means a received frame's source is the pipe number (no in-payload codec); bridge it northbound with `dws_gateway_uplink(port, pipe, ...)`. The command protocol is host-tested against a mock chip (register file + payload buffers + STATUS write-1-to-clear); the RF link itself needs the module. Example Nrf24Gateway drives a real module over SPI. See src/services/nrf24/nrf24.h.">nRF24</a></td>
  <td align="center"><a href="FEATURES.md#pca9685" title="NXP PCA9685 16-channel 12-bit PWM / servo driver (I2C). Default off. services/pca9685 turns the ESP32's two I2C wires into sixteen hardware PWM outputs. `dws_pca9685_prescale` computes the PRESCALE register for a PWM frequency from the 25 MHz oscillator (`round(25e6 / (4096*freq)) - 1`, clamped 3..255; 50 Hz -&gt; 121); `dws_pca9685_channel_reg` gives a channel's register base (`0x06 + 4*channel`); `dws_pca9685_us_to_count` converts a servo pulse width (microseconds) to a 12-bit OFF count at the configured frequency; and `dws_pca9685_set_pwm_bytes` packs the 5-byte channel write (12-bit ON/OFF little-endian, preserving the full-on/off flag). The prescale / count math + the register encoder are pure and host-tested (`native_pca9685`); only the register writes touch I2C. A cheap solder-and-test breakout for up to 16 servos or LEDs. Example Pca9685 sweeps a servo. See src/services/pca9685/pca9685.h.">PCA9685</a></td>
  <td align="center"><a href="FEATURES.md#pn532" title="PN532 NFC frame codec (v5 gateway plugin). Default off. services/pn532 is the command-frame protocol of the NXP PN532 - the ubiquitous NFC / RFID reader on I2C / SPI / HSU breakouts - so a tag read/write becomes an HTTP / MQTT event. The host and the chip exchange normal information frames `00 00 FF | LEN | LCS | TFI | PData | DCS | 00`, where TFI is 0xD4 (host) / 0xD5 (chip), LCS is the length checksum and DCS the data checksum, plus a 6-byte ACK frame. `dws_pn532_build_frame` assembles a command (the per-command PData - GetFirmwareVersion, InListPassiveTarget, InDataExchange - is the application's), `dws_pn532_parse_frame` frames + verifies a response (returning the length consumed, need-more, or a resync signal), and `dws_pn532_is_ack` / `dws_pn532_build_ack` handle the ACK. Pure - you carry the bytes over your I2C / SPI / UART - and host-tested against the documented GetFirmwareVersion command / response frames and their LCS / DCS. Example NfcGateway reads a real PN532 over I2C and bridges each tag UID northbound. See src/services/pn532/pn532.h.">PN532</a></td>
  <td align="center"><a href="FEATURES.md#radio-gateway" title="Opt-in radio / wireless gateway bridge (v5 milestone). Default off. services/gateway is the generic southbound-to-northbound bridge that ties the hardware-ingest pipeline to the web stack: a southbound radio (LoRa / nRF24 / CC1101 / Zigbee / Z-Wave / ... reached over SPI / I2C / UART) is a **port**. When it receives a frame - the data-ready ISR reads it over DMA (services/dma), posts it onto the FORWARD lane (services/preempt_queue), and a per-radio codec extracts the source node address + payload - you call `dws_gateway_uplink()`; the gateway envelopes it (source address, port, RSSI, sequence) and publishes it northbound through the uplink callback, which you wire to MQTT / HTTP / WebSocket / UDP. A command runs the other way via `dws_gateway_downlink()` to the port's transmit callback (the radio's SPI / UART write). `dws_gateway_topic()` formats a routing key `&lt;prefix&gt;/&lt;port&gt;/&lt;addr&gt;`; a per-port uplink rate cap and fail-closed drops (no sink / unknown port / exceeded cap / refused, all counted via `dws_gateway_get_stats()`) keep it bounded. The radio transmit and the northbound publish are callbacks (the seam a real radio driver / protocol binding plugs into), so the bridge is host- and device-testable with no radio. Zero-heap static tables (DWS_GW_MAX_PORTS). Host-tested (services/gateway) + HW-verified on a DevKitV1: 690k+ radio frames bridged northbound over DMA + the FORWARD lane with exact accounting (up_in == published) and zero payload-integrity errors while an HTTP server was stress-loaded on the same core. This is the framework the per-radio gateways (LoRa, Zigbee, ...) on the roadmap plug into. See src/services/gateway/gateway.h.">Radio Gateway</a></td>
  <td align="center"><a href="FEATURES.md#radio-power" title="Opt-in radio power controls. Default off. services/radio_power applies the WiFi modem-sleep mode and an optional max-TX-power cap in one call (esp_wifi_set_ps / esp_wifi_set_max_tx_power) - trade throughput/latency for lower average power on a battery device. The mode names are host-tested; the apply is ESP32-only.">Radio Power</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#radio-sniffer" title="Opt-in receive-only radio channel sniffer to pcap. Feeds frames pulled off the air by the RF gateway drivers (CC1101 / LoRa / 802.15.4) in receive-only mode into the capture pipeline: services/radio_sniff wraps each 802.15.4 MAC frame in the Wireshark TAP pseudo-header (carrying per-frame RSSI + channel) and a pcap record so the forwarded stream is a valid .pcap. dws_radiosniff_global writes the DLT-TAP global header and dws_radiosniff_tap_record writes one record. Pure framing (no heap/stdlib); the radio drivers own the receive. Default off.">Radio Sniffer</a></td>
  <td align="center"><a href="FEATURES.md#raw-l2" title="Opt-in raw Layer-2 Ethernet frame codec. When set, services/rawl2 builds/parses Ethernet II + 802.1Q VLAN frames (no FCS - the MAC appends it; dws_eth_fcs is provided for the cases that need it), so the app can inject/receive arbitrary L2 frames via esp_eth_transmit / esp_wifi_80211_tx - the basis for the raw-L2 industrial protocols (PROFINET DCP, GOOSE, POWERLINK). Pure codec, host-tested. Default off.">Raw L2</a></td>
  <td align="center"><a href="FEATURES.md#sht3x" title="Sensirion SHT3x (SHT30/31/35) temperature / humidity sensor (I2C). Default off. services/sht3x issues the single-shot measurement command (`0x2400`), then verifies the CRC-8 on each returned 16-bit word before trusting it (`dws_sht3x_crc8`: polynomial 0x31, init 0xFF - the Sensirion datasheet check value `0xBEEF` -&gt; `0x92` is a unit test) and converts the raw ticks with the linear map (`T = -45 + 175*raw/65535`, `RH = 100*raw/65535`) into signed integer milli-units (milli-degrees C, milli-percent RH), so there is no heap and no float printf. `dws_sht3x_parse` rejects the whole six-byte reading on any CRC mismatch. The CRC + conversion + parse are pure and host-tested (`native_sht3x`); only the command write / data read touches I2C. A cheap solder-and-test breakout (GY-SHT31) for environmental telemetry. Example Sht3x prints temperature and humidity. See src/services/sht3x/sht3x.h.">SHT3x</a></td>
  <td align="center"><a href="FEATURES.md#sigfox" title="Sigfox modem AT-command codec (v5 gateway plugin). Default off. services/sigfox is the tiny-uplink half of a Sigfox-to-web bridge: a Wisol (SFM10R) / Murata Sigfox modem is driven by AT commands over UART for ultra-low-power telemetry over the Sigfox 0G network (a message is capped at 12 bytes and ~140/day, so uplinks are rare and small). `dws_sigfox_build_uplink` formats an `AT$SF=&lt;hex&gt;` command (uppercase hex of the payload) and `dws_sigfox_parse_response` classifies the modem's reply as OK, ERROR, or still pending. Uplink-only (the common Sigfox use - a device sends readings up, it is not addressed downlink). Pure text codec - you carry the bytes over your UART - and host-tested (the AT$SF encoding, its 12-byte / output-buffer bounds, and the response classification). Example SigfoxUplink sends a reading from a real modem. See src/services/sigfox/sigfox.h.">Sigfox</a></td>
  <td align="center"><a href="FEATURES.md#thread" title="Thread spinel / HDLC-lite framing codec (v5 gateway plugin). Default off. services/thread is the HDLC-lite data-link layer that carries spinel frames to an OpenThread radio co-processor (an nRF52840 / EFR32 RCP) over UART, so an 802.15.4 / Thread mesh is bridged to IP and the web (the basis of a Thread / Matter border router). HDLC-lite appends an FCS, byte-stuffs the reserved bytes (Flag 0x7E, Escape 0x7D, XON, XOFF), and terminates with the Flag. The FCS is the HDLC frame check sequence - CRC-16/X-25 (poly 0x1021 reflected, init 0xFFFF, reflected in/out, final XOR 0xFFFF), transmitted low byte first, distinct from Zigbee's ASH CRC. `dws_spinel_frame_encode` wraps a payload; `dws_spinel_frame_decode` finds the flag, removes the stuffing, and verifies the FCS (returning the bytes consumed, need-more, or a resync signal); `dws_spinel_fcs` is the shared checksum. On top of the framing there is a **spinel command layer**: `dws_spinel_pack_uint` / `dws_spinel_unpack_uint` handle spinel's packed-unsigned-integer encoding (7 bits/byte, little-endian, continuation bit), and `dws_spinel_command_build` / `dws_spinel_command_parse` assemble and split a property command (`header | CMD | PROP | value`) - so you can issue a `PROP_VALUE_GET`/`SET` or read a `PROP_VALUE_IS` update, not just move opaque frames. On top of that there is a **property registry + value semantics**: `SpinelProp` / `SpinelStatus` name the property ids and status codes a gateway acts on, `dws_spinel_prop_lookup` / `dws_spinel_prop_name` / `dws_spinel_status_name` resolve an id to its name and primary datatype, and a typed cursor (`SpinelReader` / `SpinelWriter` with an accessor per spinel datatype - `get`/`put` for bool, uint8/int8, uint16/int16, uint32/int32, packed-uint, EUI64, IPv6, UTF8 string, raw data, and length-prefixed data) decodes or builds a property value field by field, with out-of-bounds reads latched into an error you check once. So `NCP_VERSION` decodes to its version string, `PROTOCOL_VERSION` to its two packed ints, `HWADDR` / `MAC_15_4_LADDR` to an EUI64, `MAC_15_4_PANID` to a uint16, and `LAST_STATUS` to a named status. Pure - you carry the bytes over your UART - and host-tested against the CRC-16/X-25 catalog check value (`0x906E`), the byte-stuffing round trip, the packed-int values (127 -&gt; `7F`, 128 -&gt; `80 01`, 1337 -&gt; `B9 0A`), the full command -&gt; frame -&gt; decode -&gt; parse stack, and the typed value round trips + registry lookups. Example ThreadGateway decodes named properties and bridges a real RCP. Verified two ways: **on real hardware** against an **ESP32-C6 running ESP-IDF `ot_rcp`** (a native 802.15.4 radio), where the codec drove a spinel RESET and decoded 8/8 properties off the wire - `NCP_VERSION` (`openthread-esp32/...; esp32c6; ...`), `PROTOCOL_VERSION` 4.3, `INTERFACE_TYPE` 3, `CAPS` (7 entries incl. a multi-byte packed 513), the factory `HWADDR` EUI64, `PHY_CHAN` 11, `PANID`, `MAC_15_4_LADDR`; and **against the OpenThread reference RCP** (OpenThread's own `ot-rcp` over a pty) for protocol conformance against the reference implementation. See src/services/thread/thread.h.">Thread</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#vl53l0x" title="Opt-in VL53L0X optical time-of-flight ranging sensor. A field-perturbation sensing peripheral for contactless distance / gesture: services/vl53l0x decodes the ST VL53L0X ranging registers - dws_vl53l0x_range_mm combines the range byte pair, dws_vl53l0x_data_ready decodes the interrupt-status byte, and dws_vl53l0x_range_valid checks the device range-status field; the ESP32 binding verifies the model id, starts continuous ranging, and reads the distance over I2C. Default-settings ranging (ST's tuning blob is not applied). Pure codec host-tested. Default off.">VL53L0X</a></td>
  <td align="center"><a href="FEATURES.md#wi-fi-capture" title="Passive 802.11 promiscuous (monitor) capture. Default off. `dws_promisc_begin(channel, sink)` puts the radio in promiscuous mode (`esp_wifi_set_promiscuous`) and delivers every frame - with RSSI and channel - to a sink; capture is strictly passive (no injection). Wire the sink into the forwarding plane (`DWS_ENABLE_FORWARD`) to bridge captured Wi-Fi frames to another interface - e.g. stream them to a wired collector over Ethernet (&quot;capture on Wi-Fi, forward to Ethernet&quot;). Ships a pure 802.11 MAC-header parser (`wifi_frame_parse`: type/subtype, the to/from-DS src/dst/bssid layout, QoS, WDS 4-address, sequence number) and libpcap framing (`DLT_IEEE802_11`) so a forwarded frame is a valid PCAP a wired Wireshark / tcpdump reads. The parser and PCAP framing are pure and host-tested (`native_promisc`); the radio bring-up is ESP32-only. Example WifiCapture.">Wi-Fi Capture</a></td>
  <td align="center"><a href="FEATURES.md#wi-fi-sniffer" title="Opt-in 802.11 sniffer / traffic analyzer with channel-agility roaming. Default off. `dws_wifi_parse` decodes an 802.11 MAC header (frame-control type/subtype + flags and the addresses whose roles depend on ToDS/FromDS), `dws_wifi_stats_*` tallies frames by type for a traffic panel, and `dws_wifi_should_roam` decides when a candidate AP is enough stronger (RSSI hysteresis) to justify roaming. On top of that, `dws_wifi_scan_*` is the **channel-hop dwell schedule** (sweep a channel range, wrap-safe against a `dws_millis()` rollover, counting sweeps) and `dws_wifi_survey_*` is the **per-channel RSSI survey** - frames heard, strongest RSSI, and the BSSID that sent it, per channel - with `dws_wifi_survey_best` returning the strongest _other_ channel, which is precisely the roam candidate `dws_wifi_should_roam` judges. With `DWS_ENABLE_PROMISC` also set, the live binding (`dws_wifi_sniffer_begin` / `_tick` / `_end` + `_stats` / `_survey` / `_scan`) drives the promiscuous-capture owner (`services/promisc`) rather than installing a second radio callback: its sink decodes, tallies, and surveys each captured frame, and `_tick()` hops channels on the dwell. The stats/survey are deliberately lock-free (whole-word counters, updated in the radio callback) so capture is never stalled by a reader; a report is a live snapshot, not an instantaneous cut. Schedule, survey, decode, and decision are pure and host-tested (`native_wifi_sniffer`); only the radio binding is ESP32-only. **HW-verified** on an ESP32-S3 against live 2.4 GHz traffic: 490 frames decoded across 10 channel sweeps, nine channels surveyed with real APs, and the roam decision correctly picked the -31 dBm channel over the -66 dBm one it was sitting on. Example WifiSniffer.">Wi-Fi Sniffer</a></td>
  <td align="center"><a href="FEATURES.md#wi-sun" title="Opt-in Wi-SUN FAN border-router connector. Wi-SUN FAN is an IPv6/UDP/CoAP mesh terminated by a border router, so the connector rides the existing IP stack rather than driving a radio: services/wisun keeps a table of FAN nodes (their DWSIp addresses + join state) behind the border router and builds the CoAP client requests to their resources (dws_wisun_build_coap frames an RFC 7252 header + Uri-Path options + payload; the CoAP service ships only a server). dws_wisun_nodes_json exposes the mesh to the web. The app sends the built PDU over dws_udp; the chosen devboard only sets which border router you point at. Pure, no heap/stdlib. Default off.">Wi-SUN</a></td>
  <td align="center"><a href="FEATURES.md#z-wave" title="Z-Wave Serial API frame codec (v5 gateway plugin). Default off. services/zwave is the host-side Serial API of a Silicon Labs 500 / 700-series Z-Wave controller over UART, so a Z-Wave mesh is bridged to the web. Data frames are `SOF (0x01) | LEN | Type | Command | Data | Checksum`, where LEN counts Type..Checksum, Type is 0x00 (REQ) / 0x01 (RES), and the checksum is 0xFF XOR-folded over LEN..last-data; single-byte ACK (0x06) / NAK (0x15) / CAN (0x18) frames flow-control them. `dws_zwave_build_frame` assembles a function command (GetVersion, SendData, AddNodeToNetwork, ...), `dws_zwave_parse_frame` frames + verifies a response (length / need-more / resync), and `dws_zwave_is_ack` / `_nak` / `_can` / `dws_zwave_build_ack` handle the control bytes; the per-command payload is the application's. Pull the source node id + payload out of an ApplicationCommandHandler report and bridge it northbound with `dws_gateway_uplink()`. Pure - you carry the bytes over your UART - and host-tested against the documented GetVersion frame (`01 03 00 15 E9`) and its XOR checksum. Example ZWaveGateway bridges a real controller. See src/services/zwave/zwave.h.">Z-Wave</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#zigbee" title="Zigbee EZSP / ASH framing codec (v5 gateway plugin). Default off. services/zigbee is the ASH (Asynchronous Serial Host, UG101) data-link layer that carries EZSP frames to a Silicon Labs EmberZNet network co-processor over UART, so a Zigbee network is bridged to the web. Each ASH frame is `[control | payload | CRC-16/CCITT]`, byte-stuffed so the reserved control bytes (Flag 0x7E, Escape 0x7D, XON/XOFF, Substitute, Cancel) never appear in the body, and terminated by the Flag byte. `dws_ash_frame_encode` wraps a control byte + payload into a stuffed, CRC'd frame; `dws_ash_frame_decode` finds the flag, removes the stuffing, and verifies the CRC (returning the bytes consumed, need-more, or a resync signal); `dws_ash_crc16` is the shared checksum. Bridge the EZSP payload of an incoming-message DATA frame northbound with `dws_gateway_uplink()`; interpreting the EZSP command itself is the application's. Pure - you carry the bytes over your UART - and host-tested against the documented ASH RST frame (`C0 38 BC 7E`), the CRC-16 (`0x38BC`), and the byte-stuffing round trip. Example ZigbeeGateway bridges a real NCP. See src/services/zigbee/zigbee.h.">Zigbee</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Network (L3)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#dns-resolver" title="Opt-in DNS resolver with answer verification. Default off. services/dns_resolver resolves a hostname to an IPv4 address (lwIP dns_gethostbyname, marshalled to tcpip_thread like the http_client) and can reject suspicious answers - 0.0.0.0, broadcast, loopback, multicast - which are spoofing / DNS-rebinding indicators for a remote host. The address classifier / verifier is pure and host-tested; the resolve is ESP32-only (blocking, so call it off the request hot path).">Dns Resolver</a></td>
  <td align="center"><a href="FEATURES.md#happy-eyeballs" title="Opt-in dual-stack Happy Eyeballs destination selection. The client-side IPv6/IPv4 fallback decision on top of the shipped DWSIp: dws_he_pref scores a destination (RFC 6724 scope + family), dws_he_order sorts a candidate list and interleaves the address families (RFC 8305) so successive connection attempts alternate v6/v4, and dws_he_attempt_due gates the next attempt by the Connection Attempt Delay. Fast IPv6 when it works, quick fallback to IPv4 when it does not. Needs DWS_ENABLE_IPV6 to matter. No heap/stdlib. Default off.">Happy Eyeballs</a></td>
  <td align="center"><a href="FEATURES.md#ipv6" title="Dual-stack IPv6. Default off. The TCP and UDP listeners already bind `IPADDR_TYPE_ANY`, so the server answers over IPv6 the moment the interface has a v6 address; `DWS_ENABLE_IPV6` turns IPv6 on for the Wi-Fi netif (`init_ipv6_physical()` -&gt; SLAAC: a `fe80::` link-local address plus a global one if a router advertises a prefix), and `net_global_ipv6()` reads the acquired global address from lwIP. The DWSIp address core (`network_drivers/network/ip.h`) is one family-tagged type for both v4 and v6, with RFC 4291 text parsing (`::` zero-compression, embedded-v4 `::ffff:a.b.c.d`), RFC 5952 canonical formatting, and scope classification (loopback / link-local / private-ULA / multicast / global). The address core is pure and host-tested (`native_det_ip`); the netif bring-up is ESP32-only. Example IPv6. Requires an lwIP built with `LWIP_IPV6=1` (the stock Arduino-ESP32 core ships it). HW-verified (2026-07-19) on an ESP32-S3: SLAAC formed a link-local + ULA + a router-advertised global address, and the dual-stack `:80` listener answered real HTTP `GET`s over IPv6 from an on-link host.">IPv6</a></td>
  <td align="center"><a href="FEATURES.md#link-manager" title="Opt-in multi-interface egress selection / failover policy. The policy that drives which interface carries traffic once a device has more than one (a wired Ethernet PHY alongside WiFi STA / softAP): services/link_manager keeps a small table of interfaces (kind + priority + up/down) and deterministically selects the best link that is up, escalating to a higher-priority interface when it comes up and failing over when it drops, reporting only real transitions so the app reconfigures the netif once. The PHY bring-up (esp_eth) stays the app's. No heap/stdlib. Default off.">Link Manager</a></td>
  <td align="center"><a href="FEATURES.md#network-adaptation" title="Opt-in network adaptation decisions. When set, services/netadapt provides two pure decisions: dws_netadapt_window() sizes the TCP receive window from the free heap (bigger when RAM is plentiful, shrinking when tight), and dws_netadapt_dhcp_fallback() decides when to give up on DHCP and use a static IP. The app applies the results (lwIP window / netif config). Default off.">Network Adaptation</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#proxy-protocol" title="HAProxy PROXY protocol codec - recover the real client IPv4 when the server sits behind a load balancer / reverse proxy that prepends a PROXY header. Default off. services/proxy_protocol provides `proxy_parse` (detects + decodes a v1 text `PROXY TCP4 ...\r\n` header or a v2 binary header - 12-octet signature + ver_cmd / family / address block - and reports the bytes to skip before the real stream) plus `proxy_v1_build` / `proxy_v2_build` for a TCP/IPv4 header. Handles the library's IPv4 family; IPv6 / UNIX / LOCAL headers parse to their length but yield no addresses. Format per the HAProxy PROXY protocol spec; pure and host-tested. See src/services/proxy_protocol/proxy_protocol.h.">Proxy Protocol</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Transport (L4)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#accept-throttle" title="Opt-in global accept-rate throttle (connection-flood defense). Default off (zero cost / no behavior change). When set to 1 the accept callback rejects new connections once more than DWS_ACCEPT_THROTTLE_MAX have been accepted within a DWS_ACCEPT_THROTTLE_WINDOW_MS fixed window (global across all listeners, two static counters - no per-IP table). This bounds connection churn (e.g. reconnect brute-force) on top of the bounded connection pool and the per-connection auth limits. mitigate finer-grained / per-IP attacks at the network layer.">Accept Throttle</a></td>
  <td align="center"><a href="FEATURES.md#ip-allowlist" title="Opt-in source-IP allowlist (accept-time firewall, keyed by source IPv4). Default off (zero cost / no behavior change). When set, the accept callback drops any connection whose source address does not match a configured CIDR rule (see listener_ip_allow_add()). An empty allowlist allows everything, so enabling the feature before adding rules never locks the device out. Rules live in a fixed BSS table of DWS_IP_ALLOWLIST_SLOTS entries (no heap). This is a coarse first-line filter - a spoofed source address can still pass it - so combine it with the accept throttles and network-layer filtering.">IP Allowlist</a></td>
  <td align="center"><a href="FEATURES.md#keep-alive" title="HTTP/1.1 persistent connections (keep-alive). Default on: a cleanly-parsed request is answered with `Connection: keep-alive` and the slot is recycled for the next request on the same socket: HTTP/1.1 keeps the connection open unless the client sends `Connection: close`; HTTP/1.0 closes unless the client sends `Connection: keep-alive`. Set `DWS_ENABLE_KEEPALIVE=0` for the legacy behavior (every response carries `Connection: close` and the connection is closed after one request). The default connection pool is `MAX_CONNS=8` to give a persistent-connection workload headroom above its peak concurrency. Error responses (400/413/414 and any non-PARSE_COMPLETE path) always close, since the next request boundary is unknown. Idle keep-alive connections are still reclaimed by the existing conn_timeout sweep, and each connection serves at most DWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.">Keep-Alive</a></td>
  <td align="center"><a href="FEATURES.md#mtls" title="Mutual TLS - require and verify a client certificate (mTLS). Default off. When set (requires TLS), the server can be given a trust-anchor CA via DWS::tls_require_client_cert(): the TLS handshake then demands a client certificate chaining to that CA (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents none or an untrusted one. The verified peer's subject DN is available to handlers via DWS::tls_client_subject(). Strong transport-level client authentication with no passwords.">MTLS</a></td>
  <td align="center"><a href="FEATURES.md#per-ip-throttle" title="Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4). Default off (zero cost / no behavior change). Complements the global accept throttle: the accept callback rejects a new connection once one source IPv4 address has opened more than DWS_PER_IP_THROTTLE_MAX connections within a DWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of DWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source addresses; when a new address arrives and the table is full, an expired or least-recently-started bucket is reused, so memory stays bounded (no heap). This bounds reconnect/brute-force churn from a single host (the gap left by the global throttle, which cannot tell one noisy client from many). It is best-effort: an attacker spreading across many source addresses can still churn the bounded connection pool, so combine it with the global throttle and network-layer filtering.">Per IP Throttle</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#socket-pool" title="Opt-in dynamic socket recycling: an LRU connection-slot pool. The transport-pool half of the adaptive-networking work: services/sockpool keeps a fixed table of connection slots and, when saturated, recycles the least-recently-used slot for a new peer (dws_sockpool_acquire returns the evicted id so the transport closes it), plus touch / release / find. The app owns the real sockets; this owns which slot a connection lives in and which to reclaim under pressure. No heap/stdlib. Default off.">Socket Pool</a></td>
  <td align="center"><a href="FEATURES.md#tls" title="TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only). When set, the server can accept TLS connections using mbedTLS configured with MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DWS_TLS_ARENA_SIZE) - no system heap, so the determinism guarantee is preserved. The TLS engine is compiled only on Arduino/ESP32 (mbedTLS is not part of the native build). Default off.">TLS</a></td>
  <td align="center"><a href="FEATURES.md#tls-policy" title="Opt-in TLS version negotiation + pinned cipher-suite policy. A policy layer on top of the mbedTLS-backed transport TLS (which already runs the 1.2 / 1.3 record + handshake): services/tls_policy pins the version to an audited [min,max] and makes the negotiated version observable (dws_tls_negotiate_version / dws_tls_version_name), and pins the cipher suites to an audited allowlist selected by server preference (dws_tls_select_cipher), with an AEAD-only classifier (dws_tls_is_aead) for a hardened profile. Pure, host-tested; the app feeds the results to the mbedTLS config. Default off.">TLS Policy</a></td>
  <td align="center"><a href="FEATURES.md#tls-resumption" title="TLS session resumption via RFC 5077 session tickets (requires TLS). Default off. When set, the TLS 1.2 server issues encrypted session tickets and accepts them on reconnect, so a returning client completes an abbreviated handshake (no certificate or full key exchange) - much faster and far less CPU than the ~RSA/ECDHE full handshake. Resumption is stateless: the session state lives in the client's ticket, sealed with a server-held key, so there is no growing per-session cache (the determinism / zero-heap-growth guarantee holds; only a small fixed ticket key and a little arena headroom are added). The ticket key rotates automatically on the DWS_TLS_TICKET_LIFETIME_S schedule. Needs the mbedTLS build to provide MBEDTLS_SSL_TICKET_C (stock arduino-esp32 does).">TLS Resumption</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Session (L5)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ssh" title="SSH server support (RFC 4253/4252/4254). Channels are multiplexed per connection (`DWS_SSH_MAX_CHANNELS`, default 1), each routed by its recipient id with its own flow-control window. Beyond `session` channels, `direct-tcpip` (the `ssh -L` local-forward request) is parsed and routed through a normalized forwarding seam: the codec extracts the target host:port and routes channel data but does no TCP I/O itself. With `DWS_SSH_PORT_FORWARD` set, the `ssh_forward` owner plugs into that seam and does the I/O - it opens the outbound TCP through the client transport (dws_client) and bridges bytes both ways, with an optional target policy callback, a per-poll target-to-client pump bounded by the channel window, and EOF/CLOSE propagation. Forwarding is opt-in twice over (compiled out by default, and inert until the app calls `dws_ssh_forward_begin()`) because any authenticated client could otherwise reach arbitrary hosts; with it off every `direct-tcpip` open is refused (no open relay).">SSH</a></td>
  <td align="center"><a href="FEATURES.md#ssh-compression" title="SSH `zlib@openssh.com` / `zlib` compression (RFC 4253 sec 6.2) for the SERVER-&gt;CLIENT direction. When set (and SSH is on) the server advertises `zlib@openssh.com` (delayed, OpenSSH's default) and `zlib` for the s2c direction; once active it compresses every outbound packet payload through a single context-takeover DEFLATE stream (network_drivers/presentation/ssh/transport/ssh_zlib) - a persistent sliding window carried across packets, sync-flushed per packet, wrapped as a zlib stream - which a standard `inflate()` (OpenSSH) decodes. `zlib@openssh.com` starts after `SSH_MSG_USERAUTH_SUCCESS`; plain `zlib` starts at NEWKEYS. Client-&gt;server stays `none`: SSH negotiates each direction independently, and the inbound direction (keystrokes / uploads to the device) is tiny and, because OpenSSH compresses outbound with `Z_PARTIAL_FLUSH`, would need a much larger resumable inflate engine for little gain. The compressor is a single owner (network_drivers/presentation/ssh/transport/ssh_comp) that the packet layer asks per packet. PSRAM-class (~48 KB/connection): on ESP32 set `DWS_SSH_ZLIB_IN_PSRAM=1` on a PSRAM board or `DWS_SSH_ZLIB_ACK_DRAM=1` to accept the internal-DRAM cost. Default off. HW-verified against OpenSSH 10.3 (`ssh -o Compression=yes`).">SSH Compression</a></td>
  <td align="center"><a href="FEATURES.md#ssh-scp" title="SCP server over SSH - the legacy rcp protocol via `exec &quot;scp -t &lt;path&gt;&quot;` (requires SSH + File Serving). Default off. `scp localfile admin@device:/path` drops a file onto the `fs::FS` mount bound by `dws_ssh_scp_begin(fs, root)`: the SINK direction (client -&gt; device). The binding drives the rcp handshake over the exec channel - send a ready ack, read the `C&lt;mode&gt; &lt;size&gt; &lt;name&gt;` control line, ack it, stream the data straight to the file, read the trailing end-of-record byte, final ack - reusing the SFTP fs seam + the `..` traversal guard; error/ack bytes ride the main channel (no CHANNEL_EXTENDED_DATA). One file per transfer (no `-r`). The rcp codec (services/scp) is pure and host-tested (native_scp: command + control-line parse/build, malformed rejection); the binding is HW-verified against the OpenSSH `scp` client on an ESP32-S3 with an SD card (a 60 KB upload byte-exact). Follow-ups: the SOURCE direction (`scp device:/path localfile`, device -&gt; client - use `sftp get` meanwhile), recursive directory transfers, and the graceful teardown noted under SSH SFTP.">SSH SCP</a></td>
  <td align="center"><a href="FEATURES.md#ssh-sftp" title="SFTP v3 server subsystem over SSH (draft-ietf-secsh-filexfer-02; requires SSH + File Serving). Default off. A client's `subsystem &quot;sftp&quot;` request opens an SFTP session and the device serves files from an `fs::FS` mount (SD / LittleFS / SPIFFS) bound via `dws_ssh_sftp_begin(fs, root)` - the standards-track southbound path for securely dropping files (e.g. NC / G-code programs) onto the device over the one authenticated SSH port. The channel layer recognizes the subsystem request, tags the channel, and routes its bytes to the binding; the binding accumulates `SSH_FXP_*` request packets from the channel byte stream and executes each against the filesystem, reusing the same file operations WebDAV and the static file server use: OPEN/CLOSE, READ (a short DATA the client re-requests), WRITE (streamed straight to the file as fragments arrive, so a transfer is never buffered whole), OPENDIR/READDIR (with `ls -l`-style longnames), STAT/LSTAT/FSTAT, MKDIR/RMDIR/REMOVE/RENAME, REALPATH, and SETSTAT (accepted, best-effort). Handles are a fixed BSS table (`DWS_SFTP_MAX_HANDLES`), 4-byte indices; every path is checked for `..` traversal via the shared `server/fs_path.h` guard before any filesystem call; zero heap beyond the fs layer's file handles. The wire codec (services/sftp) is pure and host-tested (native_ssh_sftp: frame round-trips, ATTRS + age of every field, length-framing, longname); the fs binding is HW-verified against the OpenSSH `sftp` client on an ESP32-S3 with an SD card (mkdir / put / ls / get / rename / rm, a 60 KB file byte-exact). Note: a `READ` returns at most one SSH packet of data (`DWS_SFTP_MAX_READ`, kept within `SSH_PKT_BUF_SIZE`); raise both for throughput. Follow-ups: the SCP SOURCE (download) direction and a graceful `SSH_MSG_DISCONNECT` on teardown.">SSH SFTP</a></td>
  <td align="center"><a href="FEATURES.md#telnet" title="Telnet server support (RFC 854 / IAC option negotiation).">Telnet</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Presentation (L6)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#auth" title="HTTP Basic Authentication per-route.">Auth</a></td>
  <td align="center"><a href="FEATURES.md#auth-lockout" title="Opt-in per-IP brute-force lockout for HTTP auth (requires AUTH). Default off (zero cost / no behavior change). When set, the auth gate counts consecutive failed authentications per source IPv4 in a fixed BSS table; after DWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for DWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to DWS_AUTH_LOCKOUT_MAX_MS. A locked address gets 429 (Retry-After) with no credential check; a successful auth clears it. Bounded memory (no heap); the table evicts idle, then least-recently-used, addresses when full.">Auth Lockout</a></td>
  <td align="center"><a href="FEATURES.md#cbor" title="Zero-heap CBOR (RFC 8949) encoder for compact binary payloads. Default off. When set, network_drivers/presentation/cbor/cbor.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, null, and float32 into a caller-provided buffer - a compact binary alternative to the JSON writer for telemetry. Pure, no heap, host-tested against the RFC 8949 vectors.">CBOR</a></td>
  <td align="center"><a href="FEATURES.md#cloudevents" title="CloudEvents v1.0 (CNCF) event envelope. Default off. services/cloudevents makes a device's events interoperable with serverless / event-mesh consumers: `dws_cloudevents_build_json()` emits a structured `application/cloudevents+json` envelope (the required `id` / `source` / `type` plus optional `subject` / `datacontenttype` / `data`) over the JSON writer, and `dws_cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers. Pure and host-tested. See src/services/cloudevents.h.">CloudEvents</a></td>
  <td align="center"><a href="FEATURES.md#http-delivery" title="Opt-in HTTP delivery optimizations that make a slow origin acceptable to a browser. Default off. **Stale-while-revalidate (RFC 5861):** `dws_delivery_swr` decides FRESH / serve-stale-and-revalidate / EXPIRED from an age against `max-age` + `stale-while-revalidate`, and `dws_delivery_cache_control` builds the matching header - wired into serving by `DWS::set_cache_control_swr(max_age_s, swr_s)`, so every `serve_file` / `serve_static` response carries `public, max-age=N, stale-while-revalidate=M` and the header can never drift from the decision. **Service worker:** `dws_delivery_sw_manifest` emits the versioned `{&quot;version&quot;:..,&quot;precache&quot;:[..]}` document, and `dws_delivery_serve_sw(srv, paths, n, version)` registers `/sw.js` (a flash-resident worker shipped through the web-asset pipeline) plus `/precache.json`; the worker precaches the shell and then serves it stale-while-revalidate client-side, naming its cache after the version so a bump invalidates the old shell exactly once, and the manifest route answers 500 rather than serving truncated JSON if it would not fit `DWS_DELIVERY_MANIFEST_BUF` (`DWS_DELIVERY_PRECACHE_MAX` paths). **Byte ranges are NOT in this service** - `server/http_range.h` (`http_parse_byte_range`, `DWS_ENABLE_RANGE`) is the single owner of the RFC 7233 range math and is already wired into static file serving and the edge cache, emitting `Accept-Ranges`, the 206 `Content-Range`, and a 416 `bytes */size`; a duplicate parser here was removed rather than given a second call site. Pure cores host-tested (`native_http_delivery`) and **HW-verified on an ESP32-P4 serving from SD**: `bytes=10-19` / `bytes=-5` / `bytes=995-` each returned byte-exact 206 payloads with the right `Content-Range`, an out-of-range request returned 416 `bytes */1000`, and served files carried the SWR header. Example HttpDelivery. See src/services/http_delivery/http_delivery.h.">HTTP Delivery</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#http11-parser" title="RFC 7230 request parser - validates method, path, header names and values byte-by-byte before storing anything. Always on.">HTTP/1.1 Parser</a></td>
  <td align="center"><a href="FEATURES.md#http2" title="HTTP/2 (RFC 9113) over the version-agnostic request/response core. Default off. Negotiated by TLS ALPN (&quot;h2&quot;, falling back to &quot;http/1.1&quot;); an h2 connection speaks the binary framing + HPACK header compression on top of the same routes and handlers as HTTP/1.1 (the response serializer branches on the connection's protocol). The three layers are pure and host-tested against the RFCs: **HPACK** (RFC 7541 - static + dynamic table, prefix-integer / string / canonical-Huffman coding; `native_hpack` vs the Appendix C vectors), the **frame layer** (RFC 9113 sec 4/6 - the 9-byte header + SETTINGS / WINDOW_UPDATE / RST_STREAM / GOAWAY / PING / HEADERS / DATA; `native_h2frame`), and the **connection/stream engine** (preface, SETTINGS exchange, per-stream HEADERS-&gt;request + DATA/flow-control, PING/RST/GOAWAY, reassembly; `native_h2conn` drives a full request/response cycle). A connection's engine is ~28 KB, so it does not fit internal DRAM alongside TLS: **HTTP/2 requires PSRAM** (set DWS_H2_POOL_IN_PSRAM=1 on an S3 / P4 / WROVER; a compile-time guard enforces it). See src/network_drivers/presentation/http2/.">HTTP/2</a></td>
  <td align="center"><a href="FEATURES.md#http3" title="HTTP/3 (RFC 9114) over QUIC (RFC 9000) - implemented, host-tested end-to-end. Default off. HTTP/3 runs over QUIC (a reliable transport over UDP) with QPACK (RFC 9204) header compression and its own binary framing. The full stack is in place and exercised by a host end-to-end test: a QUIC client completes the TLS 1.3-in-QUIC handshake, sends a QPACK-encoded HTTP/3 GET, and verifies the 1-RTT response - covering the QUIC variable-length integer (RFC 9000 sec 16), packet protection + framing, the TLS 1.3 handshake, the transport connection engine, and the HTTP/3 + QPACK codecs. On-device (ESP32) HW verification and an example are still pending. Like HTTP/2 this is a PSRAM-class feature.">HTTP/3</a></td>
  <td align="center"><a href="FEATURES.md#json" title="Zero-heap JSON writer/reader (json.h) for request bodies and responses. Always on.">JSON</a></td>
  <td align="center"><a href="FEATURES.md#jwt" title="JWT bearer-token authentication (HS256). Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer &lt;jwt&gt;` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a stateless token. Signature verification is constant-time.">JWT</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#messagepack" title="Zero-heap MessagePack encoder and decoder for compact binary payloads. Default off. When set, network_drivers/presentation/msgpack/msgpack.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, nil, and float32 into a caller-provided buffer, plus a cursor decoder (dws_msgpack_peek / dws_msgpack_read_\*, no-copy strings) over a caller buffer - the MessagePack-format sibling of the CBOR / JSON readers and writers. Pure, no heap, host-tested against the spec encodings and round-trip.">MessagePack</a></td>
  <td align="center"><a href="FEATURES.md#multipart" title="multipart/form-data body parser.">Multipart</a></td>
  <td align="center"><a href="FEATURES.md#protobuf" title="Protocol Buffers wire codec. Default off. services/protobuf is a zero-heap streaming Protobuf encoder + cursor reader over caller buffers (the same shape as the CBOR / MessagePack codecs): a writer for varint / ZigZag / fixed32 / fixed64 / length-delimited fields (`dws_pb_uint64`, `dws_pb_sint64`, `dws_pb_fixed32`, `dws_pb_fixed64`, `dws_pb_float`, `dws_pb_double`, `dws_pb_bytes`, `dws_pb_string`; embedded messages are built into a sub-buffer and added with `dws_pb_bytes`), and a reader (`dws_pb_read_field`) that decodes one field at the buffer head and reports bytes consumed, with ZigZag / float / double value decoders. Host-tested against the spec vectors. This is the standalone Protobuf deliverable; gRPC (framed Protobuf over HTTP/2) is gated on the HTTP/2 roadmap item. See src/services/protobuf/protobuf.h.">Protobuf</a></td>
  <td align="center"><a href="FEATURES.md#senml" title="SenML (RFC 8428) measurement-pack builder. Default off; implies CBOR. services/senml is a zero-heap SenML-JSON + SenML-CBOR encoder over the shipped JSON / CBOR writers: the caller fills a `SenmlRecord` array (optional base name / base time, name, unit, one value - number / string / boolean, optional time) and `dws_senml_json_build` / `dws_senml_cbor_build` emit the whole pack. SenML-JSON uses the text labels (bn/n/u/v/vs/vb/t); SenML-CBOR uses the integer labels (n=0, u=1, v=2, ..., bn=-2, bt=-3). Numbers that are integral are emitted as integers so timestamps keep full precision. The standard measurement format for CoAP / LwM2M / HTTP telemetry; verified against the RFC example, host-tested. See src/services/senml/senml.h.">SenML</a></td>
  <td align="center"><a href="FEATURES.md#sse" title="Server-Sent Events push support.">SSE</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#web-terminal" title="Browser &quot;web serial&quot; terminal over WebSocket (src/services/web_terminal). Serves a self-contained terminal page and a WebSocket endpoint: device output is broadcast to all connected browsers, browser input is delivered to a command callback. Requires WEBSOCKET. Default off.">Web Terminal</a></td>
  <td align="center"><a href="FEATURES.md#websocket" title="WebSocket support (RFC 6455 framing + SHA-1/base64 handshake).">WebSocket</a></td>
  <td align="center"><a href="FEATURES.md#ws-deflate" title="WebSocket permessage-deflate (RFC 7692) - bidirectional compression. When set (and WEBSOCKET is on), the server negotiates the `permessage-deflate` extension and both decompresses inbound compressed (RSV1) messages via a bounded INFLATE (network_drivers/presentation/inflate._) and compresses outbound data frames via a bounded DEFLATE (network_drivers/presentation/deflate._); both borrow their table scratch from the shared per-dispatch arena. The extension is negotiated with `{client,server}_no_context_takeover` so every message (de)compresses independently - no window is carried between messages. An outbound frame that would not shrink is sent uncompressed (the per-message RSV1 flag permits this). Default off.">WS Deflate</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Web &amp; HTTP</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#chunked-responses" title="Streaming / chunked responses of unbounded length in constant memory via send_chunked(). Always on.">Chunked Responses</a></td>
  <td align="center"><a href="FEATURES.md#cors" title="Cross-origin resource sharing with automatic preflight handling. Always on.">CORS</a></td>
  <td align="center"><a href="FEATURES.md#dashboard" title="Real-time SVG dashboard (DASHBOARD; requires SSE). Default off. Serves a self-contained, hand-rolled SVG dashboard page whose widgets are declared in a fixed compile-time DWSWidget table (zero-heap, deterministic). The page fetches the widget layout as JSON and subscribes to an SSE stream of live values; dws_dashboard_set() + dws_dashboard_publish() push the current readings. The widget-table -&gt; JSON serializers are host-testable; WebSocket controls are a follow-up.">Dashboard</a></td>
  <td align="center"><a href="FEATURES.md#edge-cache" title="Opt-in CDN edge-cache tier (requires HTTP Cache). Default off. services/edge_cache is the caching reverse-proxy edge that services/httpcache is the origin-side groundwork for: the device sits in front of a remote upstream origin, fetches a response once, and serves subsequent hits from a bounded local store - honoring `Cache-Control` / `Expires` / `ETag` / `Last-Modified`, revalidating stale entries with conditional requests (`If-None-Match` / `If-Modified-Since` -&gt; 304), and serving `Range` / `206 Partial Content` straight from the cache with the existing constant-memory send-pump. A two-tier store: bounded RAM (L1, hot, LRU + TTL) plus an optional dbm/WAL-backed SD tier (L2, persistent across reboot, when DBM is enabled). A miss or a stale-entry revalidation fetches the origin asynchronously - the client request is suspended and resumed from the server poll loop, so the worker never stalls - and every failure path (miss, full cache, origin down, oversize) fails open to the origin. The deterministic cache key is method + host + path (+ optional query), with a SHA-256 digest for the L2 key and `Vary` handled as a secondary key; explicit purge (single + prefix/wildcard) and stats round it out. Registered as a middleware via `dws_edge_cache_enable(server)` + `dws_edge_cache_map(prefix, origin)`; zero heap, all buffers fixed. The freshness/validator/key/store logic is pure and host-tested (native_edge_cache); the async origin fetch + serve are HW-verified against a real origin; the L2 SD tier's entry serialization, spill/promote, reboot survival, and purge are host-tested over a RAM WalDev (native_edge_cache_sd) via `dws_edge_cache_bind_sd`. When `DWS_ENABLE_RANGE` is set, a cached object also answers a single-range `Range` request with `206 Partial Content` + `Content-Range` (or `416`) and advertises `Accept-Ranges: bytes`, streaming just the window through the same shared parser (server/http_range) the file server uses.">Edge Cache</a></td>
  <td align="center"><a href="FEATURES.md#edge-cache-mesh" title="Opt-in mesh (sibling-cache) distribution for the edge cache (requires Edge Cache). Default off. Lets a fleet of edge nodes share one warm cache: on a full local miss a node queries its configured sibling peers (over a plaintext `ConnProto::PROTO_MESH` TCP link) before hitting the origin, and pulls a fresh copy from whichever peer has it - so the origin is fetched once per fleet, not once per node. Pull (read-through) only: no push, no invalidation protocol, no cross-node consistency window - a stale sibling copy self-expires by its own TTL and the requester re-checks freshness on arrival. The transfer carries the object plus its freshness/age (RFC 9111 §4.2.3 age propagation, reusing the same `edge_current_age()` math as the single-node serve), so a sibling-fresh object serves for its remaining lifetime with zero origin contact. The wire frame reuses the shared `edge_sd` entry serializer plus a small timing trailer; the requester ships a bounded snapshot of its request headers so the peer re-runs the exact `edge_store_find` Vary matcher (a header past `DWS_MESH_HDRS_MAX` degrades to a safe miss, never wrong content). A serving node (`dws_edge_cache_mesh_serve`, after `server.listen(port, ConnProto::PROTO_MESH)`) answers only from its LOCAL cache - one hop, never re-querying its own origin or peers, so the fleet cannot loop. The peer query runs as a pre-origin phase of the same async fetch slot (reusing that slot's origin buffer, so no extra per-slot memory) and is pumped from the server poll loop, so it never stalls the worker; a peer MISS or an exhausted peer list transitions the slot to the ordinary origin fetch. Peers are a static list (`dws_edge_cache_add_peer(host, port)`, up to `DWS_MESH_MAX_PEERS`, tried in series, first hit wins). The wire codec + peer-query state machine are pure and host-tested (native_edge_mesh: frame round-trips, age propagation, HIT/MISS/timeout/close), the two-node integration is HW-verified on ESP32-S3 rigs (example 80). Follow-ups: a TLS sibling link, UDP-broadcast peer auto-discovery, and push replication with invalidation.">Edge Cache Mesh</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#etag" title="Conditional GET via ETag for served files. When set, serve_file()/serve_static() emit a strong `ETag` (derived from the file size + last-modified time) and answer a matching `If-None-Match` with `304 Not Modified`, saving bandwidth on repeat fetches of static assets.">ETag</a></td>
  <td align="center"><a href="FEATURES.md#file-serving" title="Static file serving via Arduino FS (LittleFS, SPIFFS, SD).">File Serving</a></td>
  <td align="center"><a href="FEATURES.md#http-cache" title="HTTP `Cache-Control` directive helpers (RFC 9111 + RFC 8246 + RFC 5861). Default off. services/httpcache is the origin-side of edge caching so app routes emit correct, edge-cacheable responses (the groundwork for the CDN roadmap; the caching tier itself is a separate piece). `cache_control_build` serializes a `DWSCacheControl` struct into the canonical directive string (hand it to `set_cache_control()`), with first-class presets for the common cases - `cache_immutable_asset` (`public, max-age=..., immutable`), `cache_shared` (distinct browser vs shared-cache TTL via `s-maxage`), `cache_revalidatable` (`stale-while-revalidate`), and `cache_no_store`. `cache_control_parse` is a tolerant reader (case-insensitive names, bare or quoted delta-seconds, unknown directives ignored) covering the response directives plus `no-cache` / `must-revalidate` / `proxy-revalidate` / `must-understand` / the extensions and the request directives (`only-if-cached` / `max-stale` / `min-fresh`). `cache_freshness_lifetime` implements the RFC 9111 sec 4.2.1 first-match precedence (s-maxage for a shared cache, then max-age, then `Expires` minus `Date`). Directive set + freshness precedence verified against RFC 9111; pure and host-tested with a build-&gt;parse round-trip. See src/services/httpcache/httpcache.h.">HTTP Cache</a></td>
  <td align="center"><a href="FEATURES.md#middleware" title="Composable use() pipeline with a fixed-window rate limiter. Always on.">Middleware</a></td>
  <td align="center"><a href="FEATURES.md#range" title="HTTP Range requests / 206 Partial Content for served files. Default off. When set (requires FILE_SERVING), serve_file() / serve_static() honor a single-range `Range: bytes=...` request header: they answer `206 Partial Content` with a `Content-Range` header and stream only the requested bytes (seeking the file to the start offset), advertise `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range with `416 Range Not Satisfiable`. This enables resumable downloads and media seeking. Multi-range (multipart/byteranges) requests are not supported - the server falls back to a full 200 response, which is RFC 7233 §3.1 compliant.">Range</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#routing" title="Exact, wildcard (/*), :param path parameters, bounded allocation-free regex routes, and per-interface STA/softAP route filters. Always on.">Routing</a></td>
  <td align="center"><a href="FEATURES.md#spa-router" title="Opt-in single-page-app micro-routing decision. When set, services/spa_router provides dws_spa_route(): given a request path it returns whether to serve a real asset file, serve the SPA shell (index.html) for a client-side route, or pass through to the app's handlers under an API prefix - so a single-page UI's client routing works. Pure decision core (the caller wires the result into serve_static / the router). Default off.">SPA Router</a></td>
  <td align="center"><a href="FEATURES.md#templating" title="{{var}} response templating via send_template(). Always on.">Templating</a></td>
  <td align="center"><a href="FEATURES.md#themes" title="Embed the theme stylesheet library as runtime-selectable blobs (default off). Off by default: build-time theme injection (`&lt;!--#theme NAME--&gt;`) costs nothing extra, but embedding the whole library for runtime switching links every theme's CSS into flash (~1 KB each). When set, application/binary_asset_blobs.{h,cpp} exposes `dws_theme_css(name)` + the registry `DWS_THEME_BLOBS`, so a route (e.g. `/themes/&lt;name&gt;.css`) or a picker can switch themes live. Regenerate with `src/web/wizard/gen_theme_blobs.py` after adding a theme.">Themes</a></td>
  <td align="center"><a href="FEATURES.md#upload" title="Streaming file upload: POST a body straight to a file on the filesystem. Default off. When set, src/services/upload_service.h registers a POST route that streams the request body directly into an Arduino FS file (LittleFS / SPIFFS / SD) - the upload never has to fit in RAM. Reuses the same parser streaming-body hook as OTA. For reliable uploads set RX_BUF_SIZE above the largest inbound TCP segment (TCP_MSS, ~1460): the transport refuses-and-redelivers a segment that will not fit the receive ring (lossless backpressure), but a ring smaller than one segment would stall. The 1024 default suits ordinary requests, not uploads.">Upload</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#webdav" title="WebDAV server (RFC 4918, class 1 + advisory locks) over the file system. Default off. When set (requires FILE_SERVING), dav() mounts an FS subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1), PROPPATCH, GET, HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK - so a client (rclone, cadaver, curl, or a mounted network drive) can browse and edit files. PROPFIND returns a 207 Multi-Status document built into a fixed buffer (DWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at DWS_WEBDAV_MAX_ENTRIES children. PROPPATCH returns a 207 with each requested property refused 403 Forbidden (read-only live properties, no dead-property store) so Explorer/Finder - which PROPPATCH a timestamp right after a PUT - do not error on a 405. PUT streams the request body straight to the file (via the shared streaming-body sink), so an upload is not bounded by BODY_BUF_SIZE. Locks are advisory (a synthetic token is issued but not enforced). See docs/SECURITY.md before exposing it.">WebDAV</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Auth, Identity &amp; Security</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#audit-log" title="Tamper-evident audit log. Default off. services/audit_log keeps an append-only, hash-chained security log: each record carries SHA-256(prev_hash || fields), so altering, deleting, or reordering any retained record breaks the chain (dws_audit_verify() detects it). Storage is a fixed RAM ring of DWS_AUDIT_LOG_ENTRIES records (no heap); when it wraps, a moving anchor keeps the retained window verifiable. Install a sink (dws_audit_set_sink) to forward every record at creation time to a durable / remote store - SD-card file, syslog or HTTP log service, serial console - preserving the same chain off-device. Pure and host-tested.">Audit Log</a></td>
  <td align="center"><a href="FEATURES.md#csrf" title="Opt-in CSRF protection for state-changing HTTP requests. Default off (zero cost / no behavior change). When set, every POST / PUT / PATCH / DELETE must carry a valid `X-CSRF-Token` header (a stateless, HMAC-signed token); requests without one get 403 Forbidden. GET / HEAD / OPTIONS are exempt (they are not state-changing). Clients fetch a token from the built-in `GET /csrf` endpoint, which also sets it as the `csrf` cookie. No server-side session storage - the token self-validates against an HMAC secret seeded from the hardware RNG at begin(); it is independent of AUTH.">CSRF</a></td>
  <td align="center"><a href="FEATURES.md#oauth2" title="OAuth2 token-endpoint client. Default off. services/oauth2 obtains tokens - the counterpart to the OIDC ID-token verifier. It builds the percent-encoded form body for the authorization_code and refresh_token grants (RFC 6749), supporting a confidential client (client_secret) or a public client with PKCE (code_verifier, RFC 7636), and parses the JSON token response (reusing the zero-heap JSON reader). The build + parse core is pure and host-tested; the POST to the token endpoint uses the HTTP(S) client (needs HTTP_CLIENT). No heap, no stdlib.">OAuth2</a></td>
  <td align="center"><a href="FEATURES.md#oidc" title="OpenID Connect ID-token verification, RS256. Default off. services/oidc verifies an OIDC ID token (JWT) as a relying party: requires alg RS256, selects the issuer key by kid from a JWKS, verifies the RSASSA-PKCS1-v1.5 SHA-256 signature (real RSA modexp via ssh_rsa, mbedTLS- accelerated on ESP32), and checks iss / aud / exp / nbf, extracting sub / email. Pure and host-tested; the caller fetches + caches the JWKS over HTTPS (off the request hot path) and passes the JSON in. Builds on the SSH RSA primitive, not the HS256 JWT module (services/jwt), so the two are independent.">OIDC</a></td>
  <td align="center"><a href="FEATURES.md#totp" title="Opt-in TOTP two-factor auth (RFC 6238). Default off. services/totp computes and verifies time-based one-time passwords (HMAC-SHA1 over the existing SHA-1, Google Authenticator compatible) and decodes base32 shared secrets, for a second factor on top of password / JWT auth. Pure and host-tested against the RFC 6238 vectors; the verifier checks a +/- step window for clock skew.">TOTP</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">IoT, Messaging &amp; APIs</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#amqp" title="AMQP 0-9-1 frame codec - the RabbitMQ wire protocol. Default off. services/amqp lets a device be an AMQP client over the outbound client transport: `dws_amqp_protocol_header` writes the `&quot;AMQP&quot; 0 0 9 1` preamble, `dws_amqp_build_frame` / `dws_amqp_parse_frame` build and validate a frame (type + channel + 4-octet size + payload + the mandatory 0xCE frame-end), `dws_amqp_build_method` / `dws_amqp_parse_method` handle a METHOD frame's class-id / method-id / arguments, and `dws_amqp_build_heartbeat` emits a keep-alive. Pure and host-tested; the method-argument field encoding and the connection state are the application's. See src/services/amqp/amqp.h.">AMQP</a></td>
  <td align="center"><a href="FEATURES.md#coap" title="CoAP server (RFC 7252) over UDP/5683. A zero-heap Constrained Application Protocol endpoint: a fixed resource table dispatched against the request's Uri-Path, with a pure host-testable message codec (parse/build) and an ESP32 UDP binding via the transport-layer UDP service. Default off; the codec is otherwise unit-tested standalone (env:native_coap).">CoAP</a></td>
  <td align="center"><a href="FEATURES.md#coap-block" title="CoAP block-wise transfer - RFC 7959 (requires COAP). Default off. When set, the server understands the Block2 (descriptive, responses) and Block1 (control, request uploads) options: - Block2: a representation larger than one block, or any GET that carries a Block2 option, is served one block at a time. A constrained client requests a small block size (SZX) and pages through with ascending block numbers; the server re-renders the (idempotent) resource and slices out the asked-for block, setting the More bit until the last. - Block1: a POST/PUT payload larger than one block is reassembled into a single BSS buffer. Each non-final block is acknowledged 2.31 Continue; the final block dispatches the handler with the whole reassembled payload. One block-wise transfer is reassembled at a time (deterministic, single buffer); an out-of-order or oversized block yields 4.08 / 4.13. Size1/Size2 options and the /.well-known/core listing are out of scope.">CoAP Block</a></td>
  <td align="center"><a href="FEATURES.md#coap-observe" title="CoAP resource observation - RFC 7641 (requires COAP). Default off. When set, a client GET with the Observe option registers as an observer of a resource; the application calls dws_coap_notify(path) to push the resource's current representation to every observer (a CoAP notification from the server port with an increasing Observe sequence). Observers are dropped on a deregister GET, a client RST, or send failure.">CoAP Observe</a></td>
  <td align="center"><a href="FEATURES.md#dds-rtps" title="Opt-in DDS / RTPS wire-protocol codec. When set, services/dds provides the RTPS (DDSI-RTPS) message + submessage framing: the 20-octet header (magic / version / vendor / guidPrefix) and the typed submessages (INFO_TS, DATA, HEARTBEAT, ACKNACK, ...) with the endianness flag, built by dws_rtps_header / _submessage and walked by dws_rtps_parse. Pure framing (CDR payloads + SPDP/SEDP discovery layer on top). Default off.">DDS-RTPS</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#graphql" title="GraphQL query subset. Default off. services/graphql parses a GraphQL query into a fixed AST node pool (no heap) and emits a `{&quot;data&quot;:{...}}` response shaped exactly by the requested selection. Schema-free: a field with a sub-selection is an object (the engine recurses), a leaf field calls your single resolver, and arguments collected along the path are handed to it. Supports nested selections, field arguments, and the anonymous / `query` forms; mutations, subscriptions, fragments, and variables are out of scope. Pure and host-tested; bounds are compile-time (DWS_GQL_* in ServerConfig.h). Serve it from a POST /graphql route.">GraphQL</a></td>
  <td align="center"><a href="FEATURES.md#grpc-web" title="gRPC-Web message framing. Default off. services/grpcweb is a zero-heap length-prefixed frame builder + parser for gRPC-Web, the HTTP/1.1-reachable subset of gRPC (gRPC proper needs HTTP/2): `dws_grpcweb_frame_message` wraps a Protobuf message in the 5-octet `[flags][length BE32]` prefix, `dws_grpcweb_frame_trailer` emits the 0x80 trailers frame (`grpc-status` / `grpc-message`), and `dws_grpcweb_parse` reads one frame back (with `dws_grpcweb_trailer_status` to pull the status out of a trailers frame). Wraps the Protobuf codec (DWS_ENABLE_PROTOBUF) and rides the shipped HTTP/1.1 server/client. Pure and host-tested. See src/services/grpcweb/grpcweb.h.">gRPC-Web</a></td>
  <td align="center"><a href="FEATURES.md#lwm2m" title="OMA LwM2M TLV codec. Default off. services/lwm2m is a zero-heap writer + cursor reader for the LwM2M `application/vnd.oma.lwm2m+tlv` resource encoding carried over the shipped CoAP service for device management: `dws_lwm2m_tlv_write` (with typed `dws_lwm2m_tlv_write_int` shortest-form / `dws_lwm2m_tlv_write_bool` / `dws_lwm2m_tlv_write_string` / `dws_lwm2m_tlv_write_float` helpers), `dws_lwm2m_tlv_read`, and `dws_lwm2m_tlv_value_int`. Handles 8-/16-bit identifiers, inline / 8- / 16- / 24-bit lengths, and the Object-Instance / Resource / Multiple-Resource / Resource-Instance kinds; type-byte layout verified against the LwM2M spec. Pure and host-tested. The registration interface and the standard object model layer on top. See src/services/lwm2m/dws_lwm2m_tlv.h.">LwM2M</a></td>
  <td align="center"><a href="FEATURES.md#mqtt" title="MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS). Default off. When set, src/services/mqtt/mqtt.h provides a persistent outbound client: connect to a broker, PUBLISH (QoS 0/1/2) and SUBSCRIBE to topics, receive incoming messages via a callback, with keep-alive pings - the dominant IoT messaging pattern, for telemetry push and remote command. The packet codec is host-testable; the transport (DNS + raw lwIP TCP, MQTTS via client-side mbedTLS) is ESP32-only. Full QoS 0/1/2 (outbound DUP retransmit, inbound QoS-2 de-duplication by packet id) and Last-Will are supported.">MQTT</a></td>
  <td align="center"><a href="FEATURES.md#mqtt-sn" title="MQTT-SN v1.2 wire codec. Default off. services/mqtt/mqtt_sn is a zero-heap codec for MQTT for Sensor Networks - the UDP / non-TCP MQTT variant for constrained, lossy links (numeric topic IDs instead of strings, gateway discovery, sleeping-client keep-alive). Builders for CONNECT / REGISTER / PUBLISH / SUBSCRIBE (by name or pre-defined id) / PINGREQ / DISCONNECT / SEARCHGW, plus `dws_mqttsn_parse_header()` (the 1- and 3-octet Length forms, big-endian fields) and typed parsers for CONNACK / REGACK / PUBACK / SUBACK / PUBLISH / REGISTER, with a `dws_mqttsn_make_flags()` helper (DUP / QoS / retain / will / clean / TopicIdType). Wire bytes verified against the spec and the Eclipse Paho reference; pure and host-tested. The datagram send (dws_udp_sendto), topic-ID registry, and sleep / retransmit state are the application's. See src/services/mqtt/mqtt_sn.h.">MQTT SN</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#mqtt-tls" title="MQTTS: run the MQTT client over client-side TLS (needs TLS).">MQTT TLS</a></td>
  <td align="center"><a href="FEATURES.md#nats" title="NATS client protocol codec - the text-based NATS pub/sub messaging protocol. Default off. services/nats lets a device be a NATS client over the outbound client transport: builders for `CONNECT`, `PUB` (with optional reply-to), `SUB` (with optional queue group), `UNSUB`, `PING`, and `PONG`, plus `dws_nats_parse` which decodes an inbound `MSG` / `INFO` / `PING` / `PONG` / `+OK` / `-ERR` (a MSG yields subject / sid / reply-to / payload). Line-oriented (CRLF, space-delimited); only PUB and MSG carry a payload. Pure and host-tested; the connection and subscription state are the application's. See src/services/nats/nats.h.">NATS</a></td>
  <td align="center"><a href="FEATURES.md#sparkplug" title="Sparkplug B payload + topic codec - the Eclipse Sparkplug B industrial-IoT format over MQTT. Default off; implies PROTOBUF (the payload is a Protobuf message). services/sparkplug builds the topic (`dws_spb_build_topic` -&gt; `spBv1.0/group/type/node[/device]`) and serializes the payload over the protobuf codec: `dws_spb_build_metric` emits one Tahu Metric (name / alias / timestamp / datatype + a value - int / long / float / double / boolean / string), and `dws_spb_build_payload` wraps the timestamp + metrics + seq. Field numbers and datatype codes are verified against the Eclipse Tahu sparkplug_b.proto. Pure and host-tested; publish the payload with the MQTT client. See src/services/sparkplug/sparkplug.h.">Sparkplug</a></td>
  <td align="center"><a href="FEATURES.md#stomp" title="STOMP 1.2 frame codec. Default off. services/stomp is a zero-heap codec for the Simple/Streaming Text Oriented Messaging Protocol: `dws_stomp_build_frame()` writes a frame (command + escaped `key:value` headers + blank line + NUL-terminated body) and `dws_stomp_parse_frame()` is a non-mutating cursor reporting the command, header key/value slices, and body (honoring `content-length`, tolerating `\r\n` line endings, and skipping broker heart-beats), with `dws_stomp_header()` lookup and `dws_stomp_unescape()` for the header escapes (`\r` `\n` `\c` `\\`). Drives CONNECT / SEND / SUBSCRIBE / MESSAGE / ACK against a broker (ActiveMQ / RabbitMQ / Artemis) over the shipped outbound client transport, or STOMP-over-WebSocket via the WS client. Pure and host-tested; the connection and subscription state are the application's. See src/services/stomp.h.">Stomp</a></td>
  <td align="center"><a href="FEATURES.md#wamp" title="WAMP messaging codec. Default off. services/wamp is a zero-heap codec for the Web Application Messaging Protocol (unified RPC + PubSub over WebSocket, subprotocol `wamp.2.json`): builders for HELLO / SUBSCRIBE / UNSUBSCRIBE / PUBLISH / CALL / REGISTER / YIELD / GOODBYE (JSON arrays emitted via the shared JsonWriter - Options/Details default to `{}`, Arguments / ArgumentsKw are passed as JSON literals), plus a nesting-aware positional parser (`dws_wamp_get_type`, `dws_wamp_get_uint`, `dws_wamp_get_uri`, `dws_wamp_element`) that pulls the message type, ids, and URIs out of an inbound WELCOME / SUBSCRIBED / EVENT / RESULT / INVOCATION / ERROR. Message codes verified against the WAMP spec; pure and host-tested. It rides the shipped WebSocket layer; the session / subscription / registration tables are the application's. See src/services/wamp/wamp.h.">WAMP</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#xmpp" title="Opt-in XMPP (RFC 6120) stanza codec. When set, services/xmpp builds correctly XML-escaped `&lt;stream:stream&gt;` / `&lt;message&gt;` / `&lt;presence&gt;` / `&lt;iq&gt;` stanzas into a caller buffer and reads the stanza element name + an attribute value out of a received stanza, so a device is an IoT XMPP client. Pure text framing (TLS/SASL ride the client TLS path; the IoT XEPs layer inside `&lt;iq&gt;`). Default off.">XMPP</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Industrial &amp; Fieldbus</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ads-beckhoff" title="Beckhoff ADS / AMS protocol codec (TwinCAT PC-based control over TCP 48898). Default off. services/ads builds + parses the AMS/TCP + AMS-header frames the TwinCAT router speaks (little-endian throughout, target-before-source AMSNetId+port addressing, a command id + state flags + cbData + invoke id): `dws_ads_build_*` emit complete request frames for ReadDeviceInfo, Read, Write, ReadWrite, ReadState, WriteControl, and Add/DeleteDeviceNotification, and `dws_ads_parse_*` decode the matching responses - including the DeviceNotification stamp/sample stream (`dws_ads_parse_notification` walks every sample via a callback). ReadWrite drives symbol-by-name access (write the name to index group 0xF003 to get a handle, then read/write the value through 0xF005). AMS header field order + command ids + state-flag bits verified against the Beckhoff InfoSys AMS/ADS specification; payload layouts cross-checked with Beckhoff's own open-source ADS library, pyads, and Apache PLC4X. Pure codec, host-tested (`native_ads`); the caller owns the TCP socket and the AMS route registration on the target router. The most widely used PC-based-control protocol and a machine-tool data source via TwinCAT NC/CNC. See src/services/ads/ads.h.">ADS (Beckhoff)</a></td>
  <td align="center"><a href="FEATURES.md#bacnet" title="BACnet/IP BVLC + NPDU codec - the ASHRAE 135 building-automation network framing over UDP (47808). Default off. services/bacnet provides `dws_bvlc_build` / `dws_bvlc_parse` for the BACnet/IP virtual-link envelope (type 0x81, function such as Original-Unicast-NPDU 0x0A / Original-Broadcast-NPDU 0x0B, 2-octet length) and `dws_npdu_build` / `dws_npdu_parse` for the network layer (version 0x01 + the NPCI control octet + optional DNET/DLEN/DADR destination addressing + hop count), slicing out the APDU. Layout verified against ASHRAE 135 Annex J / Clause 6; pure and host-tested. The APDU application layer (objects / properties / services) layers on top. See src/services/bacnet/bacnet.h.">BACnet</a></td>
  <td align="center"><a href="FEATURES.md#canopen" title="CANopen (CiA 301) message codec. Default off. services/canopen is a zero-heap builder + parser for the CANopen messaging set over classic CAN frames (`shared_primitives/can.h`): NMT node control, SYNC, TIME, the heartbeat / boot-up (NMT error control), EMCY, PDO process data, and expedited SDO read / write / abort. The 11-bit COB-ID is a 4-bit function code plus a 7-bit node id, so `dws_canopen_build_*` compute it and `dws_canopen_parse` classifies a received frame back to its function + node; `dws_canopen_parse_sdo_response` decodes a server's expedited upload value, download acknowledge, or abort code. The object dictionary itself is the application's, and SDO is expedited only (segmented / block transfer is a future addition). Identifier allocation verified against CiA 301; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a CANopen field bus onto Wi-Fi. See src/services/canopen/canopen.h.">CANopen</a></td>
  <td align="center"><a href="FEATURES.md#cc-link" title="Opt-in CC-Link (CLPA) cyclic fieldbus frame codec. When set, services/cclink builds/validates the CC-Link cyclic frame ([station][command][RX/RY bit data][RWr/RWw word data][sum checksum]) a Mitsubishi CC-Link master exchanges with remote stations over RS-485, plus bit/word process-image accessors. Pure codec (the RS-485 timing + CC-Link IE Field PHY are hardware-gated). Default off.">CC-Link</a></td>
  <td align="center"><a href="FEATURES.md#cia-402" title="CiA 402 / IEC 61800-7-201 drive + motion profile over CANopen. Default off (requires CANOPEN). services/cia402 is the standardised servo / stepper drive profile: `dws_cia402_state` decodes the power state machine from the Statusword (the CiA 402 mask/value table - Not ready to switch on / Switch on disabled / Ready to switch on / Switched on / Operation enabled / Quick stop active / Fault reaction active / Fault), `dws_cia402_controlword` and `dws_cia402_enable_sequence` produce the Controlword commands (Shutdown 0x06, Switch on 0x07, Enable operation 0x0F, Quick stop 0x02, Fault reset 0x80) that walk an axis to Operation Enabled, and the `dws_cia402_sdo_set_*` / `dws_cia402_pack_command` / `dws_cia402_unpack_status` helpers write the Controlword, Modes of Operation (PP / PV / PT / HM / IP / CSP / CSV / CST), and target/actual position-velocity-torque objects (0x6040 / 0x6041 / 0x6060 / 0x607A / 0x60FF / 0x6071 / 0x6064 / ...) through the shipped CANopen SDO / PDO codec. State masks + command values + object indices verified against IEC 61800-7-201 and multiple drive vendors' tables; pure and host-tested (`native_cia402`). Turns the ESP32 CAN stack (TWAI or MCP2515) into a motion master; close the loop with a services/control PID. See src/services/cia402/cia402.h.">CiA 402</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#cip" title="CIP (Common Industrial Protocol) message codec - the message that rides inside an EtherNet/IP Unconnected Data item (DWS_ENABLE_ENIP). Default off. services/cip builds the request - `dws_cip_build_epath` encodes a class/instance/attribute EPATH from logical segments (`0x20 | type | format`, 8- or 16-bit ids), `dws_cip_build_get_attr_single` / `dws_cip_build_request` prepend the service code + path size - and `dws_cip_parse_response` splits a reply into service / general status / additional status / data. Service codes and the segment encoding are verified against the Wireshark CIP dissector. Pure and host-tested; wrap the request with `dws_eip_build_send_rr_data` for a working CIP read path. See src/services/cip/cip.h.">CIP</a></td>
  <td align="center"><a href="FEATURES.md#control" title="Closed-loop control law: a zero-heap, FPU-accelerated PID controller. Default off. services/control provides a single-precision-float PID (`pid_init` / `pid_update`) with the corrections that matter on real hardware - derivative-on-measurement (a setpoint step produces no derivative kick) with an optional single-pole low-pass, output clamping, anti-windup by conditional integration (the integrator freezes while saturated instead of winding up) plus a hard integral clamp, and a feed-forward term - and inline control-law primitives (`control_clamp` / `control_deadband` / `control_slew` / `control_lpf`). The maths is single-precision end to end so it runs on the ESP32 / ESP32-S3 FPU (`madd.s` fused multiply-add, never the soft-float double path), and `pid_update` is defined `inline` so the whole law folds into the caller. The derivative's `/dt` is the one divide - ~58 of the ~166 CCOUNT cycles per `pid_update` on the ESP32-S3, because the LX7 FPU has no divide instruction (`a/b` becomes a `__divsf3` call) - so for a fixed-rate loop `pid_set_rate(p, dt)` caches `1/dt` and `pid_update_fixed()` runs the hot path with no divide at all (**97.4 cyc, ~41% faster**; a compile-time-constant dt folds the divide away too). `pid_update_n` runs a batch of axes off one control tick. Pure maths, host-tested (`native_control`); see docs/FEATURE_PERFORMANCE.md for the bare-op and PID cycle counts. Pair it with a plant it can command (a services/cia402 drive, a dshot ESC, a heater PWM) and tune the gains offline by replaying the device's run log through `tools/pid_tune.py`. See src/services/control/control.h.">Control</a></td>
  <td align="center"><a href="FEATURES.md#cotp" title="TPKT (RFC 1006) + COTP (ISO 8073 / X.224 class 0) frame codec - the &quot;ISO transport on TCP&quot; foundation under S7comm and IEC 61850 MMS. Default off. services/cotp provides `dws_tpkt_build` / `dws_tpkt_parse` for the 4-octet TPKT envelope (version 3 + 16-bit length), `dws_cotp_build_dt` for a Data TPDU (`LI 0xF0 EOT|NR` + user data), `dws_cotp_build_cr` for a Connection Request (destination/source refs, class 0, the TPDU-size parameter, plus caller TSAP parameters), and `dws_cotp_parse` which reports the TPDU type and the DT user data or the CR/CC refs. Layout verified against RFC 1006 / ISO 8073; pure and host-tested. See src/services/cotp/cotp.h.">COTP</a></td>
  <td align="center"><a href="FEATURES.md#devicenet" title="DeviceNet link-adaptation codec. Default off. services/devicenet is the CAN-specific layer of &quot;CIP over CAN&quot;: `dws_devicenet_encode_id` / `dws_devicenet_decode_id` pack and unpack the 11-bit DeviceNet identifier as a Message Group (1..4) + Message ID + MAC ID (the four identifier ranges 0x000-0x3FF / 0x400-0x5FF / 0x600-0x7BF / 0x7C0-0x7EF), `dws_devicenet_msg_header` / `dws_devicenet_frag_octet` build the explicit-message header and fragmentation octets, `dws_devicenet_build_explicit` emits a single-frame explicit message, and `dws_devicenet_frag_feed` reassembles a fragmented body (first / middle / last, modulo-64 count) up to `DWS_DEVICENET_MSG_MAX` octets. The CIP application layer (services / EPATH / data) is the same one EtherNet/IP uses, so build the message body with the existing cip codec (`DWS_ENABLE_CIP`). Identifier allocation + fragmentation verified against the ODVA DeviceNet spec; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a DeviceNet segment onto Wi-Fi. See src/services/devicenet/devicenet.h.">DeviceNet</a></td>
  <td align="center"><a href="FEATURES.md#df1" title="Allen-Bradley DF1 full-duplex frame codec. Default off. services/df1 is a zero-heap framing + DLE byte-stuffing + BCC/CRC codec for the Rockwell serial PLC data-link layer (pub. 1770-6.5.16): `dws_df1_build_frame` wraps application data in `DLE STX ... DLE ETX` (a data byte equal to DLE/0x10 is doubled) with either a BCC (the 2's complement of the modulo-256 data sum) or a CRC-16/ARC (poly 0x8005, init 0, over the data plus the ETX, sent low byte first), and `dws_df1_parse_frame` validates the check and un-stuffs the data. Vectors verified against the manual (BCC 0x20-&gt;0xE0, CRC &quot;123456789&quot;-&gt;0xBB3D). Pure and host-tested; the PCCC application header lives inside the data. See src/services/df1/df1.h.">DF1</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#directnet" title="Opt-in AutomationDirect / Koyo DirectNET serial frame codec. When set, services/directnet builds/validates the DirectNET master-slave serial frames - the header (SOH + slave/type/address/blocks ASCII-hex + ETB + LRC) and the data frame (STX + data + ETX + LRC) - for V-memory read/write on an AutomationDirect DirectLOGIC PLC. Pure codec (the UART transport + ACK/NAK handshake are the device step). Default off.">DirectNET</a></td>
  <td align="center"><a href="FEATURES.md#dmx512" title="DMX512 + RDM (ANSI E1.20) lighting codec. Default off. services/dmx covers stage / architectural lighting over RS-485: `dws_dmx_build` and `dws_dmx_get_channel` assemble and read the positional DMX512 slot packet (a start code, 0x00 for dimmer data, followed by up to 512 channel slots - no checksum or in-frame addressing), and the RDM (Remote Device Management) functions build / parse the addressed management packet that shares the wire: `dws_rdm_build` / `dws_rdm_parse` carry 48-bit source / destination UIDs (`dws_rdm_uid`), a transaction number, a command class (GET / SET / DISCOVERY) + parameter id, optional parameter data, and the 16-bit additive checksum (`dws_rdm_checksum`), with named command-class / response-type / PID constants. RDM packet layout + checksum verified against ANSI E1.20; pure and host-tested. Drive a MAX485-class transceiver on a UART (250 kbit/s, 8N2; the break is the application's) and bridge a lighting rig onto Wi-Fi. See src/services/dmx/dmx.h.">DMX512</a></td>
  <td align="center"><a href="FEATURES.md#ethernetip" title="EtherNet/IP encapsulation codec - the ODVA EtherNet/IP transport (TCP/UDP 44818) that carries CIP. Default off. services/enip provides `dws_eip_build` / `dws_eip_parse` for the 24-octet encapsulation header (little-endian command / length / session handle / status / sender context / options), `dws_eip_build_register_session` to open a session, and `dws_eip_build_send_rr_data` / `dws_eip_parse_send_rr_data` to wrap + unwrap a CIP message as an unconnected message via the Common Packet Format (a Null Address item plus an Unconnected Data item). Commands and CPF item types verified against the Wireshark ENIP dissector; pure and host-tested. The CIP object model inside the Unconnected Data item layers on top. See src/services/enip/enip.h.">EtherNet/IP</a></td>
  <td align="center"><a href="FEATURES.md#fins" title="Omron FINS frame codec. Default off. services/fins is a zero-heap command/response builder + parser for the Factory Interface Network Service (FINS/UDP): `dws_fins_build_command` and `dws_fins_build_memory_area_read` emit the 10-octet routing header (ICF/RSV/GCT, destination + source net/node/unit, SID) plus the MRC/SRC command code and parameters, and `dws_fins_parse_command` / `dws_fins_parse_response` read them back (the response MRES/SRES end code included). Talks to an Omron PLC over the shipped UDP transport (dws_udp_sendto). Header layout verified against the FINS spec; pure and host-tested. See src/services/fins/fins.h.">FINS</a></td>
  <td align="center"><a href="FEATURES.md#hart" title="Opt-in HART / HART-IP process-instrument protocol codec. When set, services/hart provides the HART command-frame codec (build/parse with the longitudinal XOR checksum, short + long addressing) and the 8-octet HART-IP message header, so a device speaks HART over UDP/TCP 5094 (front-end-free) or, with a HART FSK modem, over the 4-20 mA loop. Pure, host-tested. Default off.">HART</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#host-link" title="Omron Host Link (C-mode) frame codec. Default off. services/hostlink is a zero-heap ASCII command/response codec for Omron's serial host-link protocol (the RS-232/485 sibling of FINS): `dws_hostlink_build` emits `@UU` + 2-char header code + text + FCS + `*`CR, and `dws_hostlink_parse` FCS-validates and splits a frame (`dws_hostlink_end_code` reads a response's end code). The FCS is the 8-bit XOR from `@` through the text, rendered as two hex digits (verified against the `@00RD00000010` -&gt; `57` example). Pure and host-tested; the UART transport is the application's. See src/services/hostlink/hostlink.h.">Host Link</a></td>
  <td align="center"><a href="FEATURES.md#interbus" title="Opt-in INTERBUS summation-frame fieldbus codec. When set, services/interbus assembles/disassembles the INTERBUS summation frame (loopback word + per-device 16-bit process-image slices + CRC-16/CCITT FCS) of the Phoenix Contact ring fieldbus, where every device is a shift-register slice of one circulating frame. Pure codec (the physical ring clocking is hardware-gated). Default off.">INTERBUS</a></td>
  <td align="center"><a href="FEATURES.md#io-link" title="IO-Link (SDCI, IEC 61131-9) data-link message codec. Default off. services/iolink implements the point-to-point smart-sensor link's data-link message layer: `dws_iol_mc` (with decoders) builds the M-sequence Control octet (read/write, communication channel, address), `dws_iol_ckt` builds a master message's checksum / M-sequence-type octet, `dws_iol_cks` builds a device reply's checksum / status octet (Event + PD-valid flags), and `dws_iol_checksum6` / `dws_iol_finalize` / `dws_iol_verify` implement the SDCI message checksum straight from the IO-Link Interface and System Specification v1.1.4 Annex A.1.6 (a 0x52 seed XORed octet by octet with the check octet's checksum bits zeroed, then the 8-to-6-bit compression of equation A.1). The checksum is verified against a vector hand-derived from the spec formula. Lay the per-type M-sequence and ISDU octets out per your device profile, then finalize / verify with this codec. Pure and host-tested. The wire is a UART through an IO-Link transceiver (MAX14819 / L6360 class); bridge sensor data onto Wi-Fi. See src/services/iolink/iolink.h.">IO-Link</a></td>
  <td align="center"><a href="FEATURES.md#lonworks" title="Opt-in LonWorks / LON-IP (ISO/IEC 14908) network-variable codec. When set, services/lonworks builds/parses the LonTalk network-variable PDU ([msg-code][14-bit selector][value]) that a building-automation device exchanges - over LON/IP (14908-4) UDP, so no Neuron chip is needed - plus the common SNVT scalar encodings (SNVT_temp, SNVT_switch). Pure codec (the UDP transport is the shipped UDP layer). Default off.">LonWorks</a></td>
  <td align="center"><a href="FEATURES.md#melsec" title="Mitsubishi MELSEC MC protocol (binary 3E frame) codec. Default off. services/melsec builds + parses the QnA-compatible binary 3E frames for MELSEC PLCs over TCP/UDP: `dws_melsec_build_read` emits a batch-read (word units) request (little-endian fields, request subheader 0x5000, command 0x0401, a device code - D 0xA8 / M 0x90 / X / Y / R / ... - plus a 24-bit head device number and a point count), and `dws_melsec_parse_response` validates the 0xD000 response and reports the end code (0x0000 = success) and the read data. Frame layout + device codes verified against a third-party MC implementation; pure and host-tested. Completes the major-vendor PLC read set alongside FINS / Host Link (Omron), DF1 (Allen-Bradley), and S7comm (Siemens). See src/services/melsec/melsec.h.">MELSEC</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#modbus" title="Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502. Default off. When set, listen(502, PROTO_MODBUS) serves a fixed data model (coils, discrete inputs, holding + input registers, all in BSS) over Modbus TCP: Read/Write Coils (FC 1/5/15), Read Discrete Inputs (FC 2), Read/Write Holding Registers (FC 3/6/16), and Read Input Registers (FC 4). The codec (MBAP framing + PDU dispatch) is pure and host-tested; the TCP transport is ESP32-only. The application reads/writes the model with the accessor functions and is notified of client writes via dws_modbus_on_write(). Modbus has no authentication or encryption - run it only on a trusted control network.">Modbus</a></td>
  <td align="center"><a href="FEATURES.md#modbus-master" title="Opt-in Modbus master codec + register scanner. Default off. services/modbus/dws_modbus_master builds Modbus TCP read-request ADUs and parses the responses (register values or exception), so an app can poll / auto-discover a slave's registers. Pure and host-tested as a full round-trip against the slave codec (dws_modbus_process_adu); the actual send is the app's TCP.">Modbus Master</a></td>
  <td align="center"><a href="FEATURES.md#modbus-plus" title="Opt-in Modbus Plus HDLC token-bus frame codec. When set, services/mbplus builds/validates the Modbus Plus HDLC frame (7E addr ctrl payload CRC-16/X-25 7E) that Schneider's token-passing peer bus exchanges, plus the token-rotation helper (next station in the logical ring). Reuses the shipped Modbus PDU model for the data. Pure codec (the 1 Mbit/s bus is hardware-gated). Default off.">Modbus Plus</a></td>
  <td align="center"><a href="FEATURES.md#modbus-rtu" title="Modbus RTU framing (serial / RS-485). Default off; implies MODBUS. Adds the RTU ADU codec `dws_modbus_rtu_process_adu()` - a `[slave addr][PDU][CRC16]` frame (CRC16-Modbus, little-endian) wrapped around the existing host-tested PDU dispatch: a CRC mismatch or a non-matching unit address is dropped silently (no reply, per the spec), and a broadcast (address 0) is processed with no reply. Pure and host-tested; feed it from a UART/RS-485 driver (the serial transport, framed by the 3.5-character inter-frame idle, is the application's). See src/services/modbus/modbus.h.">Modbus RTU</a></td>
  <td align="center"><a href="FEATURES.md#powerlink" title="Opt-in Ethernet POWERLINK (EPSG) basic frame codec. When set, services/powerlink builds/parses the EPL basic frames ([messageType][dest][source][payload]) of the isochronous managed-node cycle - SoC (start of cycle), PReq (poll request), PRes (poll response with process data), SoA (start of async) - over raw L2 (ethertype 0x88AB, on the shipped services/rawl2). Pure codec (the raw-L2 transmit + isochronous timing are the device step). Default off.">POWERLINK</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#profibus" title="Opt-in PROFIBUS-DP FDL telegram codec. When set, services/profibus builds/validates the PROFIBUS-DP FDL telegrams - SD1 (no-data: SD1 DA SA FC FCS ED) and SD2 (variable-data: SD2 LE LEr SD2 DA SA FC data FCS ED, arithmetic-sum FCS) - a Siemens DP master exchanges with slaves over RS-485 (the DP-V0 cyclic I/O exchange). Pure codec (the RS-485 timing + DP state machine are the device step). Default off.">PROFIBUS</a></td>
  <td align="center"><a href="FEATURES.md#profinet" title="Opt-in PROFINET DCP (Discovery and Configuration Protocol) frame codec. When set, services/profinet builds/parses the DCP frames (10-octet header + option/suboption blocks) PROFINET uses to discover and name IO-Devices over raw L2 (ethertype 0x8892) - Identify request/ response and Set (assign NameOfStation / IP). Pure codec (the raw-L2 transmit via services/rawl2 + esp_eth is the device step). Default off.">PROFINET</a></td>
  <td align="center"><a href="FEATURES.md#s7comm" title="Siemens S7comm PDU codec. Default off. services/s7comm builds + parses the S7-300/400 communication PDUs carried inside a COTP Data TPDU (DWS_ENABLE_COTP) over ISO-on-TCP (port 102): `dws_s7_build_setup` (Setup Communication), `dws_s7_build_read_request` (Read Var with S7-ANY items over the DB / I / Q / M areas, encoding the byte address as a 24-bit bit-address), `dws_s7_parse_header`, and `dws_s7_read_next_item` which walks the response data items honoring the length-in-bits transport sizes (BIT/BYTE/INT) and the even-item padding. All constants (protocol id 0x32, ROSCTR, function/area/transport codes) are verified against the Wireshark S7comm dissector. Pure and host-tested; wrap the PDU with `dws_cotp_build_dt` + `dws_tpkt_build`. See src/services/s7comm/s7comm.h.">S7comm</a></td>
  <td align="center"><a href="FEATURES.md#sdi-12" title="SDI-12 sensor-bus codec. Default off. services/sdi12 is a zero-heap command / response codec for the 1200-baud single-wire ASCII bus used by environmental / agricultural sensors (soil moisture, water level, weather). `dws_sdi12_build` and the `dws_sdi12_build_measure` / `_concurrent` / `_data` / `_identify` / `_ack` / `_change_address` / `_query_address` helpers emit the standard `&lt;addr&gt;&lt;command&gt;!` requests; `dws_sdi12_parse_measure` reads the `atttn` measurement response (seconds-until-ready + value count, both the 1-digit `aM!` and 2-digit `aC!` forms); `dws_sdi12_parse_values` splits a data response into floats; and `dws_sdi12_crc16` / `dws_sdi12_crc_encode` / `dws_sdi12_check_crc` implement the SDI-12 CRC (poly 0xA001, encoded as 3 printable octets) for the CRC-protected `aMC!` / `aCC!` variants. Command set + CRC verified against the SDI-12 specification; pure and host-tested. Drive the single 1200-baud line over a UART and bridge sensor readings onto Wi-Fi. See src/services/sdi12/sdi12.h.">SDI-12</a></td>
  <td align="center"><a href="FEATURES.md#sercos-iii" title="Opt-in SERCOS III motion-bus telegram codec. When set, services/sercos builds/parses the SERCOS III MDT/AT telegrams (type + phase + cycle + cyclic device data) the real-time drive/motion bus exchanges over raw L2 (ethertype 0x88CD, on the shipped services/rawl2), plus the IDN (IDentification Number) encode/decode for drive-parameter addressing. Pure codec (the isochronous timing + ring topology are hardware-gated). Default off.">SERCOS III</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#snp" title="Opt-in GE Fanuc SNP (Series Ninety Protocol) serial frame codec. When set, services/snp builds/validates the SNP master-slave serial frame ([control][length][data] [arithmetic-sum BCC]) for reading/writing registers on a GE Fanuc Series 90 (90-30/90-70) PLC over RS-485. Pure codec (the UART transport + SNP-X session are the device step). Default off.">SNP</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">SCADA, Energy &amp; Monitoring</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#c37118" title="IEEE C37.118.2 synchrophasor frame codec. Default off. services/c37118 is a zero-heap builder + CRC-validating parser for the PMU / PDC wide-area measurement wire protocol: `dws_c37118_build_frame` emits a `SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK` frame (CHK = CRC-CCITT) for any payload, `dws_c37118_build_command` handles the fixed Command frame, and `dws_c37118_parse_frame` validates the CRC and reports the frame type / id / timestamp / payload slice (with `dws_c37118_parse_command`). CRC verified against the canonical CRC-CCITT-FALSE check value; pure and host-tested. The fixed phasor configuration / data model layered on the framing is a future addition. See src/services/c37118/c37118.h.">C37.118</a></td>
  <td align="center"><a href="FEATURES.md#dnp3" title="DNP3 (IEEE 1815) data-link frame codec. Default off. services/dnp3 is a zero-heap builder + CRC-validating parser for the SCADA / utility outstation data-link layer: `dws_dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC` header block plus the CRC'd 16-octet user-data blocks, and `dws_dnp3_parse_frame` validates the header and every block CRC (CRC-16/DNP, verified against the canonical 0xEA82 check value) and de-blocks the user data. Addresses are little-endian. Pure codec, host-tested; the transport-function reassembly and the application layer (objects / function codes) layer on the de-blocked user data. See src/services/dnp3/dnp3.h.">DNP3</a></td>
  <td align="center"><a href="FEATURES.md#goose" title="Opt-in IEC 61850 GOOSE publisher codec. When set, services/goose builds the BER-encoded IECGoosePdu (gocbRef / timeAllowedToLive / datSet / goID / t / stNum / sqNum / simulation / confRev / ndsCom / numDatSetEntries / allData) and wraps it in the 8-octet GOOSE header + Ethernet frame (ethertype 0x88B8) for the fast raw-L2 substation-event publish. Pure codec (allData is a caller-encoded BER blob; the raw-L2 transmit is the device step). Default off.">GOOSE</a></td>
  <td align="center"><a href="FEATURES.md#iccp" title="Opt-in ICCP / TASE.2 (IEC 60870-6) inter-control-center telemetry codec. When set, services/iccp builds the TASE.2 Data_Value BER structures - StateQ (a discrete state + quality) and RealQ (a scaled real + quality), each with an optional timestamp - the indication points a control center transfers as MMS Reads (on the shipped services/mms + services/cotp). Pure BER codec. Default off.">ICCP</a></td>
  <td align="center"><a href="FEATURES.md#iec-60870" title="IEC 60870-5-101 / -104 telecontrol (SCADA) codec. Default off. services/iec60870 covers the utility-SCADA protocol in both transports: the -104 APCI over TCP (`dws_iec104_build_i` / `_s` / `_u` and `dws_iec104_parse` for the I / S / U formats - numbered information transfer with 15-bit send/receive sequence numbers, the supervisory acknowledge, and the STARTDT / STOPDT / TESTFR unnumbered commands), the shared ASDU header (`dws_iec_asdu_build_header` / `dws_iec_asdu_parse_header` - type id, variable structure qualifier, cause of transmission, common address) with the 3-octet Information Object Address (`dws_iec_put_ioa` / `dws_iec_get_ioa`), and the -101 FT1.2 serial link frames (`dws_iec101_build_fixed` / `_variable` and `dws_iec101_parse`, 8-bit sum checksum). Named type-id and cause-of-transmission constants are provided; the per-type information elements are the application's. Frame + APCI + ASDU layout verified against IEC 60870-5-101/-104; pure and host-tested. Run -104 over the shipped TCP stack or -101 over a UART / RS-485 transceiver to bridge an RTU or outstation onto Wi-Fi. See src/services/iec60870/iec60870.h.">IEC 60870</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#m-bus" title="Wired M-Bus (Meter-Bus, EN 13757) frame codec. Default off. services/mbus is a zero-heap builder + parser for the M-Bus link-layer frames used by utility meters (water / gas / heat / electricity): `dws_mbus_build_ack` (the single-character 0xE5), `dws_mbus_build_short` (`10 C A CS 16`), and `dws_mbus_build_long` (`68 L L 68 C A CI ... CS 16`, with convenience `dws_mbus_build_snd_nke` / `dws_mbus_build_req_ud2`), plus `dws_mbus_parse` which validates the start / stop octets, the doubled length, and the 8-bit sum checksum. `dws_mbus_record_next` walks the EN 13757-3 variable-data records, skipping the DIFE / VIFE extension chains and decoding the data length from the DIF coding (incl. the LVAR variable form) so the app can read each value. Frame formats + checksum verified against EN 13757-2; pure and host-tested. Talk to the powered two-wire bus over a UART through an M-Bus level converter (a TSS721-based master) and bridge meter readings onto Wi-Fi. See src/services/mbus/mbus.h.">M-Bus</a></td>
  <td align="center"><a href="FEATURES.md#mms" title="Opt-in IEC 61850 MMS PDU codec. When set, services/mms builds/parses the MMS (ISO 9506) confirmed-request/response Read PDUs (BER-encoded, the ACSI client/server core of IEC 61850) - dws_mms_read_request builds a Read of a named Data Object, dws_mms_read_response the data reply. Carried over ISO-on-TCP (TPKT + COTP via the shipped services/cotp) on port 102. Pure BER codec. Default off.">MMS</a></td>
  <td align="center"><a href="FEATURES.md#openadr" title="Opt-in OpenADR 3.0 (Automated Demand Response) JSON codec. When set, services/openadr builds the OpenADR 3.0 event (a demand-response signal: programID + eventName + interval payload points) and report (a VEN reading back to the VTN) JSON objects into a caller buffer, over the existing HTTP client/server + OAuth2. Pure JSON framing. Default off.">OpenADR</a></td>
  <td align="center"><a href="FEATURES.md#sep2" title="Opt-in IEEE 2030.5 (Smart Energy Profile 2.0) resource codec. When set, services/sep2 builds the core 2030.5 XML resource documents (DeviceCapability, EndDevice, DERControl) in the urn:ieee:std:2030.5:ns namespace, so the web server is a 2030.5 smart-grid server/client over the existing HTTP stack (DER dispatch / curtailment). Pure text framing. Default off.">SEP2</a></td>
  <td align="center"><a href="FEATURES.md#sunspec" title="SunSpec Modbus device-information-model codec. Default off. services/sunspec is a zero-heap codec for the SunSpec Alliance register maps layered on the holding-register model: a model-chain walker (`dws_sunspec_check_marker` / `dws_sunspec_begin` / `dws_sunspec_next_model` - verify the `SunS` 0x53756E53 marker, then iterate each model's id / length / body to the 0xFFFF end model) plus typed point readers (`dws_sunspec_u16`, `dws_sunspec_i16`, `dws_sunspec_u32`, `dws_sunspec_i32`, `dws_sunspec_string`), and a map writer (`dws_sunspec_write_marker`, `dws_sunspec_write_model_header`, the point writers, `dws_sunspec_write_end_model`) for a device exposing its own map. Makes a solar inverter / meter / battery interoperable. Marker and header format verified against the SunSpec spec; pure and host-tested. Pairs with the Modbus service. See src/services/sunspec/sunspec.h.">SunSpec</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Machine Tools &amp; OT</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#dnc-cnc-drip-feed" title="CNC RS-232 DNC (Distributed Numerical Control) drip-feed codec. Default off. services/dnc is the transport-agnostic framing + tape-code layer that streams a G-code program (RS-274 / ISO 6983) to a machine-tool controller over RS-232 or a socket. It carries the two historical tape codes: ISO 7-bit / ASCII (RS-358; End-of-Block = LF, program marker = `%`, optional even parity in bit 7) and the EIA RS-244 punched-tape code (a distinct odd-parity 8-track encoding with parity in channel 5, End-of-Block = 0x80, the rewind-stop mapped to EIA End-of-Record 0x0B, uppercase-only). `dws_dnc_iso_to_eia` / `dws_dnc_eia_to_iso` translate a character in either direction (fail-closed on a non-representable character); `dws_dnc_encode_block` frames one G-code line (characters + End-of-Block), `dws_dnc_encode_marker` writes the `%` program start/end, and `dws_dnc_encode_leader` the NUL runout. `DncDecoder` is a byte-at-a-time reassembler that strips parity / translates, skips runout, reports the `%` program markers, and delivers each block as an ASCII line (fail-closed, dropping an over-long block whole rather than truncating). `DncFlow` tracks XON/XOFF (DC1/DC3) software flow control on the reverse channel so the send pump pauses when the controller's buffer fills - kept separate from the forward stream, since the EIA data byte for `3` is 0x13 (DC3). The EIA table is validated by an odd-parity + exact-inverse host guardrail, and full encode -&gt; decode program round-trips pass for both codes. On top of the codec, **`dnc_stream`** (src/services/dnc/dnc_stream) is the drip-feed engine: it streams a whole program - leader, `%` start marker, one block per source line, `%` end marker, trailer - over a send/recv seam, pausing on a reverse-channel XOFF and waiting for XON before the next write. Because it is transport-agnostic (the seam is two function pointers), the same engine drip-feeds over a raw TCP socket (&quot;Ethernet DNC&quot;) or a UART; it is host-tested end to end with a scripted mock controller that decodes the streamed program back to lines and exercises the XOFF/XON pause-resume path (`native_dnc`, 22 cases). Pure and host-tested; you own the UART / socket. See src/services/dnc/dnc.h and dnc_stream.h.">DNC (CNC drip-feed)</a></td>
  <td align="center"><a href="FEATURES.md#mtconnect" title="Opt-in MTConnect agent response codec. When set, services/mtconnect builds the MTConnectStreams (current/sample) and MTConnectError XML response documents (ANSI/MTC1.4) into a caller buffer - header with instanceId + nextSequence, then per-DataItem Samples/Events/Condition observations - so the web server is an MTConnect agent over the existing HTTP stack. Pure text framing (values XML-escaped). Default off.">MTConnect</a></td>
  <td align="center"><a href="FEATURES.md#opc-ua" title="OPC UA Binary server. Default off. services/opcua provides an OPC UA (IEC 62541) Binary server: the little-endian built-in-type codec (incl. NodeId / ExtensionObject / DateTime / Variant / DataValue / ReferenceDescription), UA-TCP (UACP) message framing, the Hello/Acknowledge handshake, the SecureChannel (OpenSecureChannel, SecurityPolicy None), the Session (CreateSession + ActivateSession), GetEndpoints, the Read, Write and Browse services (registered resolvers map a NodeId to a value / accept a written value / list child references), plus CloseSession + CloseSecureChannel and a ServiceFault for unsupported services, served on TCP via PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`). The MSG framing is spec-faithful (incl. SecureChannelId), so standard clients interoperate (verified with python asyncua: connect + browse + read + write/read-back). All pure and host-tested. No heap, no stdlib.">OPC-UA</a></td>
  <td align="center"><a href="FEATURES.md#opc-ua-client" title="OPC UA Binary client. Default off (requires OPC-UA, shares the codec). services/dws_opcua_client builds the client-side requests (Hello, OpenSecureChannel, CreateSession, ActivateSession, Read, Browse, CloseSession, CloseSecureChannel) and parses the server responses, reusing the opcua.h codec. Transport-agnostic - the app supplies the outbound socket (e.g. an Arduino WiFiClient). No heap, no stdlib.">OPC-UA Client</a></td>
  <td align="center"><a href="FEATURES.md#umati-opc-ua-for-machine-tools" title="umati / OPC UA for Machine Tools information model (OPC 40501-1). Default off (requires OPC-UA). services/umati exposes a fixed MachineTool node hierarchy - Identification, Monitoring (MachineTool / Channel / Spindle / Axis_X..Z), Production, and Notification - through the OPC UA Browse + Read resolvers, served out of a caller-owned UmatiMachineTool struct you refresh each loop. Faithful BrowseNames per OPC 40501-1 (namespace `http://opcfoundation.org/UA/MachineTool/`); a read-only monitoring model any umati / OPC UA client browses and reads by BrowseName. No heap, no stdlib.">umati (OPC UA for Machine Tools)</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Transportation &amp; ITS</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#atc" title="Opt-in ATC (Advanced Traffic Controller) field-I/O interop snapshot. When set, services/atc exposes this device's field-I/O (a fixed table of named input/output points it already gathers via the NTCIP / NEMA-TS2 / gpio services) to an ATC Linux engine over the existing HTTP surface: dws_atc_snapshot_json serializes the FIO map as JSON, and dws_atc_set_output drives an output point from an ATC command. Pure interop codec (ATC is a platform spec, not a wire protocol). Default off.">ATC</a></td>
  <td align="center"><a href="FEATURES.md#j1939" title="SAE J1939 codec. Default off. services/j1939 is a zero-heap codec for the heavy-duty-vehicle / agriculture / marine / genset CAN higher-layer protocol over 29-bit extended frames (`shared_primitives/can.h`): `dws_j1939_encode_id` / `dws_j1939_decode_id` pack and unpack the priority / PGN / source / destination identifier (both the PDU1 peer-to-peer and PDU2 broadcast forms), `dws_j1939_build_message` emits single frames, `dws_j1939_build_request` and `dws_j1939_build_address_claim` (with `dws_j1939_build_name` for the 64-bit NAME) handle the Request PGN and Address Claimed messages, and the Transport Protocol (BAM announce + TP.DT data packets) reassembles multi-packet messages up to `DWS_J1939_TP_MAX` octets via `dws_j1939_tp_feed`. Identifier layout, NAME bit fields, and the TP control bytes verified against SAE J1939-21 / -81; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a J1939 bus onto Wi-Fi. See src/services/j1939/j1939.h.">J1939</a></td>
  <td align="center"><a href="FEATURES.md#j2735" title="Opt-in SAE J2735 V2X codec. When set, services/j2735 provides the ASN.1 UPER (Unaligned Packed Encoding Rules) bit-level primitive codec (constrained INTEGER / BOOLEAN / bit fields) and, on top of it, the J2735 BSMcore safety-message block (msgCnt / id / secMark / lat / long / elev / speed / heading) encode + decode, for connected- vehicle messaging. Pure codec (the DSRC / C-V2X radio is an external module). Default off.">J2735</a></td>
  <td align="center"><a href="FEATURES.md#nema-ts2" title="Opt-in NEMA TS 2 traffic-cabinet SDLC frame codec. When set, services/nema_ts2 builds/validates the TS 2 SDLC bus frames ([address][control][frame-type] [data][CRC-16/X-25]) that link a traffic-signal controller to the MMU, BIUs, and detector racks. Pure codec (the synchronous serial PHY + BIU timing are hardware-gated). Default off.">NEMA TS2</a></td>
  <td align="center"><a href="FEATURES.md#nmea-0183" title="NMEA 0183 sentence codec. Default off. services/nmea0183 is a zero-heap codec for the marine / GPS ASCII protocol (sentences like `$GPGGA,123519,4807.038,N,...*47`): `dws_nmea0183_build` emits a sentence (adding the `$`, the XOR checksum, and CR/LF), `dws_nmea0183_checksum` computes the XOR check, `dws_nmea0183_parse` validates the `*HH` checksum and splits the comma-separated fields (deriving the talker id + sentence type from the address field), and `dws_nmea0183_field_float` / `dws_nmea0183_field_int` decode field values (the field substrings are delimited so `dws_strtof` / `dws_strtol` stop cleanly). Sentence framing + checksum verified against the NMEA 0183 standard (the canonical GGA example checks to 0x47); pure and host-tested. GPS / marine receivers are cheap UART breakouts, so this is a plain HardwareSerial link; bridge position / wind / depth data onto Wi-Fi. See src/services/nmea0183/nmea0183.h.">NMEA 0183</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#nmea-2000" title="NMEA 2000 codec. Default off; implies J1939 (NMEA 2000 is J1939 at the transport layer). services/nmea2000 is a zero-heap codec for the marine instrumentation network over CAN: it reuses the J1939 29-bit identifier codec and adds the NMEA-specific Fast Packet transport. `dws_n2k_fastpacket_num_frames` sizes a transfer, `dws_n2k_fastpacket_build_frame` emits frame N of a 9..223-octet message (a control octet of sequence counter + frame counter, the first frame carrying the total length and 6 data octets, continuations carrying 7), and `dws_n2k_fastpacket_feed` reassembles a sequence (matching source / PGN / sequence counter, rejecting out-of-order frames and ignoring interleaved sequences); `dws_n2k_build_single` wraps a single-frame message. Fast Packet framing verified against the NMEA 2000 / J1939 layout; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge an NMEA 2000 backbone (GPS, wind, depth, engine PGNs) onto Wi-Fi. See src/services/nmea2000/nmea2000.h.">NMEA 2000</a></td>
  <td align="center"><a href="FEATURES.md#ntcip" title="Opt-in NTCIP transportation-device object identifiers. When set, services/ntcip provides the NTCIP (National Transportation Communications for ITS Protocol) object OID definitions for the common device classes - NTCIP 1202 (actuated signal controller: phases, timing, live states) and 1203 (dynamic message sign) - plus an OID builder, so an app exposes them via the shipped SNMP agent (services/snmp). Pure OID data. Default off.">NTCIP</a></td>
  <td align="center"><a href="FEATURES.md#ocit" title="Opt-in OCIT-Outstations message codec. When set, services/ocit builds/parses the OCIT (DE/AT/CH road-traffic-control) object messages ([msg-type][object-type][instance][data-type][value]) between central traffic computers and field controllers / detectors, with typed values (bool / byte / u16 / u32 / octets). Pure codec (the OCIT transport is the shipped transport). Default off.">OCIT</a></td>
  <td align="center"><a href="FEATURES.md#utmc" title="Opt-in UTMC (Urban Traffic Management and Control) common-database codec. When set, services/utmc builds/parses the UTMC common-database HTTP+XML messages - a UTMCRequest for an object id and a UTMCResponse carrying the object value + a data-quality flag + a timestamp - the UK modular framework for sharing traffic data across municipal systems, over the existing HTTP server. Pure text framing. Default off.">UTMC</a></td>
  <td align="center"><a href="FEATURES.md#wave" title="Opt-in IEEE 1609 WAVE (WSMP + 1609.2 envelope) codec. When set, services/wave builds/parses the IEEE 1609 vehicular-radio framing that carries J2735: the 1609.3 WSMP header (version + P-encoded PSID + length + payload) and the 1609.2 secured-message envelope header (version + content type). Pairs with services/j2735. Pure codec (the DSRC / C-V2X radio is an external module). Default off.">WAVE</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Clients &amp; Gateways</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ftp-client" title="FTP client wire codec (RFC 959 + RFC 2428). Default off. services/ftp is the pure protocol layer of an FTP client so a device can push/pull files - e.g. drip a `.nc` program to a CNC controller's FTP program store (Fanuc / Haas / Mazak / Heidenhain all expose one), fetch a config, or archive a log. `dws_ftp_build_command` frames any control-channel command (`VERB` or `VERB arg` + CRLF - USER / PASS / TYPE / CWD / RETR / STOR / LIST / SIZE / DELE / PASV / EPSV / QUIT / ...); `dws_ftp_build_port` and `dws_ftp_build_eprt` build the active-mode data-address commands (RFC 2428 EPRT for IPv4 / IPv6). `dws_ftp_parse_reply` detects a complete reply at the head of the control buffer - single line `NNN&lt;SP&gt;text` or a multiline `NNN-...` block ended by the same code followed by a space - and reports the 3-digit code + the bytes consumed (so pipelined replies advance cleanly); `dws_ftp_parse_pasv` decodes the `227 ...(h1,h2,h3,h4,p1,p2)` passive address and `dws_ftp_parse_epsv` the `229 ...(|||port|)` extended-passive port. Reply / PASV / EPSV formats verified against authentic strings captured from a live FTP server; pure and host-tested (the control + data sockets are the application's). See src/services/ftp/ftp.h.">FTP client</a></td>
  <td align="center"><a href="FEATURES.md#http-client" title="Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS). Default off. When set, src/services/http_client/http_client.h can issue a blocking GET/POST to a remote server: it resolves the host (DNS), opens a raw lwIP TCP connection (https:// goes through client-side mbedTLS over the same static arena as the server TLS), sends the request, and returns the status + body in caller buffers. For webhooks, telemetry push, REST calls from the device. The request builder + response parser are host-testable; the transport is ESP32-only.">HTTP Client</a></td>
  <td align="center"><a href="FEATURES.md#http-client-tls" title="HTTPS client support inside the HTTP client (needs TLS).">HTTP Client TLS</a></td>
  <td align="center"><a href="FEATURES.md#relay-tcp-forward--dnat" title="TCP relay / DNAT port forwarding. Default off. services/relay publishes an internal `host:port` through the server: an inbound (accepted) connection is relayed to an origin (an outbound `dws_client` connection to the internal service), moving bytes in both directions so the device fronts a service that lives behind it. `dws_relay_step` is a pure, non-blocking byte pump over two send/recv seams: the app calls it each poll tick until it returns `DWS_RELAY_DONE`, then closes both sockets. It handles **backpressure** (a `send` seam that accepts a partial write carries the rest in a per-direction buffer and retries before reading more) and **independent half-close** (each direction finishes when its source hits EOF and drains; the opposite peer's optional `shutdown` seam is then called once to propagate the FIN, and the relay is done only when both directions have finished). Zero heap - each active relay owns two `DWS_RELAY_BUF` buffers. Transports that report a close out of band (an `on_close` event rather than a short read - e.g. the server's `dws_conn`) call `dws_relay_note_eof` to finish a direction cleanly. Host-tested with two mock sockets covering a bidirectional transfer, the backpressure carry, half-close with shutdown propagation, a large multi-step byte-exact transfer, a seam error, and the out-of-band EOF path (`native_relay`, 6 cases). The server-side listener (src/services/relay/relay_listener) wires it in: after `server.listen(port, PROTO_RELAY)` you call `dws_relay_publish(listener_id, origin_host, origin_port)`, and a `PROTO_RELAY` connection handler dials the origin through `dws_client` on each inbound accept, pumps `dws_relay_step` from the server's poll loop, and tears both sockets down on close - so the app publishes an internal service in two calls (a fixed static bind/bridge table, zero heap; opt-in twice - compiled out by default and inert until you publish). Verified on ESP32 via the `PortForward` example build. See src/services/relay/relay.h and relay_listener.h.">Relay (TCP forward / DNAT)</a></td>
  <td align="center"><a href="FEATURES.md#smb" title="SMB2 client wire codec (MS-SMB2). Default off. services/smb is the pure protocol layer of an SMB2 client so a device can read/write files on a Windows share - e.g. a CNC controller's `.nc` program store (Fanuc / Haas / Mazak / Heidenhain expose one). Increment 1 (no crypto): `dws_smb2_transport_frame` / `dws_smb2_transport_len` handle the Direct-TCP transport header (`0x00` + a 24-bit big-endian length on port 445); `dws_smb2_build_header` / `dws_smb2_parse_header` build and validate the 64-byte little-endian SMB2 sync header (ProtocolId `FE 53 4D 42` + StructureSize 64, exposing the command / status / MessageId / TreeId / SessionId); `dws_smb2_build_negotiate` offers the dialect list (SMB 2.0.2 / 2.1 / 3.0 / 3.0.2) with the client GUID; and `dws_smb2_parse_negotiate_response` extracts the chosen dialect, server GUID, max transact/read/write sizes, and the SPNEGO/NTLM security token (bounds-checked against the message). All fields little-endian; you own the TCP socket. Field layout verified against MS-SMB2 §2.2.1.2 / §2.2.3 / §2.2.4. Increment 2 adds the **NTLM digests** (src/services/smb/smb_md): MD4 (RFC 1320 - the NT hash primitive, `MD4(UTF-16LE(password))`), MD5 (RFC 1321), and HMAC-MD5 (RFC 2104 - the NTLMv2 MAC), streaming + zero-heap, KAT-verified against the RFC test vectors and the well-known NT hash of &quot;password&quot;. (MD4/MD5 are cryptographically broken and are included only because NTLM requires them on the wire.) Increment 3 is the **NTLMv2 response** (src/services/smb/ntlm, MS-NLMP §3.3.2): `dws_ntlm_nt_hash` (`MD4(UTF-16LE(password))`), `dws_ntlm_ntowfv2` (`HMAC-MD5(NThash, UTF-16LE(Uppercase(user)+domain))`), and `dws_ntlm_v2_response`, which builds `temp` (version + timestamp + client challenge + the server's target-info AV_PAIRs) and computes the NTProofStr / NtChallengeResponse / SessionBaseKey from the server challenge - verified byte-for-byte against the MS-NLMP §4.2 worked example. Increment 4 is the **NTLMSSP message codec** (src/services/smb/ntlmssp, MS-NLMP §2.2.1): `dws_ntlmssp_build_negotiate` (type 1), `dws_ntlmssp_parse_challenge` (type 2 - extracts the flags, the 8-byte server challenge, and the target-info AV_PAIRs, bounds-checked), and `dws_ntlmssp_build_authenticate` (type 3 - lays out the LM/NT responses + domain/user/workstation in the little-endian Len/MaxLen/Offset payload fields); an end-to-end test parses a CHALLENGE, computes the NTLMv2 response, and confirms the AUTHENTICATE carries it. Increment 5 is the **SPNEGO GSS-API wrapping** (src/services/smb/spnego, RFC 4178): a zero-heap definite-length DER codec that wraps the NTLMSSP NEGOTIATE token in a GSS-API `InitialContextToken` (`[APPLICATION 0]` + the SPNEGO OID `1.3.6.1.5.5.2` + a `NegTokenInit` advertising the NTLM mech OID `1.3.6.1.4.1.311.2.2.10`), extracts the CHALLENGE from the server's `NegTokenResp` (skipping negState / supportedMech), and wraps the AUTHENTICATE in a reply `NegTokenResp` - the tokens SMB2 SESSION_SETUP carries. Verified byte-exact against a hand-computed vector, round-trip, and independently via `openssl asn1parse`. Increment 6 is the **SMB2 SESSION_SETUP framing** (src/services/smb/smb2, MS-SMB2 §2.2.5/§2.2.6): `dws_smb2_build_session_setup` builds the request (SecurityMode + PreviousSessionId + the SPNEGO security buffer at offset 88, echoing the server SessionId on the second round) and `dws_smb2_parse_session_setup_response` parses the response (StructureSize 9, SessionFlags, and the server's security buffer, bounds-checked) - the caller reads the SessionId and the `STATUS_MORE_PROCESSING_REQUIRED` / `SUCCESS` status from the header. An end-to-end test routes a full auth round through the framing (wrap an NTLMSSP NEGOTIATE in SPNEGO, frame it, then unwind a server response back through framing -&gt; SPNEGO -&gt; NTLMSSP and recover the server challenge intact). Increment 7 adds the **file commands** (src/services/smb/smb2, MS-SMB2 §2.2.9-§2.2.16): `dws_smb2_build_tree_connect` / `dws_smb2_parse_tree_connect_response` connect to a `\\server\share` UNC path (the TreeId comes back in the response header, the ShareType tells disk vs pipe vs print); `dws_smb2_build_create` / `dws_smb2_parse_create_response` open or create a file with a DesiredAccess / ShareAccess / CreateDisposition / CreateOptions set, returning the 16-byte FileId handle and the EndofFile size; and `dws_smb2_build_close` / `dws_smb2_parse_close_response` release the handle. Increment 8 completes the client with **READ / WRITE** (src/services/smb/smb2, MS-SMB2 §2.2.19-§2.2.22): `dws_smb2_build_read` / `dws_smb2_parse_read_response` read a length of bytes at a file offset (the response data is returned bounds-checked as a pointer into the message), and `dws_smb2_build_write` / `dws_smb2_parse_write_response` write a data buffer at an offset (the response reports the byte count). All little-endian, field layout verified against the MS-SMB2 request/response structures. Pure and host-tested (`native_smb`, 36 cases across the codec). This is the full read/write-a-file-on-a-share codec: NEGOTIATE -&gt; SESSION_SETUP (NTLMv2 over SPNEGO) -&gt; TREE_CONNECT -&gt; CREATE -&gt; READ / WRITE -&gt; CLOSE. On top of the codecs, **`smb_client`** (src/services/smb/smb_client) is the dialogue engine that runs the exchange: `smb_open` drives NEGOTIATE, the two-round NTLMv2 SESSION_SETUP (building the NTLMSSP tokens, wrapping them in SPNEGO, echoing the server SessionId, extracting the server's MsvAvTimestamp for the NTLMv2 response), TREE_CONNECT to `\\server\share`, and CREATE - returning an `SmbHandle` (session / tree / file id + file size); `smb_close` releases it. `smb_read` and `smb_write` then move the file bytes on that handle, looping the READ / WRITE commands in `DWS_SMB_BUF`-sized chunks (read stops at a short read / STATUS_END_OF_FILE; write grows the cached file size), giving a device a small POSIX-like open/read/write/close surface. It works against a send/recv seam (Direct-TCP framing handled internally), so the whole exchange - handshake and file transfer - is host-tested end to end with a scripted mock SMB2 server (`native_smb`, 46 cases total, including a byte-exact write-then-read round trip) and rides `dws_client` on the device. A runnable example is the remaining follow-up; SMB 3.1.1 negotiate contexts + preauth integrity and SMB2 signing are later options. See src/services/smb/smb_client.h.">SMB</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#smtp" title="Outbound SMTP client (RFC 5321) for device email alerts. Default off. services/smtp runs a blocking one-shot send over the shared outbound client transport (dws_client): read the greeting, EHLO, optional AUTH LOGIN (username/password base64-encoded), then MAIL FROM / RCPT TO / DATA the message and QUIT. The body is dot-stuffed (RFC 5321 sec 4.5.2) and CRLF-normalized so it can never end early, and the whole exchange is zero-heap (fixed DWS_SMTP_* buffers). `SmtpConfig.security` selects how the connection is secured - a `SmtpSecurity` enum (PLAIN / TLS / STARTTLS) rather than a bool, so &quot;implicit and explicit at once&quot; is unrepresentable. Implicit TLS (SMTPS, port 465) encrypts from the first byte; **STARTTLS** (RFC 3207, the submission port 587) issues STARTTLS after the first EHLO, upgrades the transport in place, and reissues EHLO because RFC 3207 sec 4.2 requires discarding capabilities learned in the clear. STARTTLS **fails closed**: if it is configured and the server does not advertise it, the send aborts with `SMTP_ERR_NO_STARTTLS` _before_ AUTH, so an attacker stripping the capability line cannot downgrade the exchange into sending credentials in plaintext - and the capability is matched as a whole keyword, so &quot;STARTTLSX&quot; does not count. The upgrade is invisible to the dialogue engine: the transport owns one send/recv pair that flips to TLS underneath it, so no path can keep writing plaintext after the handshake. Both need `DWS_ENABLE_SMTP_TLS` (which is what pulls in the client-TLS session). The dialogue engine (smtp_run) is written against a send/recv seam, so the full protocol - reply codes, AUTH, dot-stuffing, the terminating dot - is host-tested with a scripted mock server (env:native_smtp). SMS fallback needs no extra code (an email-to-SMS carrier gateway address). **HW-verified on an ESP32-S3** against a live aiosmtpd submission server: the device connected in the clear to :587, saw STARTTLS advertised, upgraded, reissued EHLO encrypted, and the server logged `DELIVERED from=esp32@... bytes=183 tls=True` with the message body intact. Getting there fixed three latent bugs in a TLS path that had **never been compiled** - a missing `&lt;mbedtls/ssl.h&gt;` include, SMTP being absent from the `DWS_ENABLE_CLIENT_TLS` derivation (so the session API resolved to stubs), and BIO callbacks dereferencing a `ctx` the TLS layer never passes them. Example SmtpAlert ships a from-scratch beginner walkthrough (including standing up a Postfix server). See src/services/smtp/smtp.h.">SMTP</a></td>
  <td align="center"><a href="FEATURES.md#webhook" title="Opt-in outbound webhooks / IFTTT. Default off. Requires HTTP_CLIENT. services/webhook builds an IFTTT Maker URL and a value1/value2/value3 JSON payload (pure, host-tested) and fires them - or any JSON to any URL - via the outbound http_client (POST). Use it to push an event from the device to IFTTT, a Slack/Discord hook, or your own API.">Webhook</a></td>
  <td align="center"><a href="FEATURES.md#ws-client" title="Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS). Default off. When set, src/services/ws_client/ws_client.h connects to a remote WebSocket endpoint (ws://, or wss:// over client-side mbedTLS), performs the RFC 6455 client handshake (Sec-WebSocket-Key/Accept), and sends masked text / binary frames + receives server frames via a callback - for streaming to cloud dashboards or bidirectional control. The frame/handshake codec is host-testable.">WS Client</a></td>
  <td align="center"><a href="FEATURES.md#ws-client-tls" title="wss://: run the WebSocket client over client-side TLS (needs TLS).">WS Client TLS</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Storage &amp; Database</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#dbm-key-value-store" title="Opt-in log-structured hash key-value store on the write-ahead log (services/dbm; requires WAL). Default off. A Bitcask-style store: each put/delete appends one WAL record (so every write is one of the WAL's fast sequential appends, never a slow durable random write), and an in-RAM open-addressed hash index - a fixed BSS array of DWS_DBM_SLOTS slots, no heap - maps each live key to where its value sits in the log. get re-reads the value straight from the log; open rebuilds the index by scanning the WAL and replaying puts and deletes in order, so the live key set is exactly what survived the last mount. Keys are bounded by DWS_DBM_KEY_MAX, values by DWS_DBM_VAL_MAX; writes are batched and made durable by a checkpoint (dws_dbm_sync). Pure and host-tested over a RAM-backed device (overwrite, tombstone resurrection, persistence across a remount with and without checkpoint, collisions, index-full fail-closed, bounds, max-value round-trip).">DBM Key-Value Store</a></td>
  <td align="center"><a href="FEATURES.md#document-store" title="Local JSON document store on the write-ahead log (services/docstore; requires DBM + WAL). Default off. A small NoSQL document store: JSON documents addressed by an id, kept durably on the WAL. It is a thin layer over dbm (the id is the key, the JSON body is the value), so it inherits dbm's zero-heap index, WAL persistence, and crash recovery; what it adds is the document capability - top-level field queries (`dws_docstore_find_str` / `_find_int` / `_find_bool` scan the live documents and match those whose JSON field equals a value, like a small `find({field: value})`) using the zero-heap JSON reader. put/get/delete forward to dbm; writes are batched and made durable by a checkpoint. Ids are bounded by DWS_DBM_KEY_MAX, bodies by DWS_DBM_VAL_MAX. Pure and host-tested over a RAM-backed device (put/get/delete, string/int/bool field finds, persistence and query across a remount, and early-stop). See src/services/docstore/docstore.h.">Document Store</a></td>
  <td align="center"><a href="FEATURES.md#redis" title="Redis RESP2/RESP3 wire codec. Default off. services/redis_resp lets a device drive a Redis server over the shipped outbound client transport: `dws_resp_encode_command` builds a command (an array of bulk strings, binary-safe via explicit arg lengths) and `dws_resp_parse` is a streaming cursor reply decoder covering RESP2 (simple / error / integer / bulk / array / nil) plus the RESP3 additions (null / boolean / double / big number / bulk error / verbatim / map / set / push); aggregates report a child count and the caller walks each child, so nested replies decode with no heap. Pure and host-tested against Redis spec vectors and verified live against a real redis-server (SET/GET, arrays, and a RESP3 map from HELLO 3); the connection is the application's. See src/services/redis_resp.h.">Redis</a></td>
  <td align="center"><a href="FEATURES.md#sqlite" title="Opt-in SQLite3 on-disk file-format reader (services/sqlite). Default off. This is file-format access, not the SQLite library: the documented SQLite3 database file structure parsed by hand - the 100-byte database header, the b-tree page header, the record varint, and record serial types - so a device can read a SQLite file (from dws_wal_fs / fs::FS) without the SQLite amalgamation, which needs a heap and stdio and does not fit the no-stdlib zero-heap model. It reads rows: cell pointers, the leaf-table cell (rowid + payload + overflow detection), a record cursor that yields each column's serial type and value bytes (with int/float decoders), and a multi-page table cursor that walks an interior b-tree over a table's rootpage in rowid order using a bounded descent stack and two page buffers (it pulls pages through a reader callback, so it works over a RAM image, dws_wal_fs, or fs::FS). Read-first (a bounded writer and overflow-page chains are later steps); pure (you hand it page bytes, it does no I/O) and host-tested against real databases produced by the sqlite3 CLI - reading the actual dws_sqlite_schema row column-by-column and scanning a 40-row, two-level b-tree - plus varint and serial-type spec vectors.">SQLite</a></td>
  <td align="center"><a href="FEATURES.md#write-ahead-log" title="Opt-in write-ahead store for atomic buffer-to-flash storage, the substrate for on-device data stores (dbm / sqlite / nosql). services/wal frames each record with a CRC-32 (dws_wal_record_encode), and dws_wal_replay walks a journal image on mount and stops at the first record with a bad magic, a failed CRC, or a truncated tail - the torn write a power loss leaves - so a crash costs at most the last un-checkpointed record. On top of that codec, dws_wal_store adds a mountable durable log over a block-device seam (WalDev, three function pointers - so it binds to any fs::FS: SD card or LittleFS): records append sequentially and are committed in bulk by dws_wal_store_checkpoint, which flips an A/B superblock (the single durable pointer move); mount picks the newest valid superblock and replays the tail past it, so records appended after the last checkpoint are still recovered and a torn superblock falls back to the other copy. Built to the SD-over-SPI envelope measured in docs/FEATURE_PERFORMANCE.md (append sequentially in ~32 KiB pages, checkpoint every ~128-256 KiB; never scatter small durable writes). Pure and host-tested (CRC-32 check vector, torn/truncated-tail recovery, checkpoint durability, superblock fallback), and dws_wal_fs.h binds the seam to a real fs::FS file (preallocated, random-access over File::seek + File::flush). The whole path is hardware-verified on an SD card over SPI: checkpoint recovery, torn-tail drop, byte-level payload persistence, and survival across a chip reset all pass. Zero heap. Default off.">Write-Ahead Log</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Time &amp; Discovery</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#adaptive-mdns" title="Opt-in adaptive mDNS beacon scheduling. Pure scheduling decisions on top of the shipped mDNS service: dws_mdns_beacon_adapt backs the announce interval off toward a ceiling under RF contention and recovers it when the air is quiet, dws_mdns_refresh_interval gives the TTL/2 continuous-refresher cadence, dws_mdns_beacon_due says when an announce is due, and dws_mdns_beacon_presleep_due says whether to announce before a sleep window that would otherwise let the record lapse. Wrap-safe time math, no heap/stdlib. Default off.">Adaptive mDNS</a></td>
  <td align="center"><a href="FEATURES.md#dns-server" title="Authoritative DNS server on UDP/53. Default off. services/dns_server answers A/IN queries from a small fixed table of `name -&gt; IPv4` records you register with `dws_dns_server_add()`, so devices on an offline / air-gapped LAN can use names (`printer.lan`) instead of raw IPs - a companion to the NTP server for self-hosted infrastructure. It parses the question, looks the name up case-insensitively, and on a hit appends one answer record using DNS name compression (a 2-byte pointer back to the question); an unknown name returns NXDOMAIN and non-A queries return no answer. The response builder (`dws_dns_server_build_response`) is pure and host-tested against the wire format; the binding is the transport UDP service. Zero heap. This is a general resolver, distinct from the provisioning captive-portal DNS (which points every name at the softAP) - do not enable both. Example DnsServer. See src/services/dns_server/dns_server.h.">DNS Server</a></td>
  <td align="center"><a href="FEATURES.md#mdns" title="mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS.">MDNS</a></td>
  <td align="center"><a href="FEATURES.md#ntp" title="SNTP wall-clock time sync via the ESP-IDF SNTP client.">NTP</a></td>
  <td align="center"><a href="FEATURES.md#ntp-server" title="NTP/SNTP time server (RFC 5905 / RFC 4330 server mode) on UDP/123. Default off. services/dws_ntp_server turns the device into a local time source: it answers client NTP requests from its own clock, so an offline or air-gapped LAN can keep its devices in sync without reaching the public pool. The 48-octet response builder (`dws_ntp_server_build_response`) is pure - it echoes the request's protocol version, copies the client's transmit timestamp into the origin field (so the client can compute round-trip delay), and stamps reference/receive/transmit times - and is host-tested against the wire format. `dws_ntp_server_begin(stratum, refid)` binds the port via the transport UDP service and drives it from `dws_time_now()` (seconds) plus a `dws_millis()`-derived sub-second fraction; while the device has no time it stays silent rather than serve a wrong clock. Pair it with a GPS receiver (parsed via the NMEA 0183 codec into a stratum-1 time source) and an upstream-NTP fallback for a self-hosted, offline-capable time server. Example NtpServer. See src/services/dws_ntp_server/dws_ntp_server.h.">NTP Server</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#nts" title="Opt-in Network Time Security (NTS, RFC 8915) wire codec. When set, services/nts provides the NTS-KE record codec (build/parse the TLV records - next protocol, AEAD, cookies, server/port) and the NTS NTP extension-field framing (Unique Identifier, Cookie, Authenticator). Pure framing (the AES-SIV-CMAC-256 AEAD + TLS-exporter key derivation are the crypto integration on top). Default off.">NTS</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Observability &amp; Telemetry</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#diag" title="Expose a diagnostic JSON endpoint via server.diag(). Disabled by default - enabling it exposes compile-time configuration (buffer sizes, feature flags) which could aid an attacker. Only enable in development or behind an authenticated route. When enabled, DWS_DIAG_JSON is a compile-time string constant you can serve from any route handler:">Diag</a></td>
  <td align="center"><a href="FEATURES.md#flow-export" title="Flow-record export codec. Default off. services/flow_export is a zero-heap exporter-side codec for on-device flow accounting in three formats: NetFlow v5 (the fixed 24-octet header + 48-octet record, via `flow_v5_write_header` / `flow_v5_write_record`), NetFlow v9 (RFC 3954), and IPFIX (RFC 7011). The v9 / IPFIX side is a small cursor (`FlowWriter`): begin the message (`flow_ipfix_begin` / `flow_v9_begin`), emit a Template (`flow_export_template`), open a matching Data Set and append records (`flow_export_data_begin` / `flow_export_data_record` / `flow_export_data_end`), then `flow_export_finish()` patches the IPFIX message length or the NetFlow v9 record count (and pads each v9 FlowSet to a 4-octet boundary). Field offsets verified against RFC 7011 / RFC 3954 / the published v5 layout; pure and host-tested. The flow cache (5-tuple + counters) and the UDP send (dws_udp_sendto) are the application's. See src/services/flow_export/flow_export.h.">Flow Export</a></td>
  <td align="center"><a href="FEATURES.md#log-buffer" title="Opt-in fixed-RAM rotating log buffer with severity traps. Default off. When set, services/logbuf keeps the last DWS_LOG_LINES log lines in a fixed ring (oldest pruned on overflow - no heap, bounded), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback when a line is logged at/above a severity threshold (forward criticals as an SNMP trap / webhook). The ring + trap logic is pure and host-tested.">Log-Buffer</a></td>
  <td align="center"><a href="FEATURES.md#metrics" title="Prometheus `/metrics` endpoint (text exposition format 0.0.4). Default off (requires STATS for the underlying counters). When set, DWS::metrics() emits the runtime stats as Prometheus metrics (`dws_uptime_seconds`, `dws_http_requests_total`, `dws_http_responses_total{class=...}`, `dws_active_connections`, `dws_free_heap_bytes`,...) so a Prometheus server can scrape the device.">Metrics</a></td>
  <td align="center"><a href="FEATURES.md#observability" title="Transport-layer observability: connection-event hook + counters. Default off (zero cost when unset - the notify points compile to nothing). When set, the transport (L4) fires an application callback on every connection state transition - `dws_conn_on_event(slot, old_state, new_state, reason)` - and maintains lock-free counters (accepts, closes by reason, idle timeouts, RX backpressure events, dropped deferred events, and a live CONN_CLOSING gauge) readable via `dws_conn_counters()`. The only state-transition trace the L4/L5 core exposes; pair it with STATS for request-level metrics.">Observability</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#partition-monitor" title="Opt-in flash partition-map monitor endpoint. Default off. When set, services/partition_monitor reports the device's flash partition table (label, kind, type / subtype, offset, size, and which app slot is running) as JSON, for diagnostics and OTA dashboards. The partition walk uses esp_partition / esp_ota_ops; the JSON serializer and the kind classifier are pure and host-testable.">Partition Monitor</a></td>
  <td align="center"><a href="FEATURES.md#snmp" title="SNMP agent (v1/v2c, + v3 USM when SNMP_V3) over lwIP UDP. Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER codec itself is gated by this flag and is otherwise unit-tested standalone (env:native_snmp).">SNMP</a></td>
  <td align="center"><a href="FEATURES.md#snmp-trap" title="Outbound SNMP notifications - traps and informs (requires SNMP). Default off. When set, src/services/snmp/dws_snmp_notify.h sends SNMPv2c (and, with SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over UDP - so the agent can push alerts instead of only answering polls. Reuses the BER codec and the transport-layer UDP service; the PDU builder is host-testable.">SNMP Trap</a></td>
  <td align="center"><a href="FEATURES.md#snmp-v3" title="Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off.">SNMP V3</a></td>
  <td align="center"><a href="FEATURES.md#stats" title="Runtime stats endpoint (uptime, request/error counts, pool usage, heap).">Stats</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#statsd" title="StatsD metrics client - push counters/gauges/timings/sets to a StatsD collector. Default off. services/statsd emits metrics in the StatsD wire format (`name:value|type`, e.g. `api.hits:1|c`) over UDP via dws_udp_sendto to any StatsD-speaking backend (Graphite/StatsD, Telegraf, Datadog, InfluxDB), with counters (`c`), gauges (`g`, absolute or a signed `+`/`-` delta), timings (`ms`), and sets (`s`), plus an optional sample rate (`|@0.1`) and DogStatsD tags (`|#env:prod`). This is the push counterpart to the pull-based Prometheus `/metrics` endpoint - useful behind NAT/firewalls where nothing can reach in to scrape. The value/rate are rendered without printf float/64-bit formatting; the line builder (`dws_statsd_format`) is pure and host-tested, and the emit helpers are host-tested through the transport UDP capture seam. Zero heap. Example StatsdMetrics. See src/services/statsd/statsd.h.">StatsD</a></td>
  <td align="center"><a href="FEATURES.md#syslog" title="Syslog client (RFC 5424 over UDP). Default off. When set, the device can ship log lines to a remote syslog server (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the transport-layer UDP service - a zero-heap structured-logging sink for fleets of constrained devices. See src/services/syslog/syslog.h.">Syslog</a></td>
  <td align="center"><a href="FEATURES.md#telemetry" title="Telemetry math helpers (moving-window stats, rate-of-change, totalizer). Default off. When set, src/services/telemetry/telemetry.h provides zero-heap pure-computation helpers over caller-supplied storage: a moving-window stats accumulator (mean / variance / stddev / min / max), a derivative / rate-of- change tracker, and a trapezoidal run-time totalizer. No ESP32 dependency, so the whole cluster is host-testable; it feeds dashboards, alert triggers, and odometer-style counters.">Telemetry</a></td>
  <td align="center"><a href="FEATURES.md#udp-telemetry" title="Opt-in fire-and-forget UDP telemetry cast. Default off. When set, services/udp_telemetry casts metric lines (InfluxDB line protocol: `measurement field=val,field2=val2`) to a configured collector over UDP via dws_udp_sendto - zero-heap, fire-and-forget (no ACK, no retry), ideal for shipping device metrics to Telegraf/InfluxDB/a log sink. The line builder is pure and host-tested; only the send touches the network.">UDP Telemetry</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Firmware &amp; System</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ota" title="Authenticated OTA firmware update (streaming POST to the ESP32 Update API).">OTA</a></td>
  <td align="center"><a href="FEATURES.md#ota-rollback" title="Opt-in OTA rollback protection / soft-brick safeguard. Default off. After an OTA update the new image boots in PENDING_VERIFY; this service confirms it (esp_ota_mark_app_valid) once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses without success - so a bad update self-heals instead of soft-bricking. The decision logic is pure and host-tested; the commit / rollback use esp_ota_ops. Requires the bootloader's app-rollback support (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE).">OTA Rollback</a></td>
  <td align="center"><a href="FEATURES.md#provisioning" title="First-boot WiFi provisioning: softAP + captive-portal credentials form.">Provisioning</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Application (L7) - Other</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#dtls-13" title="DTLS 1.3 (RFC 9147) datagram security for UDP transports - the secure datagram counterpart to the TLS 1.3 that already backs HTTP/3, aimed at CoAP-over-DTLS and constrained-device telemetry. Default off. This flag currently gates the DTLS 1.3 **record layer** (network_drivers/presentation/dtls/dws_dtls_record): the DTLSPlaintext classic header (initial flight + alerts) and the DTLSCiphertext unified header with per-record AEAD (AEAD_AES_128_GCM), the RFC 9147 §4.2.3 sequence-number encryption (AES-ECB mask over the ciphertext sample) and §4.2.2 reconstruction, the TLS 1.3 AEAD nonce, and the §4.5.1 anti-replay sliding window. Record and key derivation are reused from the hand-rolled TLS 1.3 stack (dws_quic_hkdf HKDF-Expand-Label, dws_quic_aead AES-128-GCM), so enabling this also compiles those primitives (otherwise gated behind HTTP/3). The record layer is byte-exact against an independent HKDF + AES-128-GCM + AES-ECB reconstruction (host-tested, `native_dtls`). This flag also gates the **handshake framing and reliability** layer (network_drivers/presentation/dtls/dws_dtls_handshake, RFC 9147 §5 + §7): the 12-byte DTLS handshake header, overlap-tolerant message reassembly (§5.4), the ACK message (content type 26, §7), and the stateless HelloRetryRequest cookie (§5.1) that binds the client address into an HMAC-SHA256 for return-routability; its cookie wire format is byte-exact against an independent Python-stdlib HMAC-SHA256 (host-tested, `native_dtls_hs`). It also gates the **server handshake state machine** (network_drivers/presentation/dtls/dws_dtls_conn, RFC 9147 §5-6): the one-round-trip full handshake (TLS_AES_128_GCM_SHA256 / X25519 / Ed25519, no PSK / 0-RTT / client auth), transport-neutral like dws_coap_server_process - ClientHello in, then ServerHello (epoch 0) and the encrypted EncryptedExtensions / Certificate / CertificateVerify / Finished flight (epoch 2), the client Finished verified, and epoch 0→2→3 key transitions. It reuses the TLS 1.3 messages + key schedule (dws_tls13_msg, dws_tls13_kdf, guards lifted to HTTP/3 or DTLS). Proven end-to-end (`native_dtls_conn`) and, most importantly, **against a real reference implementation**: the wolfSSL DTLS 1.3 client completes a full handshake and an application-data round trip with the server (test/servers/dws_dtls_wolfssl), both offering X25519 directly and through a HelloRetryRequest group renegotiation. DTLS 1.3 derives every secret and record key with the RFC 9147 §5.9 `dtls13` HKDF-Expand-Label prefix (not TLS 1.3's `tls13 `), modelled as a `Tls13Kdf` variant bound once into the key schedule. When a client offers X25519 in supported_groups but sends no X25519 key_share, the server renegotiates the group with a **HelloRetryRequest** carrying an address-bound, stateless cookie (§5.1) and restarts the transcript with the §4.4.1 message_hash before spending any asymmetric crypto. Lost flights are recovered by a caller-driven **retransmission timer** (§5.8): `dws_dtls_conn_timeout_ms` / `dws_dtls_conn_on_timeout` re-send the whole flight with fresh record sequence numbers on an exponential backoff (capped, with a retransmit ceiling that abandons a dead handshake), inbound ACKs cancel it, and a retransmitted client Finished (whose ACK was lost) is re-acknowledged. The **CoAP-over-DTLS front-end** (services/coap, when DWS_ENABLE_COAP is also set) secures CoAP over UDP end to end: the bridge (coaps/dws_coaps_process) drives the DtlsConn handshake and then unwraps each epoch-3 application record, answers it with dws_coap_server_process(), and re-wraps the response (proven host-side, `native_coaps`, replay-drop included); and coaps_server binds UDP 5684 (coaps://) to a per-peer DtlsConn pool - routing each datagram to its connection by peer address, driving the handshakes and the retransmission timer from a single dws_coaps_server_poll(), and reaping idle connections (host-tested `native_coaps_server`: peer routing, poll-driven retransmission, idle reap; example CoapSecure). Pure, zero-heap, not the mbedTLS TCP-TLS engine. See src/network_drivers/presentation/dtls/dws_dtls_conn.h.">DTLS 1.3</a></td>
  <td align="center"><a href="FEATURES.md#edge-cache-origin-tls" title="Opt-in TLS upstream origins for the edge cache (requires Edge Cache + TLS). Default off. When set, a mapped `https://` origin is fetched over the shared client-TLS session (`dws_tls_csess`, the same one MQTTS / wss use) layered over `dws_client` via mbedTLS BIO callbacks, selected per route by a scheme flag on the route map; without it an `https://` origin is rejected. The handshake blocks briefly in the transport's `open` (like `dws_client_open`'s connect and the MQTT/WS clients), and a `dws_tls_client_session_active()` guard fails open rather than tearing down a live shared session, so one TLS origin fetch runs at a time. Verification is off by default (encrypt-only, no authentication); `dws_edge_cache_set_origin_ca(pem, len)` enables full chain + hostname verification and `dws_edge_cache_set_origin_pin(sha256)` pins the cert (both write the shared client-TLS trust store, so they also apply to MQTTS / wss / the HTTP client). HW-verified on an ESP32-S3 against a real https origin: encrypt-only fetch byte-exact, a correct CA verifies and a wrong CA is rejected (fail-open), Range/`206` works over the TLS-fetched body. Note: the TLS engine adds a ~48 KB arena (an S3 / PSRAM board is recommended), and on mbedtls v2 (espressif32 default) an IP-address origin needs a CN-matching cert (IP-address SANs are not matched; DNS origins work normally). Follow-ups: an async handshake so a cold https MISS never blocks the worker, and a multi-session pool for more than one concurrent TLS fetch.">Edge Cache Origin TLS</a></td>
  <td align="center"><a href="FEATURES.md#fanuc-focas" title="FANUC FOCAS Ethernet protocol codec (FANUC CNC data over TCP 8193). Default off. services/focas builds + parses the FOCAS wire frames a FANUC control speaks (big-endian throughout, a 10-octet magic/version/type/length envelope + payload): `dws_focas_build_open`/`_close` drive the session handshake (FRAME_DST open, empty close), `dws_focas_build_request` emits the generic command frame (a 6-octet function selector + five signed 32-bit arguments + optional trailing data), and typed wrappers cover SysInfo, alarm status, CNC parameters, macro variables, position/axis data, and the actual feedrate / spindle speed. `dws_focas_parse_frame` validates the envelope, `dws_focas_parse_response` decodes the echoed selector + FOCAS return code + data, `dws_focas_parse_sysinfo` decodes ODBSYS, `dws_focas_parse_alarm` reads the alarm bitmask, and `dws_focas_decode8` / `dws_focas_value_f` decode the FANUC 8-octet `data / base^exp` numeric encoding used by positions, feeds, and macros. Frame layout, selector encoding, the SysInfo/alarm response layouts, and the value encoding were reverse-engineered by and cross-checked against diohpix/pyfanuc (the FOCAS wire protocol is not officially published; the proprietary fwlib32 library is not required or used). Pure codec, host-tested (`native_focas`); the caller owns the TCP socket and drives the open -&gt; command -&gt; close sequence. FANUC is the most widely deployed CNC control, so this is a direct machine-tool data source. See src/services/focas/focas.h.">FANUC FOCAS</a></td>
  <td align="center"><a href="FEATURES.md#forwarded-trust" title="Trusted-reverse-proxy resolution of the forwarded client address (requires AUTH_LOCKOUT). Default off. A `Forwarded` (RFC 7239) / `X-Forwarded-For` header is client-spoofable, so behind a reverse proxy the per-IP auth lockout keys on the original client address the proxy reports ONLY when the request's real TCP peer matches a trusted-upstream CIDR you register with `dws_forwarded_trust_add_cidr(&quot;10.0.0.0/8&quot;)`; a direct or untrusted peer's header is ignored, and any malformed / obfuscated / unspecified token falls back to the real TCP peer. So one abusive client behind a proxy cannot lock out every client sharing the proxy's address, and no client can spoof its way out of, or another address into, a lockout. The accept-time throttle and the IP allowlist deliberately stay on the real TCP source. The trusted-CIDR table is a fixed BSS array of DWS_TRUSTED_PROXY_MAX entries (no heap); the resolver is pure and host-tested (native_forwarded_trust) and HW-verified on an ESP32-S3 (a forwarded client is locked out independently of the TCP source and of other forwarded clients). See src/services/forwarded_trust/forwarded_trust.h.">Forwarded Trust</a></td>
  <td align="center"><a href="FEATURES.md#ftp-client-session" title="`DWS_ENABLE_FTP_SESSION` (requires `DWS_ENABLE_FTP`)">FTP client session</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#gpib" title="GPIB-over-LAN (Prologix-style) controller command codec. Default off. services/gpib is a zero-heap codec for the Prologix-compatible `++` command set that drives a bench of legacy IEEE-488 (GPIB) instruments through a Prologix GPIB-Ethernet / GPIB-USB adapter (raw socket on TCP 1234) - the bridge into pre-LAN test gear that will never speak SCPI-over-TCP directly. It builds the control commands (`dws_gpib_command` for the generic form, plus typed `dws_gpib_addr` / `dws_gpib_read` / `dws_gpib_spoll` / `dws_gpib_eos` helpers - the `++eos` mapping 0=CR+LF / 1=CR / 2=LF / 3=None), builds an escaped data line (`dws_gpib_build_data` - a leading ESC before each CR / LF / ESC / `+` byte in the payload, then an unescaped newline, verified byte-exact against the manual's binary example), classifies a line as command-vs-data (`dws_gpib_is_command`), and parses the responses (`dws_gpib_parse_decimal` for the serial-poll status byte / SRQ / address, `dws_gpib_parse_addr`, `dws_gpib_parse_version`). Command set + escaping + port verified against the Prologix GPIB-Ethernet / GPIB-USB controller manuals (cross-checked with AR488). Carries SCPI ([DWS_ENABLE_SCPI](#scpi)) to the instrument as data. Pure codec, host-tested; the socket / serial link to the adapter is the application's. See src/services/gpib/gpib.h.">GPIB</a></td>
  <td align="center"><a href="FEATURES.md#haas-mdc" title="Haas Machine Data Collection (MDC) Q-command codec. Default off. services/haas_mdc is a zero-heap codec for the documented Haas Automation MDC protocol - the `?Q` query set a Haas CNC mill / lathe control answers over RS-232 (7-E-1, XON/XOFF) or a raw TCP socket (Setting 143, default port 5051) - so a device becomes a fixed-BSS CNC data collector that fans machine status into HTTP / MQTT (alongside the shipped Fanuc [FOCAS](#fanuc-focas), [MELSEC](#melsec), [S7comm](#s7comm) and [DNC](#dnc-cnc-drip-feed) machine-tool codecs). As the collector it builds the numbered queries (`dws_haas_mdc_build_q` for `?Q100` serial / `?Q101` software version / `?Q102` model / `?Q104` mode / `?Q300` power-on time / `?Q500` program+status+parts / ..., taking a `HaasQ` value or a raw command number) and the macro/system-variable read (`dws_haas_mdc_build_var` -&gt; `?Q600 &lt;var&gt;`); then it parses the reply (`dws_haas_mdc_parse` locates the CSV payload framed between STX (0x02) and ETB (0x17) - scanning, never by offset - and splits it into trimmed fields), with typed decoders on top: `dws_haas_mdc_value` (the value of a simple `LABEL, value` reply such as Q100/Q104), `dws_haas_mdc_parse_status` (the two-branch Q500 - `PROGRAM, Oxxxxx, &lt;status&gt;, PARTS, n` when idle/running vs `STATUS, BUSY` mid-operation), `dws_haas_mdc_parse_macro` (`MACRO, &lt;var&gt;, &lt;value&gt;`), and `dws_haas_mdc_is_error` (the `UNKNOWN` reply to an unsupported / lowercase command). It also de-multiplexes the unprompted `DPRNT(...)` lines a running G-code program pushes on the same link (`dws_haas_mdc_dprnt_line` - raw CRLF text with no STX/ETB, optionally bracketed by DC2 (POPEN) / DC4 (PCLOS)). Request framing, the STX/ETB response wrapper, and the Q100/Q104/Q500/Q600 field layouts are verified against the Haas MDC service manual (Setting 143) + the Q-command table and cross-checked byte-for-byte against a production Haas serial adapter. Pure codec, host-tested (`native_haas_mdc`, 10 cases); the serial / TCP link to the control is the application's. See src/services/haas_mdc/haas_mdc.h.">Haas MDC</a></td>
  <td align="center"><a href="FEATURES.md#hislip" title="HiSLIP (High-Speed LAN Instrument Protocol, IVI-6.1) message codec. Default off. services/hislip is a zero-heap codec for the IVI Foundation's modern LXI instrument transport on TCP port 4880 - the successor to VXI-11 that carries SCPI ([DWS_ENABLE_SCPI](#scpi)) at higher throughput over two TCP channels (a synchronous SCPI command/response stream and an asynchronous out-of-band control channel), correlated by a 16-bit SessionID. `dws_hislip_build_header` / `dws_hislip_parse_header` frame the fixed 16-byte header (`&quot;HS&quot;` prologue + message type + control code + 32-bit MessageParameter + 64-bit PayloadLength, all big-endian); `dws_hislip_build_initialize` / `_initialize_response` / `_async_initialize` / `_async_initialize_response` (+ the matching parsers) drive the two-channel handshake (the MessageParameter carries `(protocol version &lt;&lt; 16) | vendor id`, then the negotiated version + SessionID); and `dws_hislip_build_data` frames a Data / DataEND message carrying a SCPI payload keyed by a MessageID (`dws_hislip_next_message_id` implements the initial-`0xFFFFFF00`, increment-by-2 rule). The full `HislipMsg` message-type enum (0-38, including the HiSLIP 2.0 TLS / SASL additions), the header byte layout, and the handshake vectors are verified against IVI-6.1 (cross-checked with the Wireshark dissector, MSL-equipment, and PyHiSLIP). Pure codec, host-tested; the two TCP connections are the application's. See src/services/hislip/hislip.h.">HiSLIP</a></td>
  <td align="center"><a href="FEATURES.md#hot-swap-storage" title="Opt-in safeties for removable storage that can vanish mid-write. Default off. An SD card is a connector, and the failure when it leaves is a quiet one: the driver still reports a mounted volume, every write fails into nothing, and code that does not check the return carries on believing it has storage. services/hotswap runs one state machine per volume - `ABSENT` (nothing mounted, no filesystem call is safe) / `READY` / `FAULTED` (was mounted, I/O is failing, unmounted and awaiting a remount probe). `dws_hotswap_ready()` is the **fail-closed gate** callers check before any filesystem call, and `dws_hotswap_io(ok)` reports every outcome; `DWS_HOTSWAP_FAIL_THRESHOLD` (default 3) consecutive failures declare the medium gone and unmount it immediately so nothing can write through a stale handle, while a probe every `DWS_HOTSWAP_PROBE_MS` remounts a card that comes back. The threshold is deliberate: a single failed write is not proof of removal (a transient bus error, a full volume) and tearing down a working mount over one error would be its own bug, so any success resets the run and intermittent noise never accumulates into a false removal. A volume starts ABSENT rather than READY (assuming storage exists because the code compiled is how silent data loss happens at boot), and present-but-unmountable stays ABSENT because a card that will not mount is not storage. The fail counter saturates rather than wrapping, and probe pacing is wrap-safe across a `millis()` rollover. Core is pure and takes an explicit `now`, so the whole machine is host-tested with a synthetic clock (`native_hotswap`, 20 cases); mounting is three app callbacks (mount / unmount / optional card-detect), since how a volume mounts is the application's business. **HW-verified on an ESP32-P4** with a real SD card: three good writes left it `ready`/`faults:0`, an unmount underneath the app made the next three writes really fail and the **third** faulted it (`{&quot;storage&quot;:&quot;faulted&quot;,&quot;mounts&quot;:1,&quot;faults&quot;:1}`), the following write was **refused** with `storage not ready` rather than attempted, and the probe then remounted it by itself (`{&quot;storage&quot;:&quot;ready&quot;,&quot;mounts&quot;:2,&quot;faults&quot;:1}`). Serialized as `{&quot;storage&quot;:..,&quot;mounts&quot;:N,&quot;faults&quot;:N}` for a /health panel. Example HotSwapStorage. See src/services/hotswap/hotswap.h.">Hot-Swap Storage</a></td>
  <td align="center"><a href="FEATURES.md#ikev2" title="IKEv2 (RFC 7296) message + payload codec. Default off. services/ikev2 is a zero-heap builder / parser for the Internet Key Exchange v2 wire format that negotiates IPsec security associations over UDP 500 / 4500 (NAT-T) - it is **tier 1 (the pure framing)** of an IKEv2 / IPsec stack, the standards-track &quot;secure machine bridge over untrusted networks&quot; southbound: the Diffie-Hellman math, the SKEYSEED / SK\_\* key derivation, the SK AEAD, and the IKE_SA_INIT -&gt; IKE_AUTH state machine are later tiers that reuse the crypto the library already ships. It builds / parses the **28-octet IKE header** (`dws_ike_hdr_build` / `_parse` - initiator / responder SPIs, next payload, version, exchange type, flags, message id, length, plus `dws_ike_set_length` to backfill the whole-message length) and walks the **generic payload chain** (`dws_ike_payload_iter_init` / `dws_ike_payload_next` - forward-linked by each payload's next-payload field, with `dws_ike_payload_build` for a raw payload). Typed builders + parsers cover the IKE_SA_INIT / IKE_AUTH payload set: **SA** (`dws_ike_sa_build` writes one proposal with its transforms - ENCR / PRF / INTEG / D-H / ESN - encoding the key-length attribute, and `dws_ike_sa_first_proposal` + `dws_ike_transform_iter_init` / `dws_ike_transform_next` decode the proposal -&gt; transform tree back, reading the key-length attribute), **KE** (`dws_ike_ke_build` / `_parse` - D-H group + key data), **Nonce** (Ni / Nr), **IDi / IDr** (`dws_ike_id_build` / `_parse`), **CERT / CERTREQ** (`dws_ike_cert_build`), **AUTH** (`dws_ike_auth_build` / `_parse` - PSK / RSA / digital-signature methods), **N notify** (`dws_ike_notify_build` / `_parse` - protocol + optional SPI + 16-bit type + data), **D delete** (`dws_ike_delete_build` / `_parse`), **TSi / TSr** traffic selectors (`dws_ike_ts_build` / `dws_ike_ts_count` / `dws_ike_ts_get` - IPv4 / IPv6 address ranges + port ranges), and the **SK** encrypted-payload envelope (`dws_ike_sk_build` / `_parse` frames IV + ciphertext + ICV so a later tier plugs the AEAD in). Every parser bounds-checks and fails closed on a malformed length. The header, payload, and SA / proposal / transform layouts (including the key-length transform attribute) are verified against RFC 7296 + the IANA registry and cross-checked byte-for-byte against scapy's IKEv2 codec. Pure codec, host-tested (`native_ikev2`, 16 cases); the UDP transport, the crypto, and the SA state machine are the application's / later tiers. The protocol constants are **scoped** (`enum class`) and the types flow through the API - `IkePayloadType`, `IkeExchange`, `IkeProtocol`, `IkeIdType`, `IkeAuthMethod`, `IkeTransformType`, `IkeTsType` appear on the struct fields and builder / parser signatures, with a cast only where an octet is written to or read from the wire. That is not cosmetic: RFC 7296 numbers the exchange and payload registries into the same range, so `IKE_SA_INIT` (34) collides with `IKE_PL_KE` (34), `IKE_AUTH` (35) with `IKE_PL_IDI` (35), and so on for every exchange value. Passing the wrong one used to compile silently and emit a structurally valid message a peer would parse and misinterpret; it is now a compile error. See src/services/ikev2/ikev2.h.">IKEv2</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#interface-bridge" title="Opt-in user-defined address:port -&gt; hardware-bus bridge, a configurable &quot;device server&quot;. The app registers rules mapping a listen `x.x.x.x:nnnn` (TCP/UDP) to a UART, an SPI chip-select, or an I2C address (`dws_iface_bridge_map(ip, port, proto, target)`), so a network client talking to that port is transparently bridged to the bus: raw bidirectional stream passthrough for UART (a ser2net-style serial device server), or framed write-then-read transactions (`uint16 write_len || uint16 read_len || write_bytes`, big-endian) for the master-initiated SPI / I2C buses, with the bus address / chip-select / clock / mode taken from the rule's target. The fixed-capacity rule table (keyed by port+proto, carrying the full DWSIp bind address, never a flattened one) and the transaction frame codec are a pure, zero-heap, host-tested core (services/iface_bridge); the bus I/O (Serial / SPI / Wire) and the PROTO_BRIDGE connection handler are the ESP32 step (iface_bridge_hw), wired up in two calls: `server.listen(port, ConnProto::PROTO_BRIDGE)` then `dws_iface_bridge_publish(listener_id, port, proto, target)`. UART stream mode is a ser2net-style raw pipe pumped by the server poll loop; SPI/I2C transaction mode peeks a whole frame out of the connection ring, clocks it against the bus (I2C uses a repeated-start read), and returns the read bytes. Default off. See src/services/iface_bridge/iface_bridge_hw.h and examples/L7-Application/InterfaceBridge.">Interface Bridge</a></td>
  <td align="center"><a href="FEATURES.md#logging-macros" title="Abstract logging whose disabled levels cost nothing at all. Instrumentation is only worth leaving in the source permanently if a build that does not want it pays nothing for it - not a branch, not a call, and not a format string sitting in flash, where a runtime `if (level &gt;= threshold)` still links every message. So the filter is the preprocessor: `DWS_LOGD` / `DWS_LOGI` / `DWS_LOGW` / `DWS_LOGE` below `DWS_LOG_LEVEL` expand to a form that names its arguments only inside `sizeof(...)`, an unevaluated context, which emits no code and no string literal yet still runs the compiler's printf format checking over them and marks the arguments used (so a variable read only by a log does not warn). Enable the level and the same line starts logging, with no source change. Defaults to `DWS_LOG_LEVEL_NONE`, so it is opt-in per build (`-DDWS_LOG_LEVEL=DWS_LOG_LEVEL_WARN`); at NONE the implementation compiles to nothing at all, defining no symbols and no BSS. An emitted line is formatted once into a `DWS_LOG_LINE_LEN` stack buffer and handed to services/logbuf's ring (when `DWS_ENABLE_LOGBUF` is on) and to a sink registered with `dws_log_set_sink()` (Serial, syslog, a websocket console). **Measured, not asserted**: a translation unit with four discarded log statements compiles byte-identical to the same function with none (7 bytes .text, 87 total in both), a bad format at a discarded level is still diagnosed by `-Wformat`, and a discarded call never evaluates its arguments. Host-tested (`native_log`); dogfooded by services/ftp_session, whose `DWS_LOGD` trace is what localized a real transport-gating bug on hardware. See src/shared_primitives/log.h.">Logging macros</a></td>
  <td align="center"><a href="FEATURES.md#lsv2" title="Heidenhain LSV/2 telegram codec. Default off. services/lsv2 is a zero-heap codec for the LSV/2 protocol Heidenhain TNC controls (iTNC 530, TNC 320 / 620 / 640, ...) speak for DNC and data access over a serial link or, as implemented here, LSV/2-over-TCP (default port 19000) - so a device becomes a fixed-BSS collector for the common European CNC control, alongside the shipped Fanuc [FOCAS](#fanuc-focas), [Haas MDC](#haas-mdc), [MELSEC](#melsec), [S7comm](#s7comm) and [DNC](#dnc-cnc-drip-feed) machine-tool codecs. **Framing** (byte-exact, both directions): a telegram is a 4-byte big-endian payload-length prefix, a 4-character ASCII command / response mnemonic, then the payload - the length counts the payload only (the mnemonic is not included), so a telegram with no payload is exactly 8 bytes on the wire (a bare `T_OK` acknowledgement is `00 00 00 00 'T' '_' 'O' 'K'`). `dws_lsv2_build` frames an arbitrary mnemonic + payload and `dws_lsv2_parse` slices one complete telegram off a byte stream (reporting the consumed byte count so a caller can re-frame the rest). **Typed builders** cover the common requests: `dws_lsv2_build_login` / `dws_lsv2_build_logout` (the `A_LG` / `A_LO` privilege-group access with the `INSPECT` / `FILE` / `DNC` / `MONITOR` / `DIAGNOSTICS` / `PLCDEBUG` groups + optional password), `dws_lsv2_build_filename` (the null-terminated-filename file commands - `R_FL` load, `C_FL` send, `C_FD` delete, `C_DC` change dir, `C_DM` / `C_DD` make / delete dir), and `dws_lsv2_build_run_info` (`R_RI` with a 2-byte big-endian run-info selector: execution state / selected program / override / program state). **Response readers** decode the reply mnemonic (`dws_lsv2_is_ok` for `T_OK`, `dws_lsv2_is_error` for the `T_ER` / `T_BD` errors, `dws_lsv2_error` for their two-byte error-class + error-code, and `dws_lsv2_is` for the `S_*` data replies). The telegram framing, mnemonic set, and the login / filename / run-info payload layouts are cross-checked byte-for-byte against the pyLSV2 reference (drunsinn/pyLSV2). Pure codec, host-tested (`native_lsv2`, 12 cases); the serial / TCP link to the control is the application's. See src/services/lsv2/lsv2.h.">LSV/2</a></td>
  <td align="center"><a href="FEATURES.md#ntrip-caster" title="Opt-in GNSS RTK base station + NTRIP caster (services/gnss). Default off (implies `DWS_ENABLE_NMEA0183`). Turns the device into a differential-GNSS correction source: it surveys in a fixed antenna position and serves RTCM 3.x corrections to rovers over the network (the NTRIP protocol), so a rover applies them for RTK / DGPS accuracy instead of the ~2.5 m of a bare receiver. Three pure, zero-heap, host-tested cores plus a ConnProto listener: (1) the RTCM3 codec (services/gnss/rtcm3) - the transport frame (0xD3 preamble, 6 reserved + 10-bit length, payload, 24-bit CRC-24Q), MSB-first bit I/O, and the Stationary Antenna Reference Point messages 1005 (no height) / 1006 (with antenna height) that advertise the base's surveyed ECEF position (38-bit signed coordinates at 0.0001 m resolution), verified byte-for-byte against pyrtcm; (2) the survey-in core (services/gnss/dws_gnss_survey) - the exact WGS84 geodetic&amp;lt;-&amp;gt;ECEF transform (matched against pyproj), a shifted-origin position averager with a 3-D accuracy estimate and a min-observations / accuracy-limit convergence gate, and a GGA-fix fold (ellipsoidal height = MSL + geoid separation); (3) the NTRIP caster protocol (services/gnss/dws_ntrip_caster) - rover request parsing (mountpoint, NTRIP 1.0 / 2.0 version, optional HTTP Basic auth), the stream-accept / error / 401 responses, and the RTCM source table (STR records + ENDSOURCETABLE). The `ConnProto::PROTO_NTRIP_CASTER` listener (services/gnss/dws_ntrip_caster_listener) answers rovers and fans RTCM corrections out to every subscriber, published like the relay: `server.listen(2101, ConnProto::PROTO_NTRIP_CASTER)` then `dws_ntrip_caster_add_mount()` / `dws_ntrip_caster_broadcast()`. Example NtripCaster runs a base (survey-in + caster) and a rover (NTRIP client that CRC-validates and decodes the 1005) on two boards. Generating RTCM3 _observation_ messages (the MSM sets 1074/1077/1084/... that let a rover fix carrier-phase ambiguities for centimeter RTK) requires a receiver that outputs raw measurements (u-blox RXM-RAWX: F9P / M8T class); a raw-less module (NEO-6/7, GT-U7) can still survey in and serve the reference point + sourcetable. See src/services/gnss/dws_ntrip_caster_listener.h.">NTRIP Caster</a></td>
  <td align="center"><a href="FEATURES.md#post-quantum-hybrid-kex" title="Post-quantum / traditional hybrid key exchange: ML-KEM-768 (FIPS 203) combined with X25519. Closes the harvest-now-decrypt-later gap - OpenSSH 9.9+ and current browsers now DEFAULT to a hybrid group, so without this the device negotiates DOWN to classical X25519. When set (and SSH is on) the server advertises `mlkem768x25519-sha256` (draft-ietf-sshm-mlkem-hybrid-kex) first in its SSH KEX list and, on selection, ML-KEM-Encaps to the client's key + X25519, combining `K = SHA256(K_PQ || K_CL)` per the RFC 9370 concatenation combiner. The device is always the KEM responder, so only Encaps ships (no KeyGen/Decaps, so none of the constant-time FO re-encryption surface). The ML-KEM core (network_drivers/presentation/pqc) is a software NTT over q=3329 with Montgomery reduction plus a Keccak/SHA-3/SHAKE sponge (FIPS 202); zero heap, peak ~7 KB of worker stack (raise `DWS_WORKER_TASK_STACK` to &gt;= `DWS_WORKER_STACK_PQC_MIN` = 16384). Byte-exact against the FIPS 203 reference (kyber-py) and verified end to end vs an independent client (ML-KEM Decaps). Wired into both transports from the one core: the SSH key exchange (`DWS_ENABLE_SSH`, `mlkem768x25519-sha256`, K = SHA256(K_PQ || K_CL), K as an RFC 4251 string) and the HTTP/3 QUIC TLS 1.3 handshake (`DWS_ENABLE_HTTP3`, the **X25519MLKEM768** group, IANA 0x11ec, ML-KEM-first client/server shares and a 64-byte ML-KEM || X25519 secret into the key schedule). A PQC-capable peer (OpenSSH 9.9+, current browsers) negotiates the hybrid; others fall back to classical X25519. Default off.">Post-Quantum Hybrid KEX</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#power-governor" title="Opt-in SoC power governor. Default off. services/radio_power owns the radio and services/sleep_sched decides how _long_ to sleep; neither owns the SoC, which is where the rest of the power budget goes. `dws_power_plan()` decides the CPU clock every tick from three inputs, with a deliberate precedence - **brownout beats thermal beats load**, because a board that cannot hold its supply must not be clocked up merely because it is busy, and neither must a hot one. **Scaling**: an idle server drops to `DWS_POWER_MHZ_MIN` instead of spinning a 240 MHz core to poll a quiet socket. **Thermal throttle**: a hot die clocks down, and the restore threshold is lower than the throttle threshold - the caller feeds the previous tick's `throttled` flag back in, which is what supplies the hysteresis. **Brownout recovery**: after an `ESP_RST_BROWNOUT` reset (read once and latched at boot) the part holds the floor clock for `DWS_POWER_RECOVER_MS` rather than slamming back into the load that collapsed the rail and boot-looping. **Gating**: `dws_power_gate_bt()` disables and releases the Bluetooth controller's power domain on a build that never uses BT, which draws current whether or not anything is connected. A part with no usable internal sensor (classic ESP32) reports `INT16_MIN`, treated as &quot;no reading&quot; rather than ice-cold, so it neither throttles nor silently releases one. Pure decision core taking every input explicitly, host-tested (`native_power_mgmt`, 19 cases incl. a 20-tick feedback loop asserting exactly one throttle transition). **HW-verified on an ESP32-S3** with its real die sensor: the Bluetooth domain was released, an idle server clocked itself 240 -&gt; 80 MHz, `GET /busy` took it to 240 MHz with the die rising 36 -&gt; 38 C and back, and with the thresholds moved to a reachable band the throttle engaged at exactly `temp_hot_c` and released at exactly `temp_cool_c`. That run also produced a tuning rule now documented at the flag: **the hysteresis band must be wider than the temperature swing the clock change itself causes** (measured ~2 C for 240 -&gt; 80 MHz on an S3 within one 500 ms tick), or the throttle's own effect carries the die back across the release threshold and it self-sustains regardless of how correct the comparison is - which is why the default band is 10 C. Example PowerGovernor. See src/services/power_mgmt/power_mgmt.h.">Power Governor</a></td>
  <td align="center"><a href="FEATURES.md#scpi" title="SCPI / IEEE 488.2 instrument-control codec. Default off. services/scpi is a zero-heap codec for the text command language nearly every modern bench instrument speaks (DMMs, oscilloscopes, power supplies, function/arbitrary generators, SMUs, spectrum/network analyzers, electronic loads) over a raw TCP socket on port 5025 (also USBTMC / VXI-11 / HiSLIP / serial). The codec is symmetric: as a **controller** it builds command lines with `dws_scpi_build` (a `:`-hierarchy header + comma-separated params + newline) and `dws_scpi_common` (the 13 mandatory IEEE 488.2 commands `*IDN?` / `*RST` / `*CLS` / `*ESR?` / `*STB?` / ...), then parses replies - `dws_scpi_parse_number` (NR1/NR2/NR3, hand-rolled, no stdlib), `_parse_bool` (`1`/`0`/`ON`/`OFF`), `_parse_string` (quote-stripping with doubled-quote collapse), and `_parse_block` (the definite `#&lt;n&gt;&lt;len&gt;&lt;data&gt;` and indefinite `#0&lt;data&gt;` arbitrary block used for waveform captures); as an **instrument** it carries the IEEE 488.2 status model - `ScpiStatus` (the Status Byte, Standard Event Status Register + its enable mask, the Service Request Enable mask, and the FIFO error/event queue) with `dws_scpi_push_error` (latching the CME/EXE/DDE/QYE/... ESR class bit from the error-number range and replacing the tail with -350 &quot;Queue overflow&quot; when full), `dws_scpi_pop_error` (the `SYSTem:ERRor?` action, `0,&quot;No error&quot;` when empty), `dws_scpi_stb` (EAV/ESB/MSS computation), and `dws_scpi_cls` (`*CLS`) - plus `dws_scpi_match`, a SCPI short/long-form + numeric-suffix header matcher for dispatching an incoming command against a pattern like `&quot;SYSTem:ERRor?&quot;`. Register bits, error-number classes, and the exact standard error strings are verified against the SCPI-1999 standard and IEEE 488.2-1992. Pure codec, host-tested; the TCP/USB/serial transport is the application's. See src/services/scpi/scpi.h.">SCPI</a></td>
  <td align="center"><a href="FEATURES.md#sen0192" title="DFRobot SEN0192 10.525 GHz microwave Doppler motion sensor (single digital OUT line). Default off. Unlike a PIR it senses movement through thin non-metal enclosures and is unaffected by ambient light or temperature; unlike the LD2410 it carries no protocol - just one digital line - so services/sen0192 tracks that line as a debounced presence signal: presence asserts on an active sample and is held for `DWS_SEN0192_HOLD_MS` after the last active sample (so brief gaps between Doppler returns don't make presence flap), clears after it, and counts clear-to-present edges. The presence state machine (`Sen0192Motion`) is pure - it takes a sampled level and a timestamp, needing no clock or GPIO - and host-tested (`native_sen0192`); the ESP32 binding reads `DWS_SEN0192_PIN` each poll via dws_millis() and only that read touches hardware. The OUT pin, hold window, and polarity are ServerConfig knobs (`DWS_SEN0192_PIN` / `DWS_SEN0192_HOLD_MS` / `DWS_SEN0192_ACTIVE_HIGH`). Example Sen0192 lights the onboard LED on motion. See src/services/sen0192/sen0192.h.">SEN0192</a></td>
  <td align="center"><a href="FEATURES.md#smtp-tls" title="Secure SMTP: run the mail client over client-side TLS (needs TLS). Covers both `SmtpSecurity::SMTP_TLS` (implicit, port 465) and `SmtpSecurity::SMTP_STARTTLS` (the RFC 3207 in-band upgrade on the submission port, 587). Separate from `DWS_ENABLE_SMTP` because the plain codec needs neither the TLS stack nor the client-session singleton, and it is what pulls SMTP into the `DWS_ENABLE_CLIENT_TLS` derivation - without that every `dws_tls_client_session_*` call resolves to a stub that always fails. See src/services/smtp/smtp.h.">SMTP TLS</a></td>
  <td align="center"><a href="FEATURES.md#vxi-11" title="VXI-11 (TCP/IP Instrument Protocol) codec over ONC RPC / XDR. Default off. services/vxi11 is a zero-heap codec for the legacy LXI instrument transport that predates HiSLIP ([DWS_ENABLE_HISLIP](#hislip)) - VXI-11 rides on ONC RPC (Sun RPC, RFC 5531) with XDR (RFC 4506) over TCP, carrying SCPI ([DWS_ENABLE_SCPI](#scpi)). It provides the reusable ONC-RPC framing - `dws_rpc_record_mark` / `dws_rpc_parse_record_mark` (the TCP record-marking header) and `dws_rpc_parse_reply` (the accepted-reply header with AUTH_NONE) over big-endian, 4-byte-aligned, length-prefixed XDR (no sub-word types on the wire) - plus the DEVICE_CORE procedures: `dws_vxi11_build_create_link` / `_device_write` / `_device_read` / `_device_readstb` / `_destroy_link` (program `0x0607AF` v1) with their response parsers, and the portmapper `dws_vxi11_build_getport` call that maps the program to its dynamic TCP port (the instrument channel is not on a fixed port). Device_Flags (waitlock / end / termchrset), the read `reason` bits (REQCNT / CHR / END), and the Device_ErrorCode set are covered; `dws_vxi11_error_str` names an error. The XDR struct layouts, procedure numbers, and RPC headers are verified against the VXI-11 spec + RFC 5531 / 4506 / 1833 (cross-checked with python-vxi11 and the Wireshark dissector), with a byte-exact create_link vector. Pure codec, host-tested; the TCP connection is the application's. See src/services/vxi11/vxi11.h.">VXI-11</a></td>
</tr>
</tbody>
</table>

<!-- END GENERATED FEATURE TABLES -->


## New to this? Start here

If networking is new to you, the [**learn series**](learn/) is a from-scratch on-ramp
that assumes no prior knowledge: the [OSI model](learn/osi-model.md),
[TCP/IP](learn/tcp-ip.md), and [a primer on every language](learn/languages.md) in the
project - each tied back to the code below. Every protocol the library implements is
mapped to its authoritative spec in [STANDARDS.md](STANDARDS.md).

## Architecture

Each OSI layer lives in its own subdirectory under `src/network_drivers/`:

<details>
<summary><b>View Directory and OSI Layer Layout</b></summary>

```
L7  src/dwserver.h/cpp     Route table, dispatch, send()
L6  src/network_drivers/presentation/
        presentation.h/cpp                        Drains ring buffer → parser
        http_parser.h/cpp                         RFC 7230 byte-stream state machine
        sha1.h/cpp  base64.h/cpp                  mbedTLS hardware-accelerated helpers
        websocket.h/cpp  sse.h/cpp                WS frame parser; SSE connection pool
        multipart.h/cpp                           Multipart form-data parser
L5  src/network_drivers/session/
        session.h/cpp                             FreeRTOS event queue drain
L4  src/network_drivers/transport/
        tcp.h/cpp                           lwIP callbacks, ring buffers, timeouts
        listener.h/cpp                            Per-port TCP listener, per-listener queue
L3  src/network_drivers/network/
        network.h/cpp                             lwIP stub
L2  src/network_drivers/datalink/
        datalink.h/cpp                            Espressif WiFi driver stub
L1  src/network_drivers/physical/
        physical.h/cpp                            WiFi.begin() wrapper

    src/network_drivers/tls/                      mbedTLS over a fixed static pool (HTTPS / wss)
    src/network_drivers/application/              Generated web assets (dashboard, terminal)
    src/network_drivers/presentation/ssh/         Zero-heap SSH-2.0 server
    src/services/                                 Optional L7 subsystems, one folder each:
        opcua/ + opcua_client/, modbus/, mqtt/, coap/, snmp/, dns_resolver/, oidc/,
        oauth2/, totp/, audit_log/, vfs/, graphql/, espnow/, ...  (see FEATURES.md)
```

The conceptual layer map above is a summary; the complete file layout is generated
below from `src/` by `docs/utilities/gen_readme_sections.py` (single-`.h`/`.cpp`
service folders are collapsed to their name; generated web-asset blobs are counted,
not listed).

<details>
<summary><b>Full source tree (every library file)</b></summary>

<!-- BEGIN GENERATED SOURCE-TREE (docs/utilities/gen_readme_sections.py) -->

```text
src/
├── board_profiles/
│   ├── 16mbflash.h
│   ├── 16mbpsram.h
│   ├── 2mbflash.h
│   ├── 2mbpsram.h
│   ├── 32mbflash.h
│   ├── 32mbpsram.h
│   ├── 4mbflash.h
│   ├── 4mbpsram.h
│   ├── 8mbflash.h
│   ├── 8mbpsram.h
│   ├── board_profile.h
│   ├── c2_defaults.h
│   ├── c3_defaults.h
│   ├── c5_defaults.h
│   ├── c61_defaults.h
│   ├── c6_defaults.h
│   ├── classic_defaults.h
│   ├── h21_defaults.h
│   ├── h2_defaults.h
│   ├── h4_defaults.h
│   ├── p4_defaults.h
│   ├── s2_defaults.h
│   ├── s31_defaults.h
│   └── s3_defaults.h
├── network_drivers/
│   ├── application/
│   │   ├── binary_asset_blobs.cpp
│   │   ├── binary_asset_blobs.h
│   │   ├── web_assets.cpp
│   │   └── web_assets.h
│   ├── datalink/  (datalink.h, datalink.cpp)
│   ├── network/
│   │   ├── ip.cpp
│   │   ├── ip.h
│   │   ├── network.cpp
│   │   └── network.h
│   ├── physical/  (physical.h, physical.cpp)
│   ├── presentation/
│   │   ├── base64/  (base64.h, base64.cpp)
│   │   ├── cbor/  (cbor.h, cbor.cpp)
│   │   ├── deflate/  (deflate.h, deflate.cpp)
│   │   ├── dtls/
│   │   │   ├── dtls_conn.cpp
│   │   │   ├── dtls_conn.h
│   │   │   ├── dtls_handshake.cpp
│   │   │   ├── dtls_handshake.h
│   │   │   ├── dtls_record.cpp
│   │   │   └── dtls_record.h
│   │   ├── hpack_prim/  (hpack_prim.h, hpack_prim.cpp)
│   │   ├── http2/
│   │   │   ├── h2_conn.cpp
│   │   │   ├── h2_conn.h
│   │   │   ├── h2_frame.cpp
│   │   │   ├── h2_frame.h
│   │   │   ├── h2_server.cpp
│   │   │   ├── h2_server.h
│   │   │   ├── hpack.cpp
│   │   │   └── hpack.h
│   │   ├── http3/
│   │   │   ├── h3_conn.cpp
│   │   │   ├── h3_conn.h
│   │   │   ├── h3_frame.cpp
│   │   │   ├── h3_frame.h
│   │   │   ├── qpack.cpp
│   │   │   ├── qpack.h
│   │   │   ├── quic_aead.cpp
│   │   │   ├── quic_aead.h
│   │   │   ├── quic_conn.cpp
│   │   │   ├── quic_conn.h
│   │   │   ├── quic_crypto.cpp
│   │   │   ├── quic_crypto.h
│   │   │   ├── quic_frame.cpp
│   │   │   ├── quic_frame.h
│   │   │   ├── quic_hkdf.cpp
│   │   │   ├── quic_hkdf.h
│   │   │   ├── quic_packet.cpp
│   │   │   ├── quic_packet.h
│   │   │   ├── quic_server.cpp
│   │   │   ├── quic_server.h
│   │   │   ├── quic_tls.cpp
│   │   │   ├── quic_tls.h
│   │   │   ├── quic_tp.cpp
│   │   │   ├── quic_tp.h
│   │   │   ├── quic_varint.cpp
│   │   │   ├── quic_varint.h
│   │   │   ├── tls13_kdf.cpp
│   │   │   ├── tls13_kdf.h
│   │   │   ├── tls13_msg.cpp
│   │   │   └── tls13_msg.h
│   │   ├── http_parser/  (http_parser.h, http_parser.cpp)
│   │   ├── inflate/  (inflate.h, inflate.cpp)
│   │   ├── json/  (json.h, json.cpp)
│   │   ├── msgpack/  (msgpack.h, msgpack.cpp)
│   │   ├── multipart/  (multipart.h, multipart.cpp)
│   │   ├── pqc/
│   │   │   ├── mlkem.cpp
│   │   │   ├── mlkem.h
│   │   │   ├── sha3.cpp
│   │   │   └── sha3.h
│   │   ├── sha1/  (sha1.h, sha1.cpp)
│   │   ├── sse/  (sse.h, sse.cpp)
│   │   ├── ssh/
│   │   │   ├── auth/  (ssh_auth.h, ssh_auth.cpp)
│   │   │   ├── connection/
│   │   │   │   ├── ssh_channel.cpp
│   │   │   │   ├── ssh_channel.h
│   │   │   │   ├── ssh_conn.cpp
│   │   │   │   ├── ssh_conn.h
│   │   │   │   ├── ssh_forward.cpp
│   │   │   │   ├── ssh_forward.h
│   │   │   │   ├── ssh_server.cpp
│   │   │   │   └── ssh_server.h
│   │   │   ├── crypto/
│   │   │   │   ├── ssh_aes256ctr.cpp
│   │   │   │   ├── ssh_aes256ctr.h
│   │   │   │   ├── ssh_aesgcm.cpp
│   │   │   │   ├── ssh_aesgcm.h
│   │   │   │   ├── ssh_bignum.cpp
│   │   │   │   ├── ssh_bignum.h
│   │   │   │   ├── ssh_chacha20.cpp
│   │   │   │   ├── ssh_chacha20.h
│   │   │   │   ├── ssh_chachapoly.cpp
│   │   │   │   ├── ssh_chachapoly.h
│   │   │   │   ├── ssh_curve25519.cpp
│   │   │   │   ├── ssh_curve25519.h
│   │   │   │   ├── ssh_ecdsa.cpp
│   │   │   │   ├── ssh_ecdsa.h
│   │   │   │   ├── ssh_ed25519.cpp
│   │   │   │   ├── ssh_ed25519.h
│   │   │   │   ├── ssh_ed25519_comb_table.h
│   │   │   │   ├── ssh_fe25519.h
│   │   │   │   ├── ssh_hmac_sha256.cpp
│   │   │   │   ├── ssh_hmac_sha256.h
│   │   │   │   ├── ssh_hmac_sha512.cpp
│   │   │   │   ├── ssh_hmac_sha512.h
│   │   │   │   ├── ssh_poly1305.cpp
│   │   │   │   ├── ssh_poly1305.h
│   │   │   │   ├── ssh_rsa.cpp
│   │   │   │   ├── ssh_rsa.h
│   │   │   │   ├── ssh_sha256.cpp
│   │   │   │   ├── ssh_sha256.h
│   │   │   │   ├── ssh_sha512.cpp
│   │   │   │   └── ssh_sha512.h
│   │   │   └── transport/
│   │   │       ├── ssh_comp.cpp
│   │   │       ├── ssh_comp.h
│   │   │       ├── ssh_dh.cpp
│   │   │       ├── ssh_dh.h
│   │   │       ├── ssh_keymat.cpp
│   │   │       ├── ssh_keymat.h
│   │   │       ├── ssh_packet.cpp
│   │   │       ├── ssh_packet.h
│   │   │       ├── ssh_transport.cpp
│   │   │       ├── ssh_transport.h
│   │   │       ├── ssh_zlib.cpp
│   │   │       └── ssh_zlib.h
│   │   ├── telnet/  (telnet.h, telnet.cpp)
│   │   ├── websocket/  (websocket.h, websocket.cpp)
│   │   ├── presentation.cpp
│   │   └── presentation.h
│   ├── session/
│   │   ├── arena.cpp
│   │   ├── arena.h
│   │   ├── proto_builtins.cpp
│   │   ├── proto_handler.h
│   │   ├── scratch.cpp
│   │   ├── scratch.h
│   │   ├── session.cpp
│   │   ├── session.h
│   │   ├── worker.cpp
│   │   └── worker.h
│   ├── tls/  (tls.h, tls.cpp)
│   └── transport/
│       ├── client.cpp
│       ├── client.h
│       ├── listener.cpp
│       ├── listener.h
│       ├── tcp.cpp
│       ├── tcp.h
│       ├── udp.cpp
│       └── udp.h
├── server/
│   ├── auth.cpp
│   ├── dwserver_internal.h
│   ├── file_serving.cpp
│   ├── fs_path.h
│   ├── http_range.cpp
│   ├── http_range.h
│   ├── middleware.cpp
│   ├── regex.cpp
│   ├── response.cpp
│   ├── ssh_scp.cpp
│   ├── ssh_scp.h
│   ├── ssh_sftp.cpp
│   ├── ssh_sftp.h
│   ├── webdav.cpp
│   └── websocket_sse.cpp
├── services/
│   ├── ads/  (ads.h, ads.cpp)
│   ├── ads1115/  (ads1115.h, ads1115.cpp)
│   ├── amqp/  (amqp.h, amqp.cpp)
│   ├── atc/  (atc.h, atc.cpp)
│   ├── audit_log/  (audit_log.h, audit_log.cpp)
│   ├── auth_lockout/  (auth_lockout.h, auth_lockout.cpp)
│   ├── bacnet/  (bacnet.h, bacnet.cpp)
│   ├── ble_gatt/  (ble_gatt.h, ble_gatt.cpp)
│   ├── bus_capture/  (bus_capture.h, bus_capture.cpp)
│   ├── c37118/  (c37118.h, c37118.cpp)
│   ├── canopen/  (canopen.h, canopen.cpp)
│   ├── cc1101/  (cc1101.h, cc1101.cpp)
│   ├── cclink/  (cclink.h, cclink.cpp)
│   ├── cia402/  (cia402.h, cia402.cpp)
│   ├── cip/  (cip.h, cip.cpp)
│   ├── cloudevents/  (cloudevents.h, cloudevents.cpp)
│   ├── coap/
│   │   ├── coap.cpp
│   │   ├── coap.h
│   │   ├── coaps.cpp
│   │   ├── coaps.h
│   │   ├── coaps_server.cpp
│   │   └── coaps_server.h
│   ├── config_io/  (config_io.h, config_io.cpp)
│   ├── config_store/  (config_store.h, config_store.cpp)
│   ├── control/  (control.h, control.cpp)
│   ├── cotp/  (cotp.h, cotp.cpp)
│   ├── csrf/  (csrf.h, csrf.cpp)
│   ├── dashboard/
│   │   ├── dashboard.cpp
│   │   ├── dashboard.h
│   │   └── dashboard_routes.cpp
│   ├── dbm/  (dbm.h, dbm.cpp)
│   ├── dds/  (dds.h, dds.cpp)
│   ├── device_id/  (device_id.h, device_id.cpp)
│   ├── devicenet/  (devicenet.h, devicenet.cpp)
│   ├── df1/  (df1.h, df1.cpp)
│   ├── directnet/  (directnet.h, directnet.cpp)
│   ├── dma/  (dma.h, dma.cpp)
│   ├── dmx/  (dmx.h, dmx.cpp)
│   ├── dnc/
│   │   ├── dnc.cpp
│   │   ├── dnc.h
│   │   ├── dnc_stream.cpp
│   │   └── dnc_stream.h
│   ├── dnp3/  (dnp3.h, dnp3.cpp)
│   ├── dns_resolver/  (dns_resolver.h, dns_resolver.cpp)
│   ├── dns_server/  (dns_server.h, dns_server.cpp)
│   ├── docstore/  (docstore.h, docstore.cpp)
│   ├── dshot/  (dshot.h, dshot.cpp)
│   ├── edge_cache/
│   │   ├── edge_cache.cpp
│   │   ├── edge_cache.h
│   │   ├── edge_cache_proxy.cpp
│   │   ├── edge_cache_proxy.h
│   │   ├── edge_cache_sd.cpp
│   │   ├── edge_cache_sd.h
│   │   ├── edge_fetch.cpp
│   │   ├── edge_fetch.h
│   │   ├── edge_mesh.cpp
│   │   └── edge_mesh.h
│   ├── enip/  (enip.h, enip.cpp)
│   ├── enocean/  (enocean.h, enocean.cpp)
│   ├── espnow/  (espnow.h, espnow.cpp)
│   ├── exc_decoder/
│   │   ├── exc_coredump.cpp
│   │   ├── exc_decoder.cpp
│   │   └── exc_decoder.h
│   ├── failsafe/  (failsafe.h, failsafe.cpp)
│   ├── fdc2214/  (fdc2214.h, fdc2214.cpp)
│   ├── fins/  (fins.h, fins.cpp)
│   ├── flow_export/  (flow_export.h, flow_export.cpp)
│   ├── focas/  (focas.h, focas.cpp)
│   ├── forward/  (forward.h, forward.cpp)
│   ├── forwarded_trust/  (forwarded_trust.h, forwarded_trust.cpp)
│   ├── ftp/
│   │   ├── ftp.cpp
│   │   ├── ftp.h
│   │   ├── ftp_session.cpp
│   │   └── ftp_session.h
│   ├── gateway/  (gateway.h, gateway.cpp)
│   ├── gnss/
│   │   ├── gnss_survey.cpp
│   │   ├── gnss_survey.h
│   │   ├── ntrip_caster.cpp
│   │   ├── ntrip_caster.h
│   │   ├── ntrip_caster_listener.cpp
│   │   ├── ntrip_caster_listener.h
│   │   ├── rtcm3.cpp
│   │   └── rtcm3.h
│   ├── goose/  (goose.h, goose.cpp)
│   ├── gpib/  (gpib.h, gpib.cpp)
│   ├── gpio_map/
│   │   ├── gpio_map.cpp
│   │   ├── gpio_map.h
│   │   └── gpio_map_routes.cpp
│   ├── graphql/  (graphql.h, graphql.cpp)
│   ├── grpcweb/  (grpcweb.h, grpcweb.cpp)
│   ├── guardrails/  (guardrails.h, guardrails.cpp)
│   ├── haas_mdc/  (haas_mdc.h, haas_mdc.cpp)
│   ├── happy_eyeballs/  (happy_eyeballs.h, happy_eyeballs.cpp)
│   ├── hart/  (hart.h, hart.cpp)
│   ├── hislip/  (hislip.h, hislip.cpp)
│   ├── hostlink/  (hostlink.h, hostlink.cpp)
│   ├── hotswap/  (hotswap.h, hotswap.cpp)
│   ├── http_client/  (http_client.h, http_client.cpp)
│   ├── http_delivery/
│   │   ├── http_delivery.cpp
│   │   ├── http_delivery.h
│   │   └── http_delivery_routes.cpp
│   ├── httpcache/  (httpcache.h, httpcache.cpp)
│   ├── hw_health/  (hw_health.h, hw_health.cpp)
│   ├── iccp/  (iccp.h, iccp.cpp)
│   ├── iec60870/  (iec60870.h, iec60870.cpp)
│   ├── iface_bridge/
│   │   ├── iface_bridge.cpp
│   │   ├── iface_bridge.h
│   │   ├── iface_bridge_hw.cpp
│   │   └── iface_bridge_hw.h
│   ├── ikev2/  (ikev2.h, ikev2.cpp)
│   ├── ina219/  (ina219.h, ina219.cpp)
│   ├── interbus/  (interbus.h, interbus.cpp)
│   ├── iolink/  (iolink.h, iolink.cpp)
│   ├── j1939/  (j1939.h, j1939.cpp)
│   ├── j2735/  (j2735.h, j2735.cpp)
│   ├── jwt/  (jwt.h, jwt.cpp)
│   ├── ld2410/  (ld2410.h, ld2410.cpp)
│   ├── ldc1614/  (ldc1614.h, ldc1614.cpp)
│   ├── link_manager/  (link_manager.h, link_manager.cpp)
│   ├── logbuf/  (logbuf.h, logbuf.cpp)
│   ├── lonworks/  (lonworks.h, lonworks.cpp)
│   ├── lora/  (lora.h, lora.cpp)
│   ├── lsv2/  (lsv2.h, lsv2.cpp)
│   ├── lwm2m/  (lwm2m_tlv.h, lwm2m_tlv.cpp)
│   ├── mbplus/  (mbplus.h, mbplus.cpp)
│   ├── mbus/  (mbus.h, mbus.cpp)
│   ├── mdns_adaptive/  (mdns_adaptive.h, mdns_adaptive.cpp)
│   ├── mdns_service/  (mdns_service.h, mdns_service.cpp)
│   ├── melsec/  (melsec.h, melsec.cpp)
│   ├── mms/  (mms.h, mms.cpp)
│   ├── modbus/
│   │   ├── modbus.cpp
│   │   ├── modbus.h
│   │   ├── modbus_master.cpp
│   │   └── modbus_master.h
│   ├── mpr121/  (mpr121.h, mpr121.cpp)
│   ├── mqtt/
│   │   ├── mqtt.cpp
│   │   ├── mqtt.h
│   │   ├── mqtt_sn.cpp
│   │   └── mqtt_sn.h
│   ├── mtconnect/  (mtconnect.h, mtconnect.cpp)
│   ├── nats/  (nats.h, nats.cpp)
│   ├── nema_ts2/  (nema_ts2.h, nema_ts2.cpp)
│   ├── netadapt/  (netadapt.h, netadapt.cpp)
│   ├── nmea0183/  (nmea0183.h, nmea0183.cpp)
│   ├── nmea2000/  (nmea2000.h, nmea2000.cpp)
│   ├── nrf24/  (nrf24.h, nrf24.cpp)
│   ├── ntcip/  (ntcip.h, ntcip.cpp)
│   ├── ntp_server/  (ntp_server.h, ntp_server.cpp)
│   ├── ntp_service/  (ntp_service.h, ntp_service.cpp)
│   ├── nts/  (nts.h, nts.cpp)
│   ├── oauth2/  (oauth2.h, oauth2.cpp)
│   ├── ocit/  (ocit.h, ocit.cpp)
│   ├── oidc/  (oidc.h, oidc.cpp)
│   ├── opcua/  (opcua.h, opcua.cpp)
│   ├── opcua_client/  (opcua_client.h, opcua_client.cpp)
│   ├── openadr/  (openadr.h, openadr.cpp)
│   ├── ota_rollback/  (ota_rollback.h, ota_rollback.cpp)
│   ├── ota_service/  (ota_service.h, ota_service.cpp)
│   ├── partition_monitor/
│   │   ├── partition_monitor.cpp
│   │   ├── partition_monitor.h
│   │   └── partition_monitor_routes.cpp
│   ├── pca9685/  (pca9685.h, pca9685.cpp)
│   ├── pn532/  (pn532.h, pn532.cpp)
│   ├── power_mgmt/  (power_mgmt.h, power_mgmt.cpp)
│   ├── powerlink/  (powerlink.h, powerlink.cpp)
│   ├── preempt_queue/  (preempt_queue.h, preempt_queue.cpp)
│   ├── profibus/  (profibus.h, profibus.cpp)
│   ├── profinet/  (profinet.h, profinet.cpp)
│   ├── promisc/  (promisc.h, promisc.cpp)
│   ├── protobuf/  (protobuf.h, protobuf.cpp)
│   ├── provisioning_service/  (provisioning_service.h, provisioning_service.cpp)
│   ├── proxy_protocol/  (proxy_protocol.h, proxy_protocol.cpp)
│   ├── psram_pool/  (psram_pool.h, psram_pool.cpp)
│   ├── radio_power/  (radio_power.h, radio_power.cpp)
│   ├── radio_sniff/  (radio_sniff.h, radio_sniff.cpp)
│   ├── rawl2/  (rawl2.h, rawl2.cpp)
│   ├── redis_resp/  (redis_resp.h, redis_resp.cpp)
│   ├── relay/
│   │   ├── relay.cpp
│   │   ├── relay.h
│   │   ├── relay_listener.cpp
│   │   └── relay_listener.h
│   ├── rtc/  (rtc.h, rtc.cpp)
│   ├── s7comm/  (s7comm.h, s7comm.cpp)
│   ├── scp/  (scp.h, scp.cpp)
│   ├── scpi/  (scpi.h, scpi.cpp)
│   ├── sdi12/  (sdi12.h, sdi12.cpp)
│   ├── sen0192/  (sen0192.h, sen0192.cpp)
│   ├── senml/  (senml.h, senml.cpp)
│   ├── sep2/  (sep2.h, sep2.cpp)
│   ├── sercos/  (sercos.h, sercos.cpp)
│   ├── sftp/  (sftp.h, sftp.cpp)
│   ├── sht3x/  (sht3x.h, sht3x.cpp)
│   ├── sigfox/  (sigfox.h, sigfox.cpp)
│   ├── sleep_sched/  (sleep_sched.h, sleep_sched.cpp)
│   ├── smb/
│   │   ├── ntlm.cpp
│   │   ├── ntlm.h
│   │   ├── ntlmssp.cpp
│   │   ├── ntlmssp.h
│   │   ├── smb2.cpp
│   │   ├── smb2.h
│   │   ├── smb_client.cpp
│   │   ├── smb_client.h
│   │   ├── smb_md.cpp
│   │   ├── smb_md.h
│   │   ├── spnego.cpp
│   │   └── spnego.h
│   ├── smtp/  (smtp.h, smtp.cpp)
│   ├── snmp/
│   │   ├── snmp_agent.cpp
│   │   ├── snmp_agent.h
│   │   ├── snmp_ber.cpp
│   │   ├── snmp_ber.h
│   │   ├── snmp_crypto.cpp
│   │   ├── snmp_crypto.h
│   │   ├── snmp_notify.cpp
│   │   ├── snmp_notify.h
│   │   ├── snmp_v3.cpp
│   │   └── snmp_v3.h
│   ├── snp/  (snp.h, snp.cpp)
│   ├── sockpool/  (sockpool.h, sockpool.cpp)
│   ├── southbound/  (southbound.h, southbound.cpp)
│   ├── spa_router/  (spa_router.h, spa_router.cpp)
│   ├── sparkplug/  (sparkplug.h, sparkplug.cpp)
│   ├── sqlite/  (sqlite_format.h, sqlite_format.cpp)
│   ├── statsd/  (statsd.h, statsd.cpp)
│   ├── stomp/  (stomp.h, stomp.cpp)
│   ├── sunspec/  (sunspec.h, sunspec.cpp)
│   ├── syslog/  (syslog.h, syslog.cpp)
│   ├── telemetry/  (telemetry.h, telemetry.cpp)
│   ├── thread/  (thread.h, thread.cpp)
│   ├── time_source/  (time_source.h, time_source.cpp)
│   ├── tls_policy/  (tls_policy.h, tls_policy.cpp)
│   ├── totp/  (totp.h, totp.cpp)
│   ├── udp_telemetry/  (udp_telemetry.h, udp_telemetry.cpp)
│   ├── umati/  (umati.h, umati.cpp)
│   ├── upload_service/  (upload_service.h, upload_service.cpp)
│   ├── utmc/  (utmc.h, utmc.cpp)
│   ├── vfs/  (vfs.h, vfs.cpp)
│   ├── vl53l0x/  (vl53l0x.h, vl53l0x.cpp)
│   ├── vxi11/  (vxi11.h, vxi11.cpp)
│   ├── wal/
│   │   ├── wal.cpp
│   │   ├── wal.h
│   │   ├── wal_fs.h
│   │   ├── wal_store.cpp
│   │   └── wal_store.h
│   ├── wamp/  (wamp.h, wamp.cpp)
│   ├── wave/  (wave.h, wave.cpp)
│   ├── wearlevel/  (wearlevel.h, wearlevel.cpp)
│   ├── web_terminal/  (web_terminal.h, web_terminal.cpp)
│   ├── webdav/  (webdav.h, webdav.cpp)
│   ├── webhook/  (webhook.h, webhook.cpp)
│   ├── wifi_sniffer/  (wifi_sniffer.h, wifi_sniffer.cpp)
│   ├── wisun/  (wisun.h, wisun.cpp)
│   ├── ws_client/  (ws_client.h, ws_client.cpp)
│   ├── xmpp/  (xmpp.h, xmpp.cpp)
│   ├── zigbee/  (zigbee.h, zigbee.cpp)
│   ├── zwave/  (zwave.h, zwave.cpp)
│   ├── clock.h
│   └── i2c.h
├── shared_primitives/
│   ├── aes_block.h
│   ├── aes_sbox.h
│   ├── bitio.h
│   ├── bytes.h
│   ├── can.h
│   ├── crypto_opt.h
│   ├── endian.h
│   ├── fmtbuf.h
│   ├── ghash.h
│   ├── hex.h
│   ├── http_date.h
│   ├── log.cpp
│   ├── log.h
│   ├── mime.h
│   ├── numparse.h
│   ├── pcap.h
│   ├── ring.h
│   ├── strbuf.h
│   └── utf8.h
├── web/
│   ├── favicons/  (288 generated files)
│   ├── input/
│   │   ├── DWS_DASHBOARD_PAGE.html
│   │   ├── DWS_METRICS_PROM.txt
│   │   ├── DWS_PROV_FORM.html
│   │   ├── DWS_PROV_SAVED_HTML.html
│   │   ├── DWS_SERVICE_WORKER.js
│   │   ├── DWS_STATS_JSON.json
│   │   └── DWS_TERMINAL_PAGE.html
│   ├── themes/  (112 generated files)
│   ├── wizard/
│   │   ├── build_assets.py
│   │   ├── gen_favicons.py
│   │   ├── gen_theme_blobs.py
│   │   └── gen_themes.py
│   └── README.md
├── dwserver.cpp
├── dwserver.h
└── ServerConfig.h
```

<!-- END GENERATED SOURCE-TREE -->

</details>

### Build Footprint

Measured flash + static RAM for each optional feature, built in isolation over the
base server on `esp32dev`. Generated from `docs/footprints.json` (produced by the
RPi build matrix) by `docs/utilities/gen_readme_sections.py`.

<details>
<summary><b>Per-feature build footprint</b></summary>

<!-- BEGIN GENERATED BUILD-FOOTPRINT (docs/utilities/gen_readme_sections.py) -->

Measured on `esp32dev` from each feature's isolated example (one feature enabled over the
base server). Flash is the program image; RAM is static `.data + .bss`. Regenerated by the
Feature Tables workflow from `docs/footprints.json`.

| Feature | Example | Flash (bytes) | Static RAM (bytes) |
| :------ | :------ | ------------: | -----------------: |
| `SIGFOX` | `Drivers/SigfoxUplink` | 267,961 | 21,464 |
| `ENOCEAN+GATEWAY` | `Drivers/EnOceanGateway` | 268,693 | 21,848 |
| `ZWAVE+GATEWAY` | `Drivers/ZWaveGateway` | 268,905 | 21,848 |
| `ZIGBEE+GATEWAY` | `Drivers/ZigbeeGateway` | 269,237 | 22,104 |
| `DMA+PREEMPT_QUEUE+DMA_SIMULATE` | `Peripherals/DmaIngest` | 269,385 | 28,600 |
| `core/SSHCryptoSelfTest` | `L5-Session/SSHCryptoSelfTest` | 269,537 | 21,476 |
| `SEN0192` | `Drivers/Sen0192` | 269,805 | 21,488 |
| `DMA+PREEMPT_QUEUE+GATEWAY+DMA_SIMULATE` | `Drivers/RadioGateway` | 270,541 | 28,720 |
| `LD2410` | `Drivers/Ld2410` | 270,681 | 21,576 |
| `DMA+PREEMPT_QUEUE+FORWARD+DMA_SIMULATE` | `Foundation/InterfaceForward` | 270,797 | 29,096 |
| `THREAD+GATEWAY` | `Drivers/ThreadGateway` | 271,797 | 22,616 |
| `PREEMPT_QUEUE` | `Foundation/PreemptQueue` | 274,069 | 23,968 |
| `NRF24+GATEWAY` | `Drivers/Nrf24Gateway` | 276,105 | 21,680 |
| `LORA+GATEWAY` | `Drivers/LoRaGateway` | 276,329 | 21,688 |
| `PCA9685` | `Drivers/Pca9685` | 284,601 | 21,800 |
| `ADS1115` | `Drivers/Ads1115` | 286,921 | 21,800 |
| `INA219` | `Drivers/Ina219` | 287,001 | 21,800 |
| `SHT3X` | `Drivers/Sht3x` | 287,029 | 21,800 |
| `MPR121` | `Drivers/Mpr121` | 287,693 | 21,808 |
| `PN532+GATEWAY` | `Drivers/NfcGateway` | 288,129 | 21,920 |
| `core/EthernetW5500` | `Peripherals/EthernetW5500` | 469,573 | 73,672 |
| `HISLIP` | `L7-Application/HiSlip` | 725,029 | 44,068 |
| `GPIB` | `L7-Application/Gpib` | 725,165 | 43,588 |
| `LSV2` | `L7-Application/HeidenhainLsv2` | 725,313 | 44,068 |
| `SCPI` | `L7-Application/Scpi` | 725,325 | 43,812 |
| `HAAS_MDC` | `L7-Application/HaasMdc` | 725,337 | 43,452 |
| `DNS_SERVER` | `L7-Application/DnsServer` | 725,929 | 46,044 |
| `IKEV2` | `L5-Session/IKEv2` | 725,953 | 43,940 |
| `WIFI_SNIFFER+PROMISC` | `Peripherals/WifiSniffer` | 725,973 | 43,644 |
| `VXI11` | `L7-Application/Vxi11` | 726,205 | 44,196 |
| `COAP+COAP_BLOCK+COAP_MAX_PAYLOAD` | `L7-Application/CoapBlock` | 727,597 | 48,420 |
| `UDP_TELEMETRY` | `L7-Application/UdpTelemetry` | 728,197 | 45,012 |
| `SNMP+SNMP_TRAP` | `L7-Application/SnmpTrap` | 728,421 | 44,988 |
| `STATSD` | `L7-Application/StatsdMetrics` | 728,453 | 45,148 |
| `COAP+COAP_OBSERVE` | `L7-Application/CoapObserve` | 729,669 | 46,172 |
| `ESPNOW` | `L7-Application/EspNow` | 731,525 | 43,580 |
| `DNC` | `L7-Application/EthernetDnc` | 733,861 | 61,124 |
| `HTTP_CLIENT` | `L7-Application/HttpClient` | 734,625 | 63,180 |
| `SMTP` | `L7-Application/SmtpAlert` | 735,065 | 61,132 |
| `MQTT` | `L7-Application/MqttClient` | 736,465 | 65,340 |
| `SMB` | `L7-Application/SmbFileClient` | 742,761 | 65,220 |
| `NTP_SERVER+TIME_SOURCE+NMEA0183+NTP` | `L7-Application/NtpServer` | 748,669 | 46,708 |
| `ACCEPT_THROTTLE` | `L4-Transport/AcceptThrottle` | 752,581 | 81,796 |
| `core/CORS` | `L7-Application/CORS` | 752,749 | 81,788 |
| `core/DigestAuth` | `L6-Presentation/DigestAuth` | 752,825 | 81,788 |
| `core/BasicAuth` | `L6-Presentation/BasicAuth` | 752,837 | 81,788 |
| `RADIO_POWER+RADIO_WIFI_PS` | `L7-Application/RadioPower` | 752,841 | 81,788 |
| `core/MediaStreaming` | `L7-Application/MediaStreaming` | 752,889 | 81,788 |
| `core/RegexRoutes` | `L7-Application/RegexRoutes` | 752,941 | 81,788 |
| `KEEPALIVE` | `L4-Transport/KeepAlive` | 752,985 | 81,788 |
| `DEVICE_ID` | `L7-Application/DeviceUuid` | 753,041 | 81,828 |
| `core/PathParams` | `L7-Application/PathParams` | 753,069 | 81,788 |
| `PER_IP_THROTTLE` | `L4-Transport/PerIpThrottle` | 753,137 | 82,236 |
| `GUARDRAILS` | `L7-Application/Guardrails` | 753,141 | 81,804 |
| `core/ResponseHeaders` | `L7-Application/ResponseHeaders` | 753,201 | 81,788 |
| `core/WebSocket` | `L6-Presentation/WebSocket` | 753,209 | 81,788 |
| `core/Middleware` | `L7-Application/Middleware` | 753,297 | 81,796 |
| `ADS` | `L7-Application/AdsClient` | 753,317 | 44,204 |
| `core/ChunkedResponse` | `L7-Application/ChunkedResponse` | 753,421 | 81,804 |
| `DIAG` | `L7-Application/Diagnostics` | 753,437 | 81,788 |
| `core/ServerSentEvents` | `L6-Presentation/ServerSentEvents` | 753,465 | 81,796 |
| `core/NetEgress` | `L7-Application/NetEgress` | 753,501 | 81,788 |
| `core/FormParams` | `L6-Presentation/FormParams` | 753,521 | 81,788 |
| `PARTITION_MONITOR` | `L7-Application/PartitionMonitor` | 753,569 | 81,796 |
| `OTA_ROLLBACK` | `L7-Application/OtaRollback` | 753,829 | 81,804 |
| `AUTH_LOCKOUT` | `L6-Presentation/AuthLockout` | 753,937 | 82,364 |
| `TOTP` | `L7-Application/Totp` | 753,989 | 81,828 |
| `core/Multipart` | `L6-Presentation/Multipart` | 754,017 | 81,788 |
| `LOGBUF` | `L7-Application/LogBuffer` | 754,149 | 84,916 |
| `IP_ALLOWLIST` | `L4-Transport/IpAllowlist` | 754,221 | 81,788 |
| `core/InterfaceFilter` | `L7-Application/InterfaceFilter` | 754,449 | 81,788 |
| `CSRF` | `L7-Application/Csrf` | 754,477 | 81,844 |
| `MODBUS` | `L7-Application/ModbusTcp` | 754,493 | 82,076 |
| `core/Templating` | `L7-Application/Templating` | 754,505 | 81,836 |
| `STATS` | `L7-Application/Stats` | 754,737 | 81,892 |
| `CONTROL` | `L7-Application/PidTuning` | 754,873 | 89,868 |
| `MODBUS+MODBUS_MASTER` | `L7-Application/ModbusScan` | 754,957 | 82,068 |
| `JWT` | `L6-Presentation/JWTAuth` | 754,961 | 82,940 |
| `core/Basic` | `Foundation/Basic` | 754,981 | 81,804 |
| `TELNET` | `L5-Session/Telnet` | 755,153 | 82,332 |
| `AUDIT_LOG` | `L7-Application/AuditLog` | 755,213 | 84,780 |
| `CBOR` | `L6-Presentation/Cbor` | 755,245 | 81,868 |
| `IPV6` | `Foundation/IPv6` | 755,449 | 81,788 |
| `core/Expert` | `Foundation/Expert` | 755,657 | 81,812 |
| `SYSLOG` | `L7-Application/Syslog` | 756,437 | 83,700 |
| `MSGPACK` | `L6-Presentation/MsgPack` | 756,589 | 81,868 |
| `STATS+METRICS` | `L7-Application/PrometheusMetrics` | 756,729 | 81,932 |
| `core/Json` | `L6-Presentation/Json` | 756,873 | 81,796 |
| `AUTH_LOCKOUT+FORWARDED_TRUST` | `L6-Presentation/ForwardedTrust` | 757,097 | 82,404 |
| `GPIO_MAP` | `L7-Application/GpioMap` | 757,261 | 81,852 |
| `WEB_TERMINAL` | `L6-Presentation/WebTerminal` | 757,569 | 81,876 |
| `WS_DEFLATE` | `L6-Presentation/WebSocketCompression` | 757,589 | 90,004 |
| `GRAPHQL` | `L7-Application/GraphQL` | 757,701 | 86,204 |
| `CONFIG_STORE+CONFIG_IO` | `L7-Application/ConfigExport` | 757,805 | 81,856 |
| `OTA` | `L7-Application/OTA` | 758,353 | 102,132 |
| `COAP` | `L7-Application/CoAP` | 758,781 | 84,356 |
| `DNS_RESOLVER` | `L7-Application/DnsResolver` | 758,921 | 83,076 |
| `PROVISIONING` | `L7-Application/Provisioning` | 761,101 | 83,424 |
| `OPCUA` | `L7-Application/OpcUa` | 761,201 | 92,076 |
| `core/Advanced` | `Foundation/Advanced` | 761,585 | 81,900 |
| `TELEMETRY` | `L7-Application/Telemetry` | 761,829 | 82,112 |
| `SNMP` | `L7-Application/SNMP` | 762,053 | 94,244 |
| `RELAY` | `L7-Application/PortForward` | 762,649 | 116,404 |
| `HTTP_CLIENT+WEBHOOK` | `L7-Application/Webhook` | 763,453 | 101,548 |
| `OPCUA+OPCUA_CLIENT` | `L7-Application/OpcUaClient` | 765,585 | 94,692 |
| `OAUTH2+HTTP_CLIENT` | `L7-Application/OAuth2` | 765,793 | 104,612 |
| `PROMISC+FORWARD+ETHERNET` | `Peripherals/WifiCapture` | 766,049 | 47,584 |
| `OIDC` | `L7-Application/OidcAuth` | 766,741 | 99,844 |
| `core/Sysadmin` | `Foundation/Sysadmin` | 766,777 | 81,804 |
| `RTC+TIME_SOURCE+NTP` | `Drivers/Rtc` | 767,001 | 45,388 |
| `OPCUA+UMATI` | `L7-Application/Umati` | 767,765 | 92,220 |
| `NTRIP_CASTER` | `L7-Application/NtripCaster` | 770,769 | 84,712 |
| `BUS_CAPTURE+FORWARD+ETHERNET` | `Peripherals/CanCapture` | 771,997 | 45,568 |
| `DASHBOARD` | `L7-Application/Dashboard` | 773,657 | 82,164 |
| `EDGE_CACHE+HTTP_CACHE+HTTP_CLIENT` | `L7-Application/EdgeCache` | 773,961 | 119,276 |
| `NTP+TIME_SOURCE` | `L7-Application/TimeSourceFallback` | 774,333 | 83,396 |
| `MDNS` | `L7-Application/mDNS` | 778,345 | 83,688 |
| `EDGE_CACHE+HTTP_CACHE+HTTP_CLIENT+EDGE_MESH` | `L7-Application/MeshCache` | 778,433 | 124,240 |
| `EDGE_CACHE+HTTP_CACHE+HTTP_CLIENT+EDGE_MESH+EDGE_CACHE_SLOTS+EDGE_FETCH_SLOTS+MESH_MAX_PEERS` | `L7-Application/MeshCache` | 778,509 | 115,332 |
| `NTP` | `L7-Application/SNTP` | 778,853 | 84,328 |
| `IFACE_BRIDGE` | `L7-Application/InterfaceBridge` | 780,821 | 82,628 |
| `COAP+DTLS` | `L7-Application/CoapSecure` | 781,245 | 102,924 |
| `ETHERNET` | `Peripherals/Ethernet` | 792,009 | 81,848 |
| `ETHERNET+ETH_W5500+ETH_W5500_CS+ETH_W5500_RST+ETH_W5500_INT+ETH_W5500_SCK+ETH_W5500_MISO+ETH_W5500_MOSI` | `Peripherals/EthernetW5500` | 792,041 | 81,848 |
| `core/FileServing` | `L7-Application/FileServing` | 794,249 | 81,828 |
| `UPLOAD` | `L7-Application/FileUpload` | 795,209 | 91,140 |
| `RANGE` | `L7-Application/Range` | 795,401 | 81,828 |
| `VFS` | `L7-Application/Vfs` | 796,429 | 86,316 |
| `WEBDAV` | `L7-Application/WebDav` | 821,069 | 105,352 |
| `WEBDAV+WEBDAV_MAX_ENTRIES+WEBDAV_BUF_SIZE` | `L7-Application/WebDav` | 822,237 | 90,860 |
| `ETAG` | `L7-Application/ETag` | 829,073 | 83,092 |
| `SSH` | `L5-Session/SSHHostKey` | 829,481 | 109,176 |
| `WS_CLIENT+TLS+WS_CLIENT_TLS` | `L7-Application/WebSocketClient` | 831,333 | 120,548 |
| `WS_CLIENT+TLS+WS_CLIENT_TLS+WS_CLIENT_BUF_SIZE` | `L7-Application/WebSocketClient` | 831,745 | 123,620 |
| `WS_CLIENT+TLS+WS_CLIENT_TLS+WS_CLIENT_BUF_SIZE+TLS_ARENA_SIZE` | `L7-Application/WebSocketClient` | 832,497 | 107,256 |
| `HOTSWAP` | `L7-Application/HotSwapStorage` | 838,441 | 82,728 |
| `EXC_DECODER+FTP+FTP_SESSION` | `L7-Application/CoreDump` | 841,337 | 83,428 |
| `HTTP_DELIVERY+FILE_SERVING+RANGE` | `L7-Application/HttpDelivery` | 842,725 | 82,704 |
| `TLS` | `L6-Presentation/SecureWebSocket` | 855,873 | 122,020 |
| `TLS+TLS_RESUMPTION` | `L4-Transport/TlsResumption` | 856,693 | 122,180 |
| `TLS+MTLS` | `L4-Transport/mTLS` | 856,829 | 122,356 |
| `TLS+TLS_ARENA_SIZE` | `L6-Presentation/SecureWebSocket` | 857,077 | 105,664 |
| `TLS+TLS_RESUMPTION+TLS_ARENA_SIZE` | `L4-Transport/TlsResumption` | 858,001 | 105,824 |
| `TLS+MTLS+TLS_ARENA_SIZE` | `L4-Transport/mTLS` | 858,105 | 106,000 |
| `POWER_MGMT` | `L7-Application/PowerGovernor` | 881,293 | 85,648 |
| `SSH+FILE_SERVING+SSH_SFTP+SSH_SCP` | `L5-Session/SSHSftp` | 881,781 | 118,528 |

<!-- END GENERATED BUILD-FOOTPRINT -->

</details>

## Zero Heap Allocation

Every byte of memory the library uses is accounted for at compile time:

<details>
<summary><b>View Zero Heap Allocation Storage Details</b></summary>

| Storage                                                                        | Location                                         |
| ------------------------------------------------------------------------------ | ------------------------------------------------ |
| `conn_pool[MAX_CONNS]` - TCP connections + ring buffers                        | BSS                                              |
| `http_pool[MAX_CONNS]` - HTTP request structs                                  | BSS                                              |
| `ws_pool[MAX_WS_CONNS]` - WebSocket connection state                           | BSS                                              |
| `sse_pool[MAX_SSE_CONNS]` - SSE connection state                               | BSS                                              |
| `_queue_storage[EVT_QUEUE_DEPTH * sizeof(TcpEvt)]` - event queue backing store | BSS                                              |
| `_queue_struct` - FreeRTOS `StaticQueue_t`                                     | BSS                                              |
| Route table `_routes[MAX_ROUTES]`                                              | BSS (inside [`DetWebServer`](@ref DetWebServer)) |

</details>

[`begin()`](@ref DetWebServer::begin) calls `xQueueCreateStatic()` - no `pvPortMalloc`, no fragmentation risk. The library makes no heap allocations.

The only post-`begin()` allocation that can occur is inside `fs::File` construction in `serve_file()`, which is an Arduino FS implementation detail outside the library's control.

Every pool above is a fixed BSS array sized from the compile-time constants, so the memory cost is exactly what the configuration says - it never grows at runtime. For the measured flash and static-RAM cost of each optional feature, see the [Build Footprint](#build-footprint) table above.

## Feature Flags & Configuration

> [!IMPORTANT]
> **Use Build Flags (`-D...`), Not Sketch `#define`s!**
>
> Because PlatformIO (and standard Arduino IDE builds) compiles the library's source files (`.cpp`) independently from your sketch (`.ino` / `.cpp`), `#define` macros inside your sketch files **do not propagate** to the library's pre-compiled objects.
>
> Declaring configuration or feature macros like `#define DETWS_ENABLE_PROVISIONING 1` inside your `.ino` sketch file before the `#include` will result in configuration mismatches, linker errors (such as undefined symbols), or unstable behavior at runtime.
>
> To enable/disable features or override configuration constants, you **must** pass them as compiler build flags. For example, in PlatformIO, define them inside `platformio.ini` under `build_flags`:
>
> ```ini
> [env:esp32dev]
> platform = espressif32
> board = esp32dev
> framework = arduino
> build_flags =
>     -DDETWS_ENABLE_PROVISIONING=1
>     -DDETWS_ENABLE_WEBSOCKET=0
>     -DMAX_CONNS=6
> ```

Any feature flag set to `0` strips the corresponding code and its includes from the build entirely.

### Feature Flags

The complete set of `DETWS_ENABLE_*` flags and their defaults, scraped from
`src/ServerConfig.h` by `docs/utilities/gen_readme_sections.py` (see
[FEATURES.md](FEATURES.md) for the full description of each):

<details>
<summary><b>All feature flags and their defaults</b></summary>

<!-- BEGIN GENERATED FEATURE-FLAGS (docs/utilities/gen_readme_sections.py) -->

| Flag | Default | Description |
| :--- | :-----: | :---------- |
| `DWS_ENABLE_ACCEPT_THROTTLE` | `0` | Opt-in global accept-rate throttle (connection-flood defense). |
| `DWS_ENABLE_ADS` | `0` | Beckhoff ADS / AMS protocol codec (`services/ads`). |
| `DWS_ENABLE_ADS1115` | `0` | TI ADS1115 16-bit ADC (I2C) - a precise external analog input. |
| `DWS_ENABLE_AMQP` | `0` | AMQP 0-9-1 frame codec (`services/amqp`). |
| `DWS_ENABLE_ATC` | `0` | Opt-in ATC (Advanced Traffic Controller) field-I/O interop snapshot. |
| `DWS_ENABLE_AUDIT_LOG` | `0` | Tamper-evident audit log. |
| `DWS_ENABLE_AUTH` | `1` | HTTP Basic Authentication per-route. |
| `DWS_ENABLE_AUTH_LOCKOUT` | `0` | Opt-in per-IP brute-force lockout for HTTP auth (requires DWS_ENABLE_AUTH). |
| `DWS_ENABLE_BACNET` | `0` | BACnet/IP BVLC + NPDU codec (`services/bacnet`). |
| `DWS_ENABLE_BLE_GATT` | `0` | Opt-in Bluetooth ATT protocol codec + GATT characteristic bridge. |
| `DWS_ENABLE_BUS_CAPTURE` | `0` | Wired field-bus listen-only capture. |
| `DWS_ENABLE_C37118` | `0` | IEEE C37.118.2 synchrophasor frame codec (`services/c37118`). |
| `DWS_ENABLE_CANOPEN` | `0` | CANopen (CiA 301) message codec (`services/canopen`). |
| `DWS_ENABLE_CBOR` | `0` | Zero-heap CBOR (RFC 8949) encoder for compact binary payloads. |
| `DWS_ENABLE_CC1101` | `0` | Opt-in CC1101 sub-GHz radio driver. |
| `DWS_ENABLE_CCLINK` | `0` | Opt-in CC-Link (CLPA) cyclic fieldbus frame codec. |
| `DWS_ENABLE_CIA402` | `0` | CiA 402 / IEC 61800-7-201 drive + motion profile (`services/cia402`). |
| `DWS_ENABLE_CIP` | `0` | CIP (Common Industrial Protocol) message codec (`services/cip`). |
| `DWS_ENABLE_CLOUDEVENTS` | `0` | CloudEvents v1.0 (CNCF) event envelope (structured JSON + binary headers). |
| `DWS_ENABLE_COAP` | `0` | CoAP server (RFC 7252) over UDP/5683. |
| `DWS_ENABLE_COAP_BLOCK` | `0` | CoAP block-wise transfer - RFC 7959 (requires DWS_ENABLE_COAP). |
| `DWS_ENABLE_COAP_OBSERVE` | `0` | CoAP resource observation - RFC 7641 (requires DWS_ENABLE_COAP). |
| `DWS_ENABLE_CONFIG_IO` | `0` | Opt-in schema-driven config export / restore. |
| `DWS_ENABLE_CONFIG_STORE` | `0` | Typed NVS configuration store (WiFi creds, IP config, ... |
| `DWS_ENABLE_CONTROL` | `0` | Closed-loop control law (`services/control`). |
| `DWS_ENABLE_COTP` | `0` | TPKT (RFC 1006) + COTP (X.224 class 0) frame codec (`services/cotp`). |
| `DWS_ENABLE_CSRF` | `0` | Opt-in CSRF protection for state-changing HTTP requests. |
| `DWS_ENABLE_DASHBOARD` | `0` | Real-time SVG dashboard (DWS_ENABLE_DASHBOARD; requires DWS_ENABLE_SSE). |
| `DWS_ENABLE_DBM` | `0` | Opt-in dbm: a log-structured hash key-value store on the WAL (DWS_ENABLE_DBM, requires WAL). |
| `DWS_ENABLE_DDS` | `0` | Opt-in DDS / RTPS wire-protocol codec. |
| `DWS_ENABLE_DEVICENET` | `0` | DeviceNet link-adaptation codec (`services/devicenet`). |
| `DWS_ENABLE_DEVICE_ID` | `0` | Stable device UUID derived from the chip MAC (RFC 4122 v5). |
| `DWS_ENABLE_DF1` | `0` | Allen-Bradley DF1 full-duplex frame codec (`services/df1`). |
| `DWS_ENABLE_DIAG` | `0` | Expose a diagnostic JSON endpoint via server.diag(). |
| `DWS_ENABLE_DIRECTNET` | `0` | Opt-in AutomationDirect / Koyo DirectNET serial frame codec. |
| `DWS_ENABLE_DMA` | `0` | Enable the DMA peripheral ingest / egress primitive (default off). |
| `DWS_ENABLE_DMX` | `0` | DMX512 + RDM (ANSI E1.20) lighting codec (`services/dmx`). |
| `DWS_ENABLE_DNC` | `0` | Opt-in CNC RS-232 DNC drip-feed codec. |
| `DWS_ENABLE_DNP3` | `0` | DNP3 (IEEE 1815) data-link frame codec (`services/dnp3`). |
| `DWS_ENABLE_DNS_RESOLVER` | `0` | Opt-in DNS resolver with answer verification. |
| `DWS_ENABLE_DNS_SERVER` | `0` | Authoritative DNS server (services/dns_server) on UDP/53. |
| `DWS_ENABLE_DOCSTORE` | `0` | Opt-in local JSON document store on the WAL (DWS_ENABLE_DOCSTORE, requires DBM + WAL). |
| `DWS_ENABLE_DSHOT` | `0` | Opt-in DShot ESC throttle protocol codec. |
| `DWS_ENABLE_DTLS` | `0` | DTLS 1.3 datagram security (RFC 9147) - the record layer. |
| `DWS_ENABLE_EDGE_CACHE` | `0` | Opt-in CDN edge-cache tier (DWS_ENABLE_EDGE_CACHE, requires HTTP_CACHE). |
| `DWS_ENABLE_EDGE_MESH` | `0` | Opt-in mesh (sibling-cache) distribution for the edge cache. |
| `DWS_ENABLE_EDGE_ORIGIN_TLS` | `0` |  |
| `DWS_ENABLE_ENIP` | `0` | EtherNet/IP encapsulation codec (`services/enip`). |
| `DWS_ENABLE_ENOCEAN` | `0` | Enable the EnOcean ESP3 serial codec (default off). |
| `DWS_ENABLE_ESPNOW` | `0` | ESP-NOW peer messaging. |
| `DWS_ENABLE_ETAG` | `0` | Conditional GET (ETag + Last-Modified) for served files. |
| `DWS_ENABLE_ETHERNET` | `0` | Enable wired Ethernet bring-up (init_eth_physical / eth_ready). |
| `DWS_ENABLE_EXC_DECODER` | `0` | Opt-in ESP32 panic / exception decoder for a live diagnostics panel. |
| `DWS_ENABLE_FAILSAFE` | `0` | Opt-in software watchdog: deadlock detection + fail-safe safe-state. |
| `DWS_ENABLE_FDC2214` | `0` | Opt-in FDC2114/2214 capacitance-to-digital field sensor. |
| `DWS_ENABLE_FILE_SERVING` | `1` | Static file serving via Arduino FS (LittleFS, SPIFFS, SD). |
| `DWS_ENABLE_FINS` | `0` | Omron FINS frame codec (`services/fins`). |
| `DWS_ENABLE_FLOW_EXPORT` | `0` | Flow-record export codec (`services/flow_export`). |
| `DWS_ENABLE_FOCAS` | `0` | FANUC FOCAS Ethernet protocol codec (`services/focas`). |
| `DWS_ENABLE_FORWARD` | `0` | Enable the interface forwarding plane (default off). |
| `DWS_ENABLE_FORWARDED_TRUST` | `0` | Believe a `Forwarded` / `X-Forwarded-For` client address only from a trusted upstream. |
| `DWS_ENABLE_FTP` | `0` | Opt-in FTP client wire codec. |
| `DWS_ENABLE_FTP_SESSION` | `0` | Opt-in FTP client session driver (DWS_ENABLE_FTP_SESSION, requires DWS_ENABLE_FTP). |
| `DWS_ENABLE_GATEWAY` | `0` | Enable the radio / wireless gateway bridge (default off). |
| `DWS_ENABLE_GOOSE` | `0` | Opt-in IEC 61850 GOOSE publisher codec. |
| `DWS_ENABLE_GPIB` | `0` | GPIB-over-LAN (Prologix-style) controller command codec (`services/gpib`). |
| `DWS_ENABLE_GPIO_MAP` | `0` | Opt-in browser GPIO pin-mapper / diagnostics endpoint. |
| `DWS_ENABLE_GRAPHQL` | `0` | GraphQL query subset. |
| `DWS_ENABLE_GRPC_WEB` | `0` | gRPC-Web message framing (`services/grpcweb`). |
| `DWS_ENABLE_GUARDRAILS` | `0` | Opt-in runtime heap/stack guardrails. |
| `DWS_ENABLE_HAAS_MDC` | `0` | Haas Machine Data Collection (MDC) Q-command codec (`services/haas_mdc`). |
| `DWS_ENABLE_HAPPY_EYEBALLS` | `0` | Opt-in dual-stack Happy Eyeballs destination selection. |
| `DWS_ENABLE_HART` | `0` | Opt-in HART / HART-IP process-instrument protocol codec. |
| `DWS_ENABLE_HISLIP` | `0` | HiSLIP (High-Speed LAN Instrument Protocol) message codec (`services/hislip`). |
| `DWS_ENABLE_HOSTLINK` | `0` | Omron Host Link (C-mode) frame codec (`services/hostlink`). |
| `DWS_ENABLE_HOTSWAP` | `0` | Opt-in removable-storage hot-swap safeties. |
| `DWS_ENABLE_HTTP2` | `0` | HTTP/2 (RFC 9113) over the version-agnostic request/response core. |
| `DWS_ENABLE_HTTP3` | `0` | HTTP/3 (RFC 9114) over QUIC (RFC 9000) - implemented, host-tested end-to-end (HW verification pending). |
| `DWS_ENABLE_HTTP_CACHE` | `0` | Opt-in HTTP Cache-Control directive helpers. |
| `DWS_ENABLE_HTTP_CLIENT` | `0` | Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS). |
| `DWS_ENABLE_HTTP_CLIENT_TLS` | `0` | HTTPS client support inside the HTTP client (needs DWS_ENABLE_TLS). |
| `DWS_ENABLE_HTTP_DELIVERY` | `0` | Opt-in HTTP delivery optimizations. |
| `DWS_ENABLE_HW_HEALTH` | `0` | Opt-in hardware-health diagnostics. |
| `DWS_ENABLE_ICCP` | `0` | Opt-in ICCP / TASE.2 (IEC 60870-6) inter-control-center telemetry codec. |
| `DWS_ENABLE_IEC60870` | `0` | IEC 60870-5-101 / -104 telecontrol (SCADA) codec (`services/iec60870`). |
| `DWS_ENABLE_IFACE_BRIDGE` | `0` | User-defined address:port -> hardware-bus bridge (services/iface_bridge). |
| `DWS_ENABLE_IKEV2` | `0` | IKEv2 (RFC 7296) message + payload codec (`services/ikev2`). |
| `DWS_ENABLE_INA219` | `0` | TI INA219 high-side current / power monitor (I2C). |
| `DWS_ENABLE_INTERBUS` | `0` | Opt-in INTERBUS summation-frame fieldbus codec. |
| `DWS_ENABLE_IOLINK` | `0` | IO-Link (SDCI, IEC 61131-9) data-link message codec (`services/iolink`). |
| `DWS_ENABLE_IPV6` | `0` | Enable IPv6 on the network interface (dual-stack). |
| `DWS_ENABLE_IP_ALLOWLIST` | `0` | Opt-in source-IP allowlist (accept-time firewall, IPv4 and IPv6). |
| `DWS_ENABLE_J1939` | `0` | SAE J1939 message codec (`services/j1939`). |
| `DWS_ENABLE_J2735` | `0` | Opt-in SAE J2735 V2X codec. |
| `DWS_ENABLE_JWT` | `0` | JWT bearer-token authentication (HS256). |
| `DWS_ENABLE_KEEPALIVE` | `1` | HTTP/1.1 persistent connections (keep-alive). |
| `DWS_ENABLE_LD2410` | `0` | HLK-LD2410 24 GHz mmWave presence / motion radar (UART). |
| `DWS_ENABLE_LDC1614` | `0` | Opt-in LDC1614 inductance-to-digital field sensor. |
| `DWS_ENABLE_LINK_MANAGER` | `0` | Opt-in multi-interface egress selection / failover policy. |
| `DWS_ENABLE_LOGBUF` | `0` | Opt-in fixed-RAM rotating log buffer with severity traps. |
| `DWS_ENABLE_LONWORKS` | `0` | Opt-in LonWorks / LON-IP (ISO/IEC 14908) network-variable codec. |
| `DWS_ENABLE_LORA` | `0` | Enable the LoRa (SX127x) radio codec + driver (default off). |
| `DWS_ENABLE_LSV2` | `0` | Heidenhain LSV/2 telegram codec (`services/lsv2`). |
| `DWS_ENABLE_LWM2M` | `0` | OMA LwM2M TLV codec (`services/lwm2m`). |
| `DWS_ENABLE_MBPLUS` | `0` | Opt-in Modbus Plus HDLC token-bus frame codec. |
| `DWS_ENABLE_MBUS` | `0` | Wired M-Bus (Meter-Bus, EN 13757) frame codec (`services/mbus`). |
| `DWS_ENABLE_MDNS` | `0` | mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS. |
| `DWS_ENABLE_MDNS_ADAPTIVE` | `0` | Opt-in adaptive mDNS beacon scheduling. |
| `DWS_ENABLE_MELSEC` | `0` | Mitsubishi MELSEC MC protocol (binary 3E) codec (`services/melsec`). |
| `DWS_ENABLE_METRICS` | `0` | Prometheus `/metrics` endpoint (text exposition format 0.0.4). |
| `DWS_ENABLE_MMS` | `0` | Opt-in IEC 61850 MMS PDU codec. |
| `DWS_ENABLE_MODBUS` | `0` | Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502. |
| `DWS_ENABLE_MODBUS_MASTER` | `0` | Opt-in Modbus master codec + register scanner. |
| `DWS_ENABLE_MODBUS_RTU` | `0` | Modbus RTU framing (serial / RS-485) over the same data model + PDU dispatch. |
| `DWS_ENABLE_MPR121` | `0` | NXP MPR121 12-channel capacitive-touch controller (I2C). |
| `DWS_ENABLE_MQTT` | `0` | MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS). |
| `DWS_ENABLE_MQTT_SN` | `0` | MQTT-SN v1.2 wire codec (`services/mqtt/mqtt_sn`). |
| `DWS_ENABLE_MQTT_TLS` | `0` | MQTTS: run the MQTT client over client-side TLS (needs DWS_ENABLE_TLS). |
| `DWS_ENABLE_MSGPACK` | `0` | Zero-heap MessagePack encoder and decoder for compact binary payloads. |
| `DWS_ENABLE_MTCONNECT` | `0` | Opt-in MTConnect agent response codec. |
| `DWS_ENABLE_MTLS` | `0` | Mutual TLS - require and verify a client certificate (mTLS). |
| `DWS_ENABLE_MULTIPART` | `1` | multipart/form-data body parser. |
| `DWS_ENABLE_NATS` | `0` | NATS client protocol codec (`services/nats`). |
| `DWS_ENABLE_NEMA_TS2` | `0` | Opt-in NEMA TS 2 traffic-cabinet SDLC frame codec. |
| `DWS_ENABLE_NETADAPT` | `0` | Opt-in network adaptation decisions. |
| `DWS_ENABLE_NMEA0183` | `0` | NMEA 0183 sentence codec (`services/nmea0183`). |
| `DWS_ENABLE_NMEA2000` | `0` | NMEA 2000 codec (`services/nmea2000`). |
| `DWS_ENABLE_NRF24` | `0` | Enable the nRF24L01+ radio driver (default off). |
| `DWS_ENABLE_NTCIP` | `0` | Opt-in NTCIP transportation-device object identifiers. |
| `DWS_ENABLE_NTP` | `0` | SNTP wall-clock time sync via the ESP-IDF SNTP client. |
| `DWS_ENABLE_NTP_SERVER` | `0` | NTP/SNTP time server (RFC 5905 / RFC 4330 server mode) on UDP/123 (services/dws_ntp_server). |
| `DWS_ENABLE_NTRIP_CASTER` | `0` | GNSS RTK base station + NTRIP caster (services/gnss). |
| `DWS_ENABLE_NTS` | `0` | Opt-in Network Time Security (NTS, RFC 8915) wire codec. |
| `DWS_ENABLE_OAUTH2` | `0` | OAuth2 token-endpoint client. |
| `DWS_ENABLE_OBSERVABILITY` | `0` | Transport-layer observability: connection event hook + counters. |
| `DWS_ENABLE_OCIT` | `0` | Opt-in OCIT-Outstations message codec. |
| `DWS_ENABLE_OIDC` | `0` | OpenID Connect ID-token verification, RS256. |
| `DWS_ENABLE_OPCUA` | `0` | OPC UA Binary server. |
| `DWS_ENABLE_OPCUA_CLIENT` | `0` | OPC UA Binary client. |
| `DWS_ENABLE_OPENADR` | `0` | Opt-in OpenADR 3.0 (Automated Demand Response) JSON codec. |
| `DWS_ENABLE_OTA` | `0` | Authenticated OTA firmware update (streaming POST to the ESP32 Update API). |
| `DWS_ENABLE_OTA_ROLLBACK` | `0` | Opt-in OTA rollback protection / soft-brick safeguard. |
| `DWS_ENABLE_PARTITION_MONITOR` | `0` | Opt-in flash partition-map monitor endpoint. |
| `DWS_ENABLE_PCA9685` | `0` | NXP PCA9685 16-channel 12-bit PWM / servo driver (I2C). |
| `DWS_ENABLE_PER_IP_THROTTLE` | `0` | Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4). |
| `DWS_ENABLE_PN532` | `0` | Enable the PN532 NFC frame codec (default off). |
| `DWS_ENABLE_POWERLINK` | `0` | Opt-in Ethernet POWERLINK (EPSG) basic frame codec. |
| `DWS_ENABLE_POWER_MGMT` | `0` | Opt-in SoC power governor. |
| `DWS_ENABLE_PQC_KEX` | `0` | Post-quantum hybrid key exchange: ML-KEM-768 + X25519 (FIPS 203 / RFC 9370 combiner). |
| `DWS_ENABLE_PREEMPT_QUEUE` | `0` | Enable the preempting work queue primitive (default off). |
| `DWS_ENABLE_PROFIBUS` | `0` | Opt-in PROFIBUS-DP FDL telegram codec. |
| `DWS_ENABLE_PROFINET` | `0` | Opt-in PROFINET DCP (Discovery and Configuration Protocol) frame codec. |
| `DWS_ENABLE_PROMISC` | `0` | Wi-Fi promiscuous (monitor) capture. |
| `DWS_ENABLE_PROTOBUF` | `0` | Protocol Buffers wire codec (`services/protobuf`). |
| `DWS_ENABLE_PROVISIONING` | `0` | First-boot WiFi provisioning: softAP + captive-portal credentials form. |
| `DWS_ENABLE_PROXY_PROTOCOL` | `0` | HAProxy PROXY protocol codec (`services/proxy_protocol`). |
| `DWS_ENABLE_PSRAM_POOL` | `0` | Opt-in buffer placement policy (DRAM vs PSRAM) + SPI DMA ping-pong manager. |
| `DWS_ENABLE_RADIO_POWER` | `0` | Opt-in radio power controls. |
| `DWS_ENABLE_RADIO_SNIFF` | `0` | Opt-in receive-only radio channel sniffer to pcap. |
| `DWS_ENABLE_RANGE` | `0` | HTTP Range requests / 206 Partial Content (requires DWS_ENABLE_FILE_SERVING or DWS_ENABLE_EDGE_CACHE). |
| `DWS_ENABLE_RAWL2` | `0` | Opt-in raw Layer-2 Ethernet frame codec. |
| `DWS_ENABLE_REDIS` | `0` | Redis RESP2 wire codec (`services/redis_resp`). |
| `DWS_ENABLE_REDIS` | `0` | Redis RESP2 wire codec (`services/redis_resp`). |
| `DWS_ENABLE_RELAY` | `0` | Opt-in TCP relay / DNAT port forwarding. |
| `DWS_ENABLE_RTC` | `0` | I2C real-time-clock driver (DS1307 / DS3231) - a battery-backed time source. |
| `DWS_ENABLE_S7COMM` | `0` | Siemens S7comm PDU codec (`services/s7comm`). |
| `DWS_ENABLE_SCPI` | `0` | SCPI / IEEE 488.2 instrument-control codec (`services/scpi`). |
| `DWS_ENABLE_SDI12` | `0` | SDI-12 sensor-bus codec (`services/sdi12`). |
| `DWS_ENABLE_SEN0192` | `0` | DFRobot SEN0192 10.525 GHz microwave Doppler motion sensor (single digital OUT line). |
| `DWS_ENABLE_SENML` | `0` | SenML (RFC 8428) measurement-pack builder (`services/senml`). |
| `DWS_ENABLE_SEP2` | `0` | Opt-in IEEE 2030.5 (Smart Energy Profile 2.0) resource codec. |
| `DWS_ENABLE_SERCOS` | `0` | Opt-in SERCOS III motion-bus telegram codec. |
| `DWS_ENABLE_SHT3X` | `0` | Sensirion SHT3x temperature / humidity sensor (I2C). |
| `DWS_ENABLE_SIGFOX` | `0` | Enable the Sigfox AT-command codec (default off). |
| `DWS_ENABLE_SLEEP_SCHED` | `0` | Opt-in dynamic sleep-cycle scheduler. |
| `DWS_ENABLE_SMB` | `0` | Opt-in SMB2 client. |
| `DWS_ENABLE_SMTP` | `0` | Outbound SMTP client (RFC 5321) for device email alerts (services/smtp). |
| `DWS_ENABLE_SMTP_TLS` | `0` | Secure SMTP: run the mail client over client-side TLS (needs DWS_ENABLE_TLS). |
| `DWS_ENABLE_SNMP` | `0` | SNMP agent (v1/v2c, + v3 USM when DWS_ENABLE_SNMP_V3) over lwIP UDP. |
| `DWS_ENABLE_SNMP_TRAP` | `0` | Outbound SNMP notifications - traps and informs (requires DWS_ENABLE_SNMP). |
| `DWS_ENABLE_SNMP_V3` | `0` | Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). |
| `DWS_ENABLE_SNP` | `0` | Opt-in GE Fanuc SNP (Series Ninety Protocol) serial frame codec. |
| `DWS_ENABLE_SOCKPOOL` | `0` | Opt-in dynamic socket recycling: an LRU connection-slot pool. |
| `DWS_ENABLE_SOUTHBOUND` | `0` | Opt-in southbound protocol-driver framework. |
| `DWS_ENABLE_SPARKPLUG` | `0` | Sparkplug B payload + topic codec (`services/sparkplug`). |
| `DWS_ENABLE_SPA_ROUTER` | `0` | Opt-in single-page-app micro-routing decision. |
| `DWS_ENABLE_SQLITE` | `0` | Opt-in SQLite3 on-disk file-format reader. |
| `DWS_ENABLE_SSE` | `1` | Server-Sent Events push support. |
| `DWS_ENABLE_SSH` | `0` | SSH server support (RFC 4253/4252/4254). |
| `DWS_ENABLE_SSH_SCP` | `0` | SCP server over SSH (the legacy RCP protocol via `exec "scp -t/-f"`). |
| `DWS_ENABLE_SSH_SFTP` | `0` | SFTP server subsystem over SSH (SSH_FXP_* v3, draft-ietf-secsh-filexfer-02). |
| `DWS_ENABLE_SSH_ZLIB` | `0` | SSH server-to-client compression (`zlib@openssh.com` / `zlib`, RFC 4253 sec 6.2). |
| `DWS_ENABLE_STATS` | `0` | Runtime stats endpoint (uptime, request/error counts, pool usage, heap). |
| `DWS_ENABLE_STATSD` | `0` | Opt-in StatsD metrics client. |
| `DWS_ENABLE_STOMP` | `0` | STOMP 1.2 frame codec (`services/stomp`). |
| `DWS_ENABLE_SUNSPEC` | `0` | SunSpec Modbus device-information-model codec (`services/sunspec`). |
| `DWS_ENABLE_SYSLOG` | `0` | Syslog client (RFC 5424 over UDP). |
| `DWS_ENABLE_TELEMETRY` | `0` | Telemetry math helpers (moving-window stats, rate-of-change, totalizer). |
| `DWS_ENABLE_TELNET` | `0` | Telnet server support (RFC 854 / IAC option negotiation). |
| `DWS_ENABLE_THEMES` | `0` | Embed the theme stylesheet library as runtime-selectable blobs (default off). |
| `DWS_ENABLE_THREAD` | `0` | Enable the Thread spinel / HDLC-lite framing codec (default off). |
| `DWS_ENABLE_TIME_SOURCE` | `0` | Multi-source time fallback (NTP / RTC / GPS / ... |
| `DWS_ENABLE_TLS` | `0` | TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only). |
| `DWS_ENABLE_TLS_POLICY` | `0` | Opt-in TLS version negotiation + pinned cipher-suite policy. |
| `DWS_ENABLE_TLS_RESUMPTION` | `0` | TLS session resumption via RFC 5077 session tickets (requires DWS_ENABLE_TLS). |
| `DWS_ENABLE_TOTP` | `0` | Opt-in TOTP two-factor auth (RFC 6238). |
| `DWS_ENABLE_UDP_TELEMETRY` | `0` | Opt-in fire-and-forget UDP telemetry cast. |
| `DWS_ENABLE_UMATI` | `0` | umati - OPC UA for Machine Tools information model. |
| `DWS_ENABLE_UPLOAD` | `0` | Streaming file upload: POST a body straight to a file on the filesystem. |
| `DWS_ENABLE_UTMC` | `0` | Opt-in UTMC (Urban Traffic Management and Control) common-database codec. |
| `DWS_ENABLE_VFS` | `0` | Unified virtual filesystem wrapper. |
| `DWS_ENABLE_VL53L0X` | `0` | Opt-in VL53L0X optical time-of-flight ranging sensor. |
| `DWS_ENABLE_VXI11` | `0` | VXI-11 (TCP/IP Instrument Protocol) codec (`services/vxi11`). |
| `DWS_ENABLE_WAL` | `0` | Opt-in write-ahead store for atomic buffer-to-flash storage. |
| `DWS_ENABLE_WAMP` | `0` | WAMP messaging codec (`services/wamp`). |
| `DWS_ENABLE_WAVE` | `0` | Opt-in IEEE 1609 WAVE (WSMP + 1609.2 envelope) codec. |
| `DWS_ENABLE_WEARLEVEL` | `0` | Opt-in flash wear-leveling slot selector. |
| `DWS_ENABLE_WEBDAV` | `0` | WebDAV server (RFC 4918, class 1 + advisory locks) over the file system. |
| `DWS_ENABLE_WEBHOOK` | `0` | Opt-in outbound webhooks / IFTTT. |
| `DWS_ENABLE_WEBSOCKET` | `1` | WebSocket support (RFC 6455 framing + SHA-1/base64 handshake). |
| `DWS_ENABLE_WEB_TERMINAL` | `0` | Browser "web serial" terminal over WebSocket (src/services/web_terminal). |
| `DWS_ENABLE_WIFI_SNIFFER` | `0` | Opt-in 802.11 sniffer / traffic analyzer. |
| `DWS_ENABLE_WISUN` | `0` | Opt-in Wi-SUN FAN border-router connector. |
| `DWS_ENABLE_WS_CLIENT` | `0` | Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS). |
| `DWS_ENABLE_WS_CLIENT_TLS` | `0` | wss://: run the WebSocket client over client-side TLS (needs DWS_ENABLE_TLS). |
| `DWS_ENABLE_WS_DEFLATE` | `0` | WebSocket permessage-deflate (RFC 7692) - bidirectional compression. |
| `DWS_ENABLE_XMPP` | `0` | Opt-in XMPP (RFC 6120) stanza codec. |
| `DWS_ENABLE_ZIGBEE` | `0` | Enable the Zigbee EZSP / ASH framing codec (default off). |
| `DWS_ENABLE_ZWAVE` | `0` | Enable the Z-Wave Serial API frame codec (default off). |

<!-- END GENERATED FEATURE-FLAGS -->

</details>

Illegal combinations (e.g. `MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS`) produce `#error` messages at compile time with a descriptive reason string.

## Configuration Overrides

All constants can be overridden using compiler build flags (e.g. `-DMAX_CONNS=6`). Default limits and sizes reside in [ServerConfig.h](@ref ServerConfig.h).

<details>
<summary><b>Expand Configuration constants and options</b></summary>

The full list of tunable `#define` constants and their defaults, scraped from
`src/ServerConfig.h` by `docs/utilities/gen_readme_sections.py`. Override any
with a build flag (e.g. `-DMAX_CONNS=6`); illegal combinations are caught by `#error`
guards at compile time.

<!-- BEGIN GENERATED CONFIG-OVERRIDES (docs/utilities/gen_readme_sections.py) -->

| Constant | Default | Description |
| :------- | :-----: | :---------- |
| `BODY_BUF_SIZE` | `256` | Maximum request body bytes stored in `HttpReq::body`. |
| `CACHE_CONTROL_BUF_SIZE` | `64` | Size of the optional Cache-Control header line stored in DWS. |
| `CHUNK_BUF_SIZE` | `1440` | Per-chunk staging buffer for send_chunked()'s ChunkSource (max bytes a source produces per call, hence the largest single chunk on the wire). |
| `CONN_TIMEOUT_MS` | `5000` | Compile-time default for connection idle timeout in milliseconds. |
| `CORS_HDR_BUF_SIZE` | `192` | Size of the pre-built CORS header block stored in DWS. |
| `DIGEST_AUTH_HDR_MAX` | `384` | Capacity for the full `Authorization` header value (Digest auth). |
| `DWS_ACCEPT_THROTTLE_MAX` | `20` | Max accepted connections per throttle window (see DWS_ENABLE_ACCEPT_THROTTLE). |
| `DWS_ACCEPT_THROTTLE_WINDOW_MS` | `1000` | Throttle window length in milliseconds (see DWS_ENABLE_ACCEPT_THROTTLE). |
| `DWS_ADS1115_DIFFERENTIAL` | `0` | ADS1115 input mode: 0 = single-ended (AINx vs GND), 1 = differential. |
| `DWS_ADS1115_I2C_ADDR` | `0x48` | I2C address of the ADS1115 (0x48 with ADDR to GND; 0x49/0x4A/0x4B for VDD/SDA/SCL). |
| `DWS_AUTH_LOCKOUT_BASE_MS` | `1000` | First lockout duration in ms; doubles on each further failure. |
| `DWS_AUTH_LOCKOUT_MAX_MS` | `300000` | Maximum lockout duration in ms (the exponential backoff cap). |
| `DWS_AUTH_LOCKOUT_SLOTS` | `16` | Number of source IPs the auth lockout tracks (BSS bucket table). |
| `DWS_AUTH_LOCKOUT_THRESHOLD` | `5` | Consecutive failed auths from one IP before it is locked out. |
| `DWS_BRIDGE_MAX_RULES` | `8` | Max concurrent address:port -> bus rules (services/iface_bridge). |
| `DWS_BRIDGE_STREAM_CHUNK` | `256` | STREAM (UART) pipe chunk size (bytes) for services/iface_bridge - one socket<->UART hop. |
| `DWS_BRIDGE_TXN_MAX` | `256` | Max write / read payload (bytes) per TRANSACTION frame (services/iface_bridge). |
| `DWS_BRIDGE_UART_TXN_MS` | `50` | UART TRANSACTION read window (ms): how long a write-then-read waits for the read_len reply. |
| `DWS_CLIENT_CONNS` | `2` | Number of simultaneous outbound client connections (BSS pool size). |
| `DWS_CLIENT_RX_BUF` | `8192` | Per-connection wire receive ring size (bytes). |
| `DWS_CLOSING_TIMEOUT_MS` | `2000` | Upper bound (ms) a slot may dwell in ConnState::CONN_CLOSING after a graceful close before the idle sweep force-aborts it. |
| `DWS_COAP_BLOCK1_MAX` | `1024` | Reassembly buffer for a block-wise (Block1) request upload, in bytes. |
| `DWS_COAP_BLOCK_SZX_MAX` | `6` | Largest block-size exponent (SZX) the server will use: block size = 2^(SZX+4) bytes, SZX 0..6 (16..1024). |
| `DWS_COAP_MAX_OBSERVERS` | `4` | Maximum simultaneous CoAP observers (one slot per observed resource per client). |
| `DWS_COAP_MAX_PATH` | `64` | Maximum reconstructed Uri-Path length, including separators and the leading '/'. |
| `DWS_COAP_MAX_PAYLOAD` | `256` | Maximum CoAP request/response payload in bytes. |
| `DWS_COAP_MAX_QUERY` | `64` | Maximum reconstructed Uri-Query length (segments joined by '&'). |
| `DWS_COAP_MAX_RESOURCES` | `8` | Maximum registered CoAP resources (the server's fixed routing table). |
| `DWS_COAP_OBSERVE_PORT` | `5683` | Default UDP port the CoAP observe transport notifies from (IANA well-known 5683). |
| `DWS_CONFIG_KEY_MAX` | `16` | Max key length incl. |
| `DWS_CONFIG_MAX_ENTRIES` | `16` | Max key/value entries in the host (test) config backend. |
| `DWS_CONFIG_VAL_MAX` | `64` | Max value bytes per entry in the host (test) config backend. |
| `DWS_DASHBOARD_JSON_BUF` | `1024` | Stack buffer for the dashboard layout / values JSON (bytes). |
| `DWS_DASHBOARD_MAX_WIDGETS` | `16` | Maximum widgets in the dashboard table (BSS value array). |
| `DWS_DEFER_QUEUE_DEPTH` | `8` | Depth of each worker's deferred-callback queue. |
| `DWS_DELIVERY_MANIFEST_BUF` | `512` | Buffer the precache manifest JSON is built into. |
| `DWS_DELIVERY_PRECACHE_MAX` | `16` | Most asset paths a service-worker precache manifest may list. |
| `DWS_DMA_BUF_SIZE` | `256` | Bytes per DMA transfer buffer (RX is double-buffered at this size). |
| `DWS_DMA_CHANNELS` | `2` | Number of DMA channels (static-allocated; each is one peripheral link). |
| `DWS_DMA_SIMULATE` | `1` | Route DMA transfers through the ingress/egress simulator (default on). |
| `DWS_DNC_LEADER_LEN` | `32` | Default leader/trailer runout length for the DNC encoder. |
| `DWS_DNC_LINE_MAX` | `128` | Largest G-code block (one line) the DNC decoder reassembles. |
| `DWS_DNC_XOFF_MAX_POLLS` | `200000` | Safety cap on how many times the DNC stream engine polls the reverse channel while paused by an XOFF, before giving up with an I/O error. |
| `DWS_DNS_NAME_MAX` | `128` | Max length of a queried/stored DNS name (bytes, incl NUL). |
| `DWS_DNS_SERVER_MAX_RECORDS` | `8` | Max A records in the DNS server's fixed table. |
| `DWS_DNS_SERVER_TTL` | `60` | TTL (seconds) the DNS server puts on its answers. |
| `DWS_DNS_TIMEOUT_MS` | `5000` | DNS resolve timeout in milliseconds. |
| `DWS_ENFORCE_HOST_HEADER` | `1` | Enforce the RFC 7230 §5.4 Host-header requirement (default on). |
| `DWS_ENOCEAN_MAX_DATA` | `512` | Reject an ESP3 telegram whose declared data length exceeds this (framing sanity). |
| `DWS_ETH_W5500` | `0` |  |
| `DWS_EXC_COREDUMP_CHUNK` | `512` | Chunk the core-dump image is streamed out of flash in. |
| `DWS_FAILSAFE_MAX_LIFELINES` | `8` | Max monitored lifelines in the fail-safe registry (static, zero-heap). |
| `DWS_FTP_CHUNK` | `512` | Bytes staged per data-channel write when the session driver streams a payload. |
| `DWS_FTP_CMD_MAX` | `256` | Suggested FTP control-command buffer size. |
| `DWS_FTP_REPLY_BUF` | `512` | Control-reply accumulator for the FTP session driver. |
| `DWS_FTP_TIMEOUT_MS` | `8000` | Per-step timeout for the FTP session driver: connect, and each control reply. |
| `DWS_FWD_ACL_PATLEN` | `4` | Bytes an ACL entry can match (its pattern / mask length). |
| `DWS_FWD_INSPECT` | `0` | Build-time toggle for the forwarding-path inspection hook (default off, for cost + privacy). |
| `DWS_FWD_MAX_ACL` | `8` | Max ingress access-control entries (byte-pattern permit/deny; static). |
| `DWS_FWD_MAX_IFACES` | `4` | Max interfaces the forwarding plane tracks (static-allocated). |
| `DWS_FWD_MAX_ROUTES` | `8` | Max policy routes (byte-pattern -> egress interface; static). |
| `DWS_FWD_MAX_RULES` | `8` | Max forwarding rules (src -> dst allow/deny + rate cap; static-allocated). |
| `DWS_GPIO_JSON_BUF` | `1024` | Stack buffer for the GPIO-map JSON (bytes). |
| `DWS_GPIO_MAX` | `40` | Maximum GPIO pins the mapper reports (BSS table). |
| `DWS_GUARDRAIL_FRAG_MIN_BLOCK` | `4096` | Largest-free-block floor (bytes); below this trips the fragmentation guardrail. |
| `DWS_GUARDRAIL_HEAP_MIN` | `8192` | Free-heap floor (bytes); below this trips the heap guardrail. |
| `DWS_GUARDRAIL_STACK_MIN` | `512` | Task remaining-stack floor (bytes); below this trips the stack guardrail. |
| `DWS_GW_MAX_PORTS` | `4` | Max southbound gateway ports (radios / buses; static-allocated). |
| `DWS_H2_HDR_BLOCK` | `4096` | Header-block reassembly buffer for HTTP/2 requests that span HEADERS + CONTINUATION frames (a single END_HEADERS frame decodes in place and needs no copy). |
| `DWS_H2_MAX_FRAME` | `16384` | Largest HTTP/2 frame we accept, in bytes (advertised as SETTINGS_MAX_FRAME_SIZE). |
| `DWS_H2_MAX_STREAMS` | `8` | Max concurrent HTTP/2 streams per connection (advertised as MAX_CONCURRENT_STREAMS). |
| `DWS_H2_POOL_IN_PSRAM` | `0` | Place the HTTP/2 connection-engine pool in external PSRAM (ESP32). |
| `DWS_H3_CRYPTO_BUF` | `2048` | Maximum bytes of one QUIC/TLS handshake CRYPTO flight (RFC 9001). |
| `DWS_H3_MAX_STREAMS` | `8` | Maximum concurrent request streams per HTTP/3 connection. |
| `DWS_HOTSWAP_FAIL_THRESHOLD` | `3` | Consecutive I/O failures that declare a removable volume gone. |
| `DWS_HOTSWAP_PROBE_MS` | `2000` | Minimum gap between remount attempts while a volume is absent or faulted (ms). |
| `DWS_HPACK_MAX_ENTRIES` | `128` | Max HPACK dynamic-table entries (>= DWS_HPACK_TABLE_BYTES / 32, the min entry size). |
| `DWS_HPACK_TABLE_BYTES` | `4096` | Per-connection HPACK dynamic-table size in bytes (our decoder; advertised to the peer as SETTINGS_HEADER_TABLE_SIZE). |
| `DWS_HTTP3_PORT` | `443` | UDP port the HTTP/3 (QUIC) server binds by default (used by DWS::dws_h3_cert). |
| `DWS_HTTP_CLIENT_BUF_SIZE` | `2048` | Receive buffer (and max response size) for the outbound HTTP client, bytes. |
| `DWS_HTTP_CLIENT_CT_BUF_SIZE` | `4096` | Ciphertext receive-ring size for the https:// client, bytes. |
| `DWS_HTTP_CLIENT_TIMEOUT_MS` | `8000` | Outbound HTTP client connect/response timeout in milliseconds. |
| `DWS_HTTP_EMIT_DATE` | `0` | Auto-inject a `Date` response header (RFC 7231 7.1.1.2) when a wall-clock time is available. |
| `DWS_INA219_CURRENT_LSB_UA` | `100` | Default INA219 current LSB in microamps per bit (calibration input). |
| `DWS_INA219_I2C_ADDR` | `0x40` | I2C address of the INA219 (0x40 default; the A0/A1 pins select 0x40..0x4F). |
| `DWS_INA219_SHUNT_MOHM` | `100` | Default INA219 shunt resistance in milliohms (calibration input). |
| `DWS_IP_ALLOWLIST_SLOTS` | `8` | Number of CIDR rules the source-IP allowlist can hold (BSS table). |
| `DWS_JWT_MAX_LEN` | `512` | Maximum accepted JWT length in bytes (header.payload.signature). |
| `DWS_KEEPALIVE_MAX_REQUESTS` | `100` | Maximum requests served on one keep-alive connection before it is closed. |
| `DWS_LD2410_BAUD` | `256000` | LD2410 UART baud rate (the module's fixed factory default is 256000). |
| `DWS_LOG_LINES` | `32` | Number of log lines retained in the ring. |
| `DWS_LOG_LINE_LEN` | `96` | Maximum length of one stored log line (bytes, including null). |
| `DWS_LORA_MAX_PAYLOAD` | `251` | Max LoRa payload bytes (SX127x FIFO is 256; RadioHead uses 251 + 4 header). |
| `DWS_MAX_UDP_LISTENERS` | `2` | Maximum simultaneously bound UDP ports (transport-layer UDP service). |
| `DWS_MODBUS_COILS` | `64` | Number of Modbus coils (FC 1/5/15), single-bit R/W (BSS, bit-packed). |
| `DWS_MODBUS_DISCRETE_INPUTS` | `64` | Number of Modbus discrete inputs (FC 2), single-bit read-only (BSS, bit-packed). |
| `DWS_MODBUS_HOLDING_REGS` | `64` | Number of Modbus holding registers (FC 3/6/16), 16-bit R/W (BSS). |
| `DWS_MODBUS_INPUT_REGS` | `64` | Number of Modbus input registers (FC 4), 16-bit read-only (BSS). |
| `DWS_MPR121_I2C_ADDR` | `0x5A` | I2C address of the MPR121 (0x5A default; 0x5B/0x5C/0x5D via the ADDR pin). |
| `DWS_MPR121_RELEASE_THRESHOLD` | `6` | MPR121 per-electrode release threshold (delta counts; should be below the touch threshold). |
| `DWS_MPR121_TOUCH_THRESHOLD` | `12` | MPR121 per-electrode touch threshold (delta counts from baseline; NXP AN3944 suggests ~4..12). |
| `DWS_MQTT_BUF_SIZE` | `1024` | MQTT packet buffer size in bytes (bounds one outgoing/incoming packet). |
| `DWS_MQTT_CT_BUF_SIZE` | `4096` | Ciphertext receive-ring size for MQTTS (draining ring; must exceed one TCP_MSS). |
| `DWS_MQTT_INFLIGHT_BUF` | `256` | Stored-packet size per in-flight QoS 1/2 slot (caps a retransmittable PUBLISH). |
| `DWS_MQTT_KEEPALIVE_S` | `30` | Default MQTT keep-alive interval in seconds (PINGREQ cadence / CONNECT field). |
| `DWS_MQTT_MAX_INFLIGHT` | `4` | Outbound QoS 1/2 in-flight slots (unacknowledged messages held for DUP retransmit). |
| `DWS_MQTT_MAX_TOPIC` | `128` | Maximum inbound MQTT topic length (including NUL) delivered to the callback. |
| `DWS_MQTT_RETRANSMIT_MS` | `5000` | Retransmit timeout (ms) for an unacknowledged in-flight QoS 1/2 message. |
| `DWS_MQTT_RX_QOS2_SLOTS` | `8` | Inbound QoS 2 packet-id de-duplication ring depth (PUBREC-acknowledged, awaiting PUBREL). |
| `DWS_MTLS_SUBJECT_MAX` | `128` | Maximum length of a verified mTLS peer subject DN string (incl. |
| `DWS_NEED_DET_CLIENT` | `0` |  |
| `DWS_NRF24_PAYLOAD` | `32` | nRF24 fixed payload width in bytes (1..32; the chip's static payload size). |
| `DWS_NTP_SERVER_STRATUM` | `3` | Stratum the NTP server advertises (distance from a reference clock; 1-15). |
| `DWS_NTRIP_MAX_MOUNTS` | `2` | Max distinct mountpoints a single caster serves (each = one RTCM stream). |
| `DWS_NTRIP_MAX_ROVERS` | `4` | Max concurrent rover connections a caster serves corrections to (services/gnss). |
| `DWS_NTRIP_MOUNT_MAX` | `32` | Max length (incl. |
| `DWS_NTRIP_REQ_MAX` | `512` | Max NTRIP client request size (bytes) the caster buffers while reading the request headers. |
| `DWS_OIDC_MAX_LEN` | `1600` | Max accepted OIDC ID-token length (also sizes the Authorization buffer). |
| `DWS_OTA_CONFIRM_WINDOW_MS` | `30000` | Confirm window (ms): a pending image not confirmed within this rolls back. |
| `DWS_PARTITION_JSON_BUF` | `1024` | Stack buffer for the partition-map JSON (bytes). |
| `DWS_PARTITION_MAX` | `16` | Maximum partitions the monitor reports (BSS table). |
| `DWS_PCA9685_FREQ` | `50` | Default PWM output frequency in Hz (50 Hz suits hobby servos). |
| `DWS_PCA9685_I2C_ADDR` | `0x40` | I2C address of the PCA9685 (0x40 default; the six address pins select 0x40..0x7F). |
| `DWS_PER_IP_THROTTLE_MAX` | `10` | Max accepted connections per window from one source IP (see DWS_ENABLE_PER_IP_THROTTLE). |
| `DWS_PER_IP_THROTTLE_SLOTS` | `16` | Number of source IPv4 addresses tracked by the per-IP throttle (BSS bucket table). |
| `DWS_PER_IP_THROTTLE_WINDOW_MS` | `10000` | Per-IP throttle window length in milliseconds (see DWS_ENABLE_PER_IP_THROTTLE). |
| `DWS_PN532_MAX_DATA` | `254` | Reject a PN532 normal frame whose declared length exceeds this (framing sanity). |
| `DWS_POWER_BUSY_PCT` | `40` | Load percentage at/above which the ceiling clock is used. |
| `DWS_POWER_MHZ_MAX` | `240` | CPU clock (MHz) when there is work to do. |
| `DWS_POWER_MHZ_MIN` | `80` | CPU clock (MHz) when idle, thermally throttled, or recovering from a brownout. |
| `DWS_POWER_RECOVER_MS` | `10000` | How long (ms) to hold the floor clock after a brownout reset before ramping back up. |
| `DWS_POWER_TEMP_COOL_C` | `70` | Die temperature (C) at/below which the throttle is released. |
| `DWS_POWER_TEMP_HOT_C` | `80` | Die temperature (C) at/above which the clock is throttled. |
| `DWS_PQ_DEPTH` | `16` | Capacity of the preempting queue in items (static-allocated). |
| `DWS_PQ_INTERNAL_PRIORITY` | `8` | Base FreeRTOS priority for the internal preempting lanes (DMA / forwarding / device access). |
| `DWS_PQ_ITEM_SIZE` | `32` | Bytes per preempting-queue item (the posted item must fit). |
| `DWS_PQ_STACK` | `4096` | Stack (bytes) for each preempting-queue processing task (ESP32). |
| `DWS_PROTO_MAX` | `11` | Size of the protocol-handler dispatch table; must exceed the largest ConnProto id. |
| `DWS_RADIO_MAX_TX_DBM` | `0` | Max TX power cap in dBm (2..20); 0 = leave the platform default. |
| `DWS_RADIO_WIFI_PS` | `0` | WiFi modem-sleep mode: 0 = none (max perf), 1 = min modem, 2 = max modem. |
| `DWS_RELAY_BUF` | `2048` | Per-direction relay buffer size (bytes) for services/relay. |
| `DWS_RELAY_CONNECT_MS` | `5000` | Blocking connect timeout (ms) when the relay listener dials the origin on a new inbound. |
| `DWS_RELAY_DRAIN_MAX` | `8` | Max dws_relay_step passes per poll for the relay listener. |
| `DWS_RELAY_HOST_MAX` | `64` | Max origin hostname length (bytes, incl. |
| `DWS_RELAY_MAX_CONNS` | `4` | Max concurrent relayed connections (bridge table size) for the relay listener. |
| `DWS_RELAY_MAX_PUBLISH` | `4` | Max published relay ports (bind table size) for the relay listener. |
| `DWS_RTC_I2C_ADDR` | `0x68` | I2C address of the RTC (DS1307/DS3231 are fixed at 0x68). |
| `DWS_SCPI_ERR_QUEUE` | `8` | SCPI error/event queue depth (entries). |
| `DWS_SCRATCH_ARENA_SIZE` | `8192` | Size in bytes of the shared per-dispatch scratch arena. |
| `DWS_SEN0192_ACTIVE_HIGH` | `1` | SEN0192 OUT polarity: 1 = the OUT line reads HIGH on motion, 0 = active-LOW. |
| `DWS_SEN0192_HOLD_MS` | `2000` | Presence is held this many ms after the last active (motion) sample before it clears. |
| `DWS_SEN0192_PIN` | `4` | GPIO the SEN0192 OUT line is wired to. |
| `DWS_SFTP_MAX_HANDLES` | `4` | Max concurrent open SFTP handles (files + dirs) per SSH connection. |
| `DWS_SFTP_MAX_READ` | `1024` | Largest SSH_FXP_DATA payload returned for one READ (a short read - the client re-requests). |
| `DWS_SFTP_PATH_MAX` | `256` | Largest absolute path the SFTP/SCP server resolves (mount root + request path). |
| `DWS_SFTP_PKT_BUF` | `2048` | SFTP packet-assembly buffer per SFTP channel (bytes); bounds one non-streamed request/response. |
| `DWS_SHT3X_I2C_ADDR` | `0x44` | I2C address of the SHT3x (0x44 with ADDR low; 0x45 with ADDR high). |
| `DWS_SIGFOX_MAX_PAYLOAD` | `12` | Maximum Sigfox uplink payload (the network caps a message at 12 bytes). |
| `DWS_SMB_BUF` | `1024` | SMB2 client work-buffer size (bytes) for smb_client's request/response framing. |
| `DWS_SMTP_CT_BUF_SIZE` | `4096` | Ciphertext receive-ring size for SMTPS, bytes (only used when the message is TLS). |
| `DWS_SMTP_LINE_MAX` | `256` | Max length of one SMTP command / address line (bytes, incl. |
| `DWS_SMTP_MSG_MAX` | `2048` | Max size of the assembled DATA payload (headers + dot-stuffed body), bytes. |
| `DWS_SMTP_REPLY_MAX` | `512` | Max size of one (possibly multi-line) server reply held while parsing, bytes. |
| `DWS_SMTP_TIMEOUT_MS` | `10000` | SMTP connect / per-reply timeout in milliseconds. |
| `DWS_SNMP_TRAP_BUF_SIZE` | `1024` | Static datagram buffer for an outbound SNMP notification, bytes. |
| `DWS_SNMP_TRAP_MAX_VARBINDS` | `8` | Maximum extra variable-bindings (beyond sysUpTime/snmpTrapOID) in one notification. |
| `DWS_SPB_METRIC_MAX` | `256` | Max serialized size of one Sparkplug B metric submessage (stack temp, bytes). |
| `DWS_SSH_ALLOW_PASSWORD` | `1` | Allow SSH password authentication (default on). |
| `DWS_SSH_FWD_CHUNK` | `1024` | Max bytes moved per forward channel per poll, target -> client (<= SSH_PKT_BUF_SIZE). |
| `DWS_SSH_FWD_CONNECT_MS` | `3000` | Blocking connect timeout (ms) when opening a forward target. |
| `DWS_SSH_FWD_HOST_MAX` | `64` | Maximum forward target hostname length including null terminator. |
| `DWS_SSH_FWD_MAX` | `2` | Maximum concurrent forwarded TCP connections (must be <= DWS_CLIENT_CONNS). |
| `DWS_SSH_MAX_CHANNELS` | `1` | Maximum concurrent SSH channels per connection (RFC 4254 multiplexing). |
| `DWS_SSH_PORT_FORWARD` | `0` | SSH TCP port forwarding (`direct-tcpip`, i.e. |
| `DWS_SSH_RFWD_BRIDGE_MAX` | `2` | Maximum concurrent bridged connections across all remote forwards. |
| `DWS_SSH_RFWD_MAX` | `1` | Maximum concurrent remote-forward listeners (`ssh -R` / `tcpip-forward`). |
| `DWS_SSH_ZLIB_ACK_DRAM` | `0` | Acknowledge placing the SSH compressor in internal DRAM (no PSRAM). |
| `DWS_SSH_ZLIB_IN_PSRAM` | `0` | Place the per-connection SSH compression state in external PSRAM (ESP32). |
| `DWS_SSH_ZLIB_MAX_IN` | `2048` | Largest uncompressed payload the s2c compressor accepts in one call (bytes). |
| `DWS_SSH_ZLIB_WINDOW` | `8192` | SSH s2c DEFLATE sliding-window size in bytes (max back-reference distance). |
| `DWS_STATSD_LINE_MAX` | `256` | Stack buffer for one StatsD line (bytes; caps metric name + value + tags). |
| `DWS_STATSD_PORT` | `8125` | Default StatsD collector UDP port (StatsD/Graphite standard). |
| `DWS_STOMP_MAX_HEADERS` | `16` | Max header lines parsed per STOMP frame (extras beyond this are ignored). |
| `DWS_SYSLOG_DEFAULT_PORT` | `514` | Default syslog collector UDP port (RFC 5426 well-known 514; overridable at runtime via dws_syslog_init and here for a non-standard collector). |
| `DWS_SYSLOG_FIELD_MAX` | `32` | Maximum syslog HOSTNAME / APP-NAME field length (including NUL). |
| `DWS_SYSLOG_MSG_MAX` | `256` | Maximum formatted syslog datagram length in bytes (RFC 5424 line). |
| `DWS_TCP_NODELAY` | `1` | Disable Nagle's algorithm (set TCP_NODELAY) on every accepted connection. |
| `DWS_THEMES_INCLUDE_TRADEMARKED` | `1` | Include the trademark-named themes in the embedded set (default on / open-source). |
| `DWS_THREAD_MAX_DATA` | `256` | Max spinel payload bytes carried in one HDLC-lite frame. |
| `DWS_TIME_SOURCE_MAX` | `4` | Maximum registered time sources. |
| `DWS_TLS_ACK_MULTI_CONN_DRAM` | `0` | Acknowledge that a MAX_TLS_CONNS > 1 build has been sized to fit. |
| `DWS_TLS_ARENA_IN_PSRAM` | `0` | Place the TLS arena in external PSRAM instead of internal DRAM (ESP32). |
| `DWS_TLS_ARENA_SIZE` | `49152` | Bytes of the static BSS arena mbedTLS allocates from. |
| `DWS_TLS_MAX_FRAG_LEN` | `0` | Cap TLS records via the Maximum Fragment Length extension (RFC 6066). |
| `DWS_TLS_TICKET_LIFETIME_S` | `86400` | Session-ticket lifetime / key-rotation period in seconds (see DWS_ENABLE_TLS_RESUMPTION). |
| `DWS_TRUSTED_PROXY_MAX` | `2` | Number of trusted-upstream CIDR rules the forwarded-client resolver holds (BSS table). |
| `DWS_UDP_RX_BUF_SIZE` | `1472` | Shared receive-scratch size for the transport-layer UDP service. |
| `DWS_UDP_TELEMETRY_BUF` | `256` | Stack buffer for one telemetry line (bytes). |
| `DWS_UMATI_NS` | `1` | NamespaceIndex the umati MachineTool nodes live at (default 1). |
| `DWS_WEBDAV_BUF_SIZE` | `2048` | Buffer (BSS) for a WebDAV 207 Multi-Status response, in bytes (see DWS_ENABLE_WEBDAV). |
| `DWS_WEBDAV_MAX_ENTRIES` | `32` | Maximum children listed in a WebDAV Depth-1 PROPFIND (bounds the response). |
| `DWS_WEBDAV_MAX_PROPS` | `16` | Maximum properties echoed in a WebDAV PROPPATCH 207 response (bounds the response). |
| `DWS_WIFI_SNIFFER_MAX_CHANNELS` | `14` | Channels tracked by the WiFi sniffer's per-channel survey. |
| `DWS_WORKER_CORE` | `1` | Core that worker 0 pins to (ESP32). |
| `DWS_WORKER_COUNT` | `1` | Number of server worker tasks (slots partitioned i % N). |
| `DWS_WORKER_POLL_TICKS` | `1` | Idle-sweep timeout, in FreeRTOS ticks, that a worker blocks between service iterations when no events are pending. |
| `DWS_WORKER_STACK_CURVE_MIN` | `12288` | Minimum worker-task stack (bytes) required once SSH is compiled in. |
| `DWS_WORKER_STACK_PQC_MIN` | `16384` |  |
| `DWS_WORKER_STACK_RSA_MIN` | `8192` | Minimum worker-task stack (bytes) required once an RSA-2048 verifier is compiled in (OIDC / SSH). |
| `DWS_WORKER_TASK_PRIORITY` | `5` | FreeRTOS priority for each server worker task (ESP32). |
| `DWS_WS_CLIENT_BUF_SIZE` | `1024` | WebSocket client send/receive buffer size in bytes (bounds one frame). |
| `DWS_WS_CLIENT_CT_BUF_SIZE` | `4096` | Ciphertext receive-ring size for wss:// (draining ring; must exceed one TCP_MSS). |
| `DWS_WS_FRAG_SIZE` | `0` | WebSocket outbound fragmentation size (RFC 6455 sec 5.4), in payload bytes. |
| `DWS_ZIGBEE_MAX_DATA` | `128` | Max ASH payload bytes (an EZSP frame; the ASH data field caps near 128). |
| `DWS_ZWAVE_MAX_DATA` | `64` | Reject a Z-Wave frame whose declared length exceeds this data cap (sanity). |
| `EXTRA_HDR_BUF_SIZE` | `256` | Per-connection buffer for app-supplied custom response headers and cookies. |
| `FILE_CHUNK_SIZE` | `1024` | Bytes read from the filesystem and passed to tcp_write() per loop(). |
| `JSON_MAX_DEPTH` | `8` | Maximum object/array nesting depth for the JsonWriter (see json.h). |
| `MAX_AUTH_LEN` | `32` | Maximum username or password length for HTTP Basic Authentication. |
| `MAX_BOUNDARY_LEN` | `72` | Maximum MIME boundary length (RFC 2046 allows up to 70 characters). |
| `MAX_CONNS` | `8` | Maximum simultaneous TCP connections (fixed static pool; ~3.95 KB of internal RAM per slot). |
| `MAX_HEADERS` | `8` | Maximum HTTP headers stored per request. |
| `MAX_KEY_LEN` | `32` | Maximum header field-name length (e.g. |
| `MAX_LISTENERS` | `3` | Maximum number of simultaneously active listener ports. |
| `MAX_MIDDLEWARE` | `4` | Maximum globally-registered middleware functions. |
| `MAX_MULTIPART_PARTS` | `4` | Maximum simultaneously parsed multipart parts per request. |
| `MAX_PATH_LEN` | `64` | Maximum URL path length (including leading `/`). |
| `MAX_PATH_PARAMS` | `4` | Maximum number of `:name` path parameters captured per route match. |
| `MAX_QUERY_LEN` | `128` | Maximum raw query-string length (everything after `?`). |
| `MAX_QUERY_PARAMS` | `8` | Maximum number of parsed query-string parameters. |
| `MAX_ROUTES` | `16` | Maximum simultaneously registered routes. |
| `MAX_SSE_CONNS` | `2` | Maximum simultaneous SSE connections. |
| `MAX_SSH_CONNS` | `1` | Maximum simultaneous SSH connections. |
| `MAX_TELNET_CONNS` | `2` | Maximum simultaneous Telnet connections. |
| `MAX_TLS_CONNS` | `1` | Maximum simultaneous TLS connections (each holds mbedTLS record buffers). |
| `MAX_VAL_LEN` | `48` | Maximum header field-value length. |
| `MAX_WS_CONNS` | `2` | Maximum simultaneous WebSocket connections. |
| `QUERY_KEY_LEN` | `24` | Maximum query-parameter key length. |
| `QUERY_VAL_LEN` | `48` | Maximum query-parameter value length. |
| `RESP_HDR_BUF_SIZE` | `768` | Stack buffer for HTTP response header lines in send() / send_empty() / send_unauth() / serve_file(). |
| `RE_MAX_STEPS` | `2000` | Step budget for the regex route matcher (see on_regex()). |
| `RX_BUF_SIZE` | `1024` | Ring-buffer capacity in bytes per connection slot. |
| `SNMP_COMMUNITY_MAX` | `32` | Maximum SNMP community-string length (including null terminator). |
| `SNMP_MAX_MIB_ENTRIES` | `16` | Maximum registered MIB objects (the agent's fixed OID table). |
| `SNMP_MAX_OID_LEN` | `32` | Maximum sub-identifiers (arcs) in an SNMP object identifier. |
| `SNMP_MAX_VARBINDS` | `16` | Maximum variable bindings the agent will emit in one response. |
| `SNMP_MSG_BUF_SIZE` | `1472` | Static request/response datagram buffers for the SNMP UDP agent. |
| `SNMP_V3_ENGINEID_MAX` | `32` | Maximum SNMPv3 authoritative engine-ID length in bytes (RFC 3411 allows 5..32). |
| `SNMP_V3_USER_MAX` | `32` | Maximum SNMPv3 USM user-name length (including null terminator). |
| `SSE_BUF_SIZE` | `256` | Output buffer size in bytes for a single SSE event. |
| `SSH_AUTH_PASS_MAX` | `64` | Max stored password length. |
| `SSH_AUTH_USER_MAX` | `32` | Max stored user name (RFC 4252 imposes no limit; we cap for BSS). |
| `SSH_CHAN_WINDOW` | `32768u` | Initial receive window the SSH server advertises (RFC 4254 §5.1). |
| `SSH_CRYPTO_WORK_SIZE` | `1536` | Shared scratch buffer for SSH big-number operations. |
| `SSH_KEXINIT_MAX` | `2048` | Max stored size of the CLIENT KEXINIT payload (I_C, for the exchange hash). |
| `SSH_MAX_AUTH_ATTEMPTS` | `6` | Maximum failed SSH authentication attempts per connection. |
| `SSH_MAX_PASSWORD_LEN` | `64` | Maximum SSH password length including null terminator. |
| `SSH_MAX_USERNAME_LEN` | `32` | Maximum SSH username length including null terminator. |
| `SSH_PKT_BUF_SIZE` | `2048` | Packet assembly buffer per SSH connection (bytes). |
| `SSH_REKEY_PACKET_THRESHOLD` | `0x40000000u` | Re-key when either packet sequence number reaches this value. |
| `SSH_REKEY_TIME_MS` | `3600000u` | Elapsed-time re-key trigger in milliseconds (RFC 4253 §9: "after each hour"). |
| `TELNET_BUF_SIZE` | `256` | Stack buffer for one Telnet I/O chunk. |
| `TERM_TX_BUF_SIZE` | `256` | Stack scratch for dws_web_terminal_printf()/println() formatting. |
| `WS_FRAME_SIZE` | `512` | Maximum WebSocket frame payload in bytes. |
| `WS_HDR_BUF_SIZE` | `256` | Stack buffer for the HTTP 101 Switching Protocols response sent during the WebSocket handshake. |

<!-- END GENERATED CONFIG-OVERRIDES -->

**Runtime Config**

The connection idle timeout can be changed without a rebuild:

```cpp
const WebServerConfig cfg PROGMEM = { .conn_timeout_ms = 10000 }; // flash, no RAM cost
server.begin(80, &cfg);
```

Pass `nullptr` (or omit) to use the compile-time default [`CONN_TIMEOUT_MS`](@ref CONN_TIMEOUT_MS) (5000 ms).

</details>

## API Reference

<details>
<summary><b>Expand API Reference</b></summary>

**DetWebServer - Lifecycle**

| Method                                  | Description                                                                     |
| --------------------------------------- | ------------------------------------------------------------------------------- |
| `begin(port, cfg = nullptr)`            | Bind and listen. Returns `DETWS_OK` (1) on success, a negative error code on failure. |
| [`stop()`](@ref DetWebServer::stop)     | Abort all connections, close listener, reset all pools.                         |
| `restart(cfg = nullptr)`                | `stop()` + `begin()` on the same port. Returns `-1` if called before `begin()`. |
| [`handle()`](@ref DetWebServer::handle) | Call every `loop()`. Runs timeout sweep, event drain, and dispatch.             |

**DetWebServer - HTTP Routes**

| Method                                         | Description                                                     |
| ---------------------------------------------- | --------------------------------------------------------------- |
| `on(path, method, handler)`                    | Register a route. Trailing `*` enables prefix matching.         |
| `on(path, method, handler, realm, user, pass)` | Same, with Basic Auth (`DETWS_ENABLE_AUTH`).                    |
| `on_not_found(handler)`                        | Fallback handler; default sends 404.                            |
| `set_cors(origin)`                             | Enable CORS and answer OPTIONS with 204. Pass `""` to disable.  |
| `send(slot_id, code, type, body)`              | Send a response with body and close the connection.             |
| `send_empty(slot_id, code)`                    | Send a headers-only response and close the connection.          |
| `serve_file(slot_id, fs, path, type)`          | Stream a file from an Arduino FS (`DETWS_ENABLE_FILE_SERVING`). |

**DetWebServer - WebSocket (DETWS_ENABLE_WEBSOCKET)**

| Method                                          | Description                                 |
| ----------------------------------------------- | ------------------------------------------- |
| `on_ws(path, on_connect, on_message, on_close)` | Register a WebSocket route.                 |
| `ws_send_text(ws_id, text)`                     | Send a UTF-8 text frame to a client.        |
| `ws_send_binary(ws_id, data, len)`              | Send a binary frame to a client.            |
| `ws_disconnect(ws_id)`                          | Send Close frame and mark slot for cleanup. |

In `on_message`, read the received payload from `ws_pool[ws_id].buf` (length in `ws_pool[ws_id].payload_len`).

**DetWebServer - SSE (DETWS_ENABLE_SSE)**

| Method                                                     | Description                             |
| ---------------------------------------------------------- | --------------------------------------- |
| `on_sse(path, on_connect)`                                 | Register an SSE route.                  |
| `sse_send(sse_id, data, event = nullptr, id = nullptr)`    | Push an event to one client.            |
| `sse_broadcast(path, data, event = nullptr, id = nullptr)` | Push an event to all clients on a path. |

**DetWebServer - Diagnostic (DETWS_ENABLE_DIAG)**

| Method          | Description                                                                                          |
| --------------- | ---------------------------------------------------------------------------------------------------- |
| `diag(slot_id)` | Send a JSON object with all active feature flags and configuration constants. Disable in production. |

**Handler Signatures**

```cpp
// HTTP
void handler(uint8_t slot_id, HttpReq *req);

// WebSocket (DETWS_ENABLE_WEBSOCKET)
void ws_connect(uint8_t ws_id);
void ws_message(uint8_t ws_id);  // payload in ws_pool[ws_id].buf
void ws_close(uint8_t ws_id);

// SSE (DETWS_ENABLE_SSE)
void sse_connect(uint8_t sse_id);
```

**HttpReq Fields**

| Field            | Type                              | Description                                                                                  |
| ---------------- | --------------------------------- | -------------------------------------------------------------------------------------------- |
| `method`         | `char[8]`                         | HTTP method string, e.g. `"GET"`                                                             |
| `path`           | `char[MAX_PATH_LEN]`              | URL path, e.g. `"/api/status"`                                                               |
| `version`        | [`HttpVersion`](@ref HttpVersion) | [`HTTP_10`](@ref HTTP_10), [`HTTP_11`](@ref HTTP_11), or [`HTTP_UNKNOWN`](@ref HTTP_UNKNOWN) |
| `query`          | `char[MAX_QUERY_LEN]`             | Raw query string (everything after `?`)                                                      |
| `query_params`   | `QueryParam[MAX_QUERY_PARAMS]`    | Parsed key=value pairs                                                                       |
| `query_count`    | `uint8_t`                         | Valid entries in `query_params[]`                                                            |
| `headers`        | `Header[MAX_HEADERS]`             | Captured header fields                                                                       |
| `header_count`   | `uint8_t`                         | Valid entries in `headers[]`                                                                 |
| `content_length` | `size_t`                          | Value of `Content-Length` header (0 if absent)                                               |
| `body`           | `uint8_t[BODY_BUF_SIZE+1]`        | Request body, always null-terminated                                                         |
| `body_len`       | `size_t`                          | Bytes stored in `body[]`                                                                     |

**Helper Functions**

```cpp
const char *http_get_header(const HttpReq *req, const char *key); // case-insensitive
const char *http_get_query (const HttpReq *req, const char *key); // case-sensitive
```

</details>

## RFC Compliance

The core HTTP/1.1 parser enforces RFC 7230 byte-by-byte; the dispatcher returns the
correct status codes (400/404/405/413/414/426/501) with `Allow` / `Sec-WebSocket-Version`
headers where required; the WebSocket layer enforces RFC 6455 framing. HTTP/2 (RFC 9113 +
HPACK RFC 7541) and the HTTP/3 stack (RFC 9114 over QUIC, RFC 9000) follow
their own specs, and every optional protocol is implemented against its authoritative
standard.

See **[RFC.md](RFC.md)** for the HTTP / WebSocket / error-response conformance tables and
**[STANDARDS.md](STANDARDS.md)** for the complete per-protocol standards map.

## SSH Support

DeterministicESPAsyncWebServer includes a **complete SSH-2.0 server** -
banner exchange → `KEXINIT` negotiation → key exchange → `NEWKEYS` → user
authentication (**publickey** and password) → `ssh-connection` session channel,
with per-direction NEWKEYS and transparent in-session re-keys. Key exchange offers
Curve25519 ECDH (`curve25519-sha256`) and `diffie-hellman-group14-sha256`; host keys
are Ed25519 (`ssh-ed25519`) and RSA (`rsa-sha2-256` / `ssh-rsa`). All state is static
(BSS), the host private key never touches static scratch memory, and password auth can
be compiled out (`DETWS_SSH_ALLOW_PASSWORD=0`) for publickey-only hardening.

See **[SSH.md](SSH.md)** for the feature summary, RFC/FIPS compliance
table, authentication/hardening details, and memory footprint, and
**[SECURITY.md](SECURITY.md)** for the security treatment.

## Utility Tools

Python tooling for generating documentation and building the embedded web assets. The
documentation generators run in CI (the Feature Tables workflow) so their output never
drifts; run any of them locally from the repo root.

<details>
<summary><b>Expand Utility Tools and Scripts Guide</b></summary>

**Documentation generators** (`docs/utilities/`)

| Script                   | Generates                                                                            |
| ------------------------ | ------------------------------------------------------------------------------------ |
| `gen_feature_tables.py`  | the README / docs feature tables from `FEATURES.md`                                  |
| `gen_readme_sections.py` | this file's feature-flag, configuration-override, source-tree, and footprint regions |
| `gen_configurator.py`    | the interactive `configurator.html` from `ServerConfig.h`                      |
| `gen_flag_deps.py`       | the build-flag dependency diagram                                                    |
| `gen_api_flow.py`        | the core API-flow diagram                                                            |
| `gen_examples.py`        | the example index in `EXAMPLES.md`                                                   |
| `decorate_changelog.py`  | wraps each release in `CHANGELOG.md` in a collapsible block (CI)                     |

The suite's own generator lives with the tests: [`test/gen_test_readme.py`](../test/gen_test_readme.py) refreshes the env matrix + per-test directory in [`test/README.md`](../test/README.md).

**Web-asset build** (`src/web/wizard/`)

| Script              | Purpose                                                                                    |
| ------------------- | ------------------------------------------------------------------------------------------ |
| `build_assets.py`   | compile the editable web sources (`src/web/input/*`) into embedded C++ application assets   |
| `gen_themes.py`     | build the theme CSS library + gallery from the palette sources                             |
| `gen_theme_blobs.py`| pack the runtime-selectable theme CSS into C++ blobs                                        |
| `gen_favicons.py`   | build the favicon library + gallery                                                        |

```bash
python docs/utilities/gen_readme_sections.py   # refresh this file's generated sections
python src/web/wizard/build_assets.py          # rebuild the embedded web assets
```

</details>

## Testing

**2,300+ Unity tests** across the native suites, all runnable on a native x86/x64 host
(no hardware required). See **[TEST_REPORT.md](../test/TEST_REPORT.md)** for the current
per-suite breakdown and totals. Run a representative subset with:

```
pio test -e native -e native_app -e native_ssh \
         -e native_ssh_hardened -e native_ssh_conn -e native_compliance
```

See the **[test suite README](../test/README.md)** for the suite
breakdown, environment matrix, and per-test directory, and
**[TEST_REPORT.md](../test/TEST_REPORT.md)** for the latest results
(auto-generated by the _Test Report_ GitHub workflow).

## Documentation

Other documentation files in this repository:

<details>
<summary><b>View Documentation Reference Directory</b></summary>

| Document                                       | Contents                                                          |
| ---------------------------------------------- | ----------------------------------------------------------------- |
| [RFC.md](RFC.md)                               | HTTP/1.1, WebSocket, and error-response RFC conformance tables    |
| [SSH.md](SSH.md)                               | SSH-2.0 server: features, RFC/FIPS compliance, auth, memory       |
| [SECURITY.md](SECURITY.md)                     | Security posture (good/ok/bad) and per-feature security treatment |
| [CODEQL.md](CODEQL.md)                         | CodeQL static-analysis setup, coverage, and findings disposition  |
| [HARDWARE_HOOKUP.md](HARDWARE_HOOKUP.md)       | Wiring and settings for codecs that talk to external hardware     |
| [test/README.md](../test/README.md)            | Test suites, environment matrix, per-test directory, how to run   |
| [TEST_REPORT.md](../test/TEST_REPORT.md)       | Latest test results (auto-generated)                              |
| [TODO.md](TODO.md)                             | Outstanding fixes and maintenance                                 |
| [ROADMAP.md](ROADMAP.md)                       | Forward-looking feature backlog (sized S/M/L)                     |
| [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md)   | Deliberate constraints and caveats                                |
| [TUNING.md](TUNING.md)                         | Performance tuning: worker count, core/affinity, poll knobs       |
| [CHANGELOG.md](CHANGELOG.md)                   | Release history                                                   |

</details>

### Generating Docs Locally

To generate the HTML API documentation locally, run the following command from the repository root:

```bash
doxygen docs/Doxyfile
```

The output will be generated in `docs/html/index.html`.

If you are viewing the offline version of this documentation, you can access the latest online version at the [GitHub Pages documentation site](https://dstroy0.github.io/DeterministicESPAsyncWebServer/).

## Licensing & Commercial Use

This library is dual-licensed.

**Open Source.** This library is, and will **ALWAYS REMAIN, FULLY OPEN-SOURCE** under the AGPLv3 (or later). We commit to maintaining a fully featured, parity-matched open-source version available to everyone - from hobbyists and educators to professionals - without hiding any non-proprietary (e.g. custom protocols, intellectual property, confidential telemetry configurations, etc.) feature behind a commercial paywall. It will always be free to use under the AGPLv3 (or later) in any environment that complies with the AGPLv3 (or later) terms. See the `LICENSE` file.

**Commercial.** For teams and applications that cannot meet the AGPLv3 copyleft requirements, a commercial license is available. Contact: Douglas Quigg (dstroy0), dquigg123@gmail.com

**Educators.** Teaching with this? We'd love that. **SERIOUSLY**. Squirty is meant to keep children engaged on the docs page. The docs and styling are set up to appeal to them, hobbyists, and anyone who wants to learn but doesn't know how to style things or glue services together. The library documentation is extensive, extremely thorough, and useful to professionals as well as educators as a teaching tool/classroom prop. If sharing your source under the AGPLv3 isn't practical for a classroom or lab, or you have concerns that have stopped you from using copyleft licensed software before, email Douglas Quigg (dstroy0) at dquigg123@gmail.com from your school address and we'll see what we can do. ESP32 boards are cheap and a hands-on HTTP / IoT-edge stack is a great way into embedded networking, so we're glad to look at education-focused requests one by one. We can't promise an exception for every situation, but please ask. (This is just for genuine educational use; for products, see the commercial option above.) I can help you set up a github repo your students can push to that will help you review their submissions, and walk you through setting up flags for your rubric items. We really need to make an effort to get as many people as possible into the profession, looking at how things work, figuring out how they work on a deeper level, and entering the profession, we need their ideas, we need them now. All great discoveries have come from fresh perspective.

---

<p align="center">
  <img src="squirty.svg" alt="Squirty the Injection Squid" width="64" height="64"><br>
  <b>Squirty the Injection Squid</b>: the official library mascot.<br>
  <sub>Copyright &copy; Douglas Quigg (dstroy0). All rights reserved.</sub>
</p>
