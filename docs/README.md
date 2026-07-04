# Documentation

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1, WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, SNMP, CoAP, Modbus TCP, MQTT, and OPC UA.

> [!WARNING]
> **Extremely active development - expect breaking changes.** This library ships fast: on a busy day that can mean dozens of new features and several public-API breaks. **We fix things the right way and put security and correctness first, even when that breaks backwards compatibility** - include paths, method signatures, defaults, and wire behavior can change between releases. **We do not write backwards-compatibility shims** (the only compatibility we maintain is platform/toolchain support); removing cruft is the price of a clean, auditable, deterministic core. Pin an exact version if you need stability, and read [CHANGELOG.md](CHANGELOG.md) and [MIGRATION.md](MIGRATION.md) before every upgrade.

![Version](https://img.shields.io/badge/version-v4.106.0-blue)
[![Test Build Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml)
[![Docs Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml)
[![Changelog Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml)
[![C++ Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml)
[![Markdown Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml)

## Features

A compile-time menu grouped by the OSI layer each feature lives at, alphabetized within each layer: each cell is an optional `DETWS_ENABLE_*` subsystem (core HTTP/1.1, routing, middleware, JSON, templating, and chunked responses are always on). **Hover an entry for its summary; click through to [FEATURES.md](FEATURES.md) for the full description.** The tables are generated from [FEATURES.md](FEATURES.md) by `docs/utilities/gen_feature_tables.py`, so they never drift.

<!-- BEGIN GENERATED FEATURE TABLES (docs/utilities/gen_feature_tables.py) -->

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Foundation</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#config-io" title="Opt-in schema-driven config export / restore. Default off. Requires CONFIG_STORE. The app declares a fixed schema (key + type); services/config_io serializes the current values to a portable `key=value` text blob (backup / migrate) and parses one back into the store (restore / bulk template). Schema-driven rather than enumerating NVS, so it stays deterministic and zero-heap; the serialize / parse is host-tested.">Config IO</a></td>
  <td align="center"><a href="FEATURES.md#config-store" title="Typed NVS configuration store (WiFi creds, IP config,... as blobs). When set, src/services/config_store/config_store.h provides a typed key/value API (string / u32 / blob) that routes core settings into the ESP32's native NVS partition (via `Preferences`) instead of a JSON file on the filesystem - which survives FS corruption and is the corruption-resistant home for credentials. On host builds it is backed by a fixed in-memory table so the typed contract is unit-testable. Default off.">Config Store</a></td>
  <td align="center"><a href="FEATURES.md#device-id" title="Stable device UUID derived from the chip MAC (RFC 4122 v5). When set, src/services/device_id/device_id.h derives a deterministic v5 UUID from a MAC (via the library's SHA-1) - a storage-free, stable identity for mDNS hostnames, MQTT client IDs, etc. The MAC-&gt;UUID core is host-testable; detws_device_uuid() reads the ESP32 factory MAC. Default off.">Device ID</a></td>
  <td align="center"><a href="FEATURES.md#dma-peripheral-ingest" title="Opt-in DMA ingest / egress path (v5 milestone). Default off. services/dma moves peripheral bytes (UART / I2C / SPI) between the wire and a static buffer while the CPU is free, then a DMA-complete event carries the result up - the high-throughput field-bus ingest path (RS-485 UART, CAN over SPI, IO-Link). RX is double-buffered (ping-pong): the completed buffer is handed to a callback while the engine fills the other; the canonical callback posts the bytes into the preempting work queue so a high-priority task processes them off the interrupt (see Preempting Work Queue). Zero heap (static DETWS_DMA_CHANNELS channels x DETWS_DMA_BUF_SIZE buffers), fail-closed (one TX in flight per channel, feeds past the staging capacity are rejected). DETWS_DMA_SIMULATE (default on) routes transfers through an in-memory ingress/egress simulator - feed bytes in, capture bytes out, optional TX-&gt;RX loopback - so the whole pipeline runs with no physical loopback wire, on the host bench and on-device; a real silicon driver plugs into the det_dma_hw_* hooks when the flag is off. Host-tested (services/dma) and HW-verified on a DevKitV1: 2.2M+ frames ingested with zero integrity errors while an HTTP server was stress-loaded on the same core, no heap growth. See src/services/dma/det_dma.h.">DMA Peripheral Ingest</a></td>
  <td align="center"><a href="FEATURES.md#gpio-map" title="Opt-in browser GPIO pin-mapper / diagnostics endpoint. Default off. When set, services/gpio_map serves a compile-time table of GPIO pins (number, label, direction, live level) as JSON for a browser diag panel, and accepts a control POST (`pin`, `level`) to drive an output. The live read / write uses the Arduino digital API on ESP32; the JSON serializer and the control parser are pure and host-testable.">GPIO Map</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#guardrails" title="Opt-in runtime heap/stack guardrails. Default off. When set, services/guardrails samples free heap, the heap low-water mark, the largest free block (fragmentation), and the calling task's remaining stack, and fires a callback when any crosses its threshold - a proactive fail-safe hook beyond the passive numbers in /metrics. The threshold evaluator and the JSON serializer are pure and host-tested; the sample reads esp_* / the FreeRTOS stack high-water on ESP32.">Guardrails</a></td>
  <td align="center"><a href="FEATURES.md#preempting-work-queue" title="Opt-in real-time ingest primitive (v5 milestone). Default off. Fixed-capacity queues, each feeding one dedicated core-pinned task: a producer posts a fixed-size item from a task (back or urgent-front, each with a wait timeout) or from an ISR (interrupt-safe via xQueueSendFromISR + portYIELD_FROM_ISR), and the scheduler preempts straight to the processing task so the work runs immediately instead of on the next tick - the clean ISR-to-&quot;process now&quot; hand-off for DMA-complete / GPIO / bus events. There are named lanes: one USER lane exposed to the app (the no-arg detws_pq_* API drives it) plus internal DMA / FORWARD / DEVICE lanes for the library's own real-time work. The internal lanes run above the user lane (base DETWS_PQ_INTERNAL_PRIORITY, DMA highest) and below the lwIP tcpip / WiFi tasks, so internal ingest always preempts user work without starving networking; detws_pq_lane_priority() reports the ordering. Zero heap (static FreeRTOS queue storage per lane, compile-time DETWS_PQ_DEPTH / DETWS_PQ_ITEM_SIZE / DETWS_PQ_STACK; a lane's task stack is created only when it starts, so unused lanes cost only their queue storage), fail-closed on a full queue, no hot-path locks so latency stays bounded. detws_pq_high_water_lane() reports the peak depth for sizing. HW-verified on a DevKitV1 (~12 us ISR-to-handler latency; the DMA and USER lanes ran continuously with zero errors under an HTTP flood); host-tested via the per-lane ring core (services/preempt_queue). See src/services/preempt_queue/preempt_queue.h.">Preempting Work Queue</a></td>
  <td align="center"><a href="FEATURES.md#rtc" title="I2C real-time-clock driver (DS1307 / DS3231). Default off. services/rtc reads and sets a battery-backed RTC over I2C (Wire, address 0x68), so the device knows the correct wall-clock time the instant it boots - offline, across power loss - with no network. It is the ideal middle of a time-source chain (GPS -&gt; RTC -&gt; upstream NTP), feeding `detws_time_now()` and the NTP server; `rtc_time_source()` plugs straight into detws_time_source_add(). The BCD &lt;-&gt; Unix-epoch conversion of the seven time registers (12/24-hour, leap years, the clock-halt / century bit masks, range validation) is pure and host-tested across the chip's 2000-2099 range (that round-trip test caught a real 32-bit overflow past 2038); only the register read/write touches I2C. Zero heap. Example 61.Rtc. See src/services/rtc/rtc.h.">RTC</a></td>
  <td align="center"><a href="FEATURES.md#time-source" title="Multi-source time fallback (NTP / RTC / GPS /... by priority). When set, src/services/time_source/time_source.h provides a small registry of user-defined time sources, each a callback returning Unix epoch seconds (0 when that source has no valid time). detws_time_now() queries them in priority order (lowest value first) and returns the first valid result, so the device falls back automatically when its preferred clock is unavailable. Pure and zero-heap (a fixed source table); host-testable. Default off.">Time Source</a></td>
  <td align="center"><a href="FEATURES.md#vfs" title="Unified virtual filesystem wrapper. Default off. services/vfs exposes one small file API (open/read/write/close, exists/size/remove/rename, whole-file helpers) over a pluggable backend, so a feature can target storage without knowing the medium. A built-in zero-heap RAM backend (fixed BSS pool - deterministic, host-identical) ships for scratch / tests; an Arduino-FS backend (ESP32) wraps a real fs::FS (LittleFS / SD / SPIFFS) for persistence. Mount one at startup; the API fails closed otherwise. Pool dimensions are tunable in DetWebServerConfig.h (DETWS_VFS_RAM_FILES, _RAM_FILE_SIZE, _MAX_OPEN, _NAME_MAX).">VFS</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Physical &amp; Data Link (L1-L2)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ads1115" title="TI ADS1115 4-channel 16-bit ADC with programmable gain (I2C). Default off. services/ads1115 builds the 16-bit config-register word for a single-shot single-ended reading (`ads1115_config_single`: the OS start bit, the channel multiplexer, the programmable gain, single-shot mode, the data rate, and the disabled comparator - so `ch0, ±4.096 V, 128 SPS` is `0xC383`; out-of-range fields fall back sanely) and converts the signed 16-bit sample to microvolts for the selected gain's full-scale range (`ads1115_raw_to_uv`: 125 µV/count at ±4.096 V). The config encoder + conversion are pure and host-tested (`native_ads1115`); only the config write / conversion read touches I2C. A cheap solder-and-test breakout for measuring batteries, potentiometers, and analog sensors with far more resolution than the ESP32 ADC. Example 66.Ads1115 reads a potentiometer. See src/services/ads1115/ads1115.h.">ADS1115</a></td>
  <td align="center"><a href="FEATURES.md#bus-capture" title="Wired field-bus listen-only capture - the wired counterpart to Wi-Fi promiscuous capture. Default off. `bus_capture_begin(tx, rx, bitrate, sink)` installs the ESP32 CAN (TWAI) controller in listen-only mode - it receives and decodes every frame on the bus but never ACKs or transmits, so it stays invisible to the other nodes - and hands each decoded `CanFrame` to a sink; `bus_capture_poll()` drains the RX queue from `loop()`. Wire the sink into the forwarding plane (`DETWS_ENABLE_FORWARD`) to bridge captured CAN frames to another interface - e.g. stream them to a wired collector over Ethernet. `can_to_socketcan()` (pure) formats a frame as a 16-byte Linux SocketCAN frame (big-endian `can_id` with the EFF / RTR flags), which with the libpcap `DLT_CAN_SOCKETCAN` link type is a capture Wireshark reads. The SocketCAN framing is pure and host-tested (`native_bus_capture`); the TWAI bring-up is ESP32-only and needs a CAN transceiver. Example 22.CanCapture.">Bus Capture</a></td>
  <td align="center"><a href="FEATURES.md#enocean" title="EnOcean ESP3 serial codec (v5 gateway plugin). Default off. services/enocean is the UART telegram codec for EnOcean Serial Protocol 3, the framing every USB / serial EnOcean gateway module (TCM 310 / USB 300) speaks for energy-harvesting 868 MHz switches and sensors. A telegram is `0x55 | data-len(2) | opt-len(1) | packet-type(1) | CRC8H | data | opt | CRC8D`. `esp3_parse` frames one telegram out of a byte stream (returning the length consumed, 0 for &quot;need more&quot;, or -1 to drop a byte and resynchronise), verifying both CRC-8s (polynomial 0x07); `esp3_build` assembles one; `esp3_crc8` is the shared checksum. This is the radio half of an EnOcean-to-web bridge: pull the sender id + payload out of a RADIO_ERP1 telegram and bridge it northbound with `det_gw_uplink()`. Pure - you feed it the UART bytes, the module does the RF - and fully host-tested (CRC-8 known-answer values incl. the 0xF4 check, a build/parse round trip, malformed framing, and resynchronisation). Example 13.EnOceanGateway reads a real module over UART. See src/services/enocean/enocean.h.">EnOcean</a></td>
  <td align="center"><a href="FEATURES.md#esp-now" title="ESP-NOW peer messaging. Default off. services/espnow wraps ESP-NOW connectionless peer-to-peer radio messaging in a 3-byte typed envelope (magic + type + length) so a receiver can demux by message type and reject a truncated frame, plus a bounded peer registry (DETWS_ESPNOW_MAX_PEERS, no heap). The envelope codec + registry are pure and host-tested; the radio path (begin / add_peer / send / broadcast over esp_now, decoded frames to a callback) is ESP32-only and can bridge to WebSocket/SSE. No stdlib.">ESP-NOW</a></td>
  <td align="center"><a href="FEATURES.md#ethernet" title="Wired Ethernet bring-up. Default off (the ETH library is not linked). When set, the physical layer gains `init_eth_physical()` / `eth_ready()` alongside `init_wifi_physical()` - a thin wrapper over the Arduino ETH library for an RMII PHY (LAN8720 / TLK110 / RTL8201 / DP83848), so the server runs over a wired uplink (PoE, panel-mount, RF-noisy sites) instead of or alongside Wi-Fi. The PHY pins / address / type / clock come from the standard ETH_PHY_* build flags for your board. Nothing else changes: the egress reporting already classifies a wired default route as DETIFACE_ETH (`det_net_egress`, host-tested classifier), so per-route STA/AP/ETH interface filters and every protocol work over the link the moment it has an IP; Wi-Fi and Ethernet can run dual-homed with the stack picking the default route. ESP32-only bring-up (the classifier is host-tested; the PHY needs the hardware). Example 19.Ethernet. See src/network_drivers/physical/physical.h.">Ethernet</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#ina219" title="TI INA219 high-side current / power monitor (I2C). Default off. services/ina219 decodes the bus-voltage register (`ina219_bus_mv`: value in bits [15:3], LSB 4 mV) and the shunt-voltage register (`ina219_shunt_uv`: signed, LSB 10 µV), computes the calibration register from the current LSB and shunt resistance (`ina219_calibration`: `40960000 / (current_lsb_ua * shunt_mohm)`, so 100 µA + 0.1 Ω -&gt; 4096), and scales the raw current / power registers to microamps / microwatts (`ina219_current_ua`, `ina219_power_uw`; the power LSB is 20x the current LSB). All the decode / calibration / scaling math is pure and host-tested (`native_ina219`); only the register read/write touches I2C. A cheap solder-and-test breakout for measuring how much current and power a circuit draws. Example 67.Ina219 is a live power meter. See src/services/ina219/ina219.h.">INA219</a></td>
  <td align="center"><a href="FEATURES.md#interface-forwarding" title="Opt-in forwarding plane (v5 milestone). Default off. services/forward turns the device into a bridge / router: register interfaces (Wi-Fi STA / AP, Ethernet, a peripheral bus, a radio), each with an egress send callback, then add per-pair rules (`det_forward_add_rule(src, dst, ALLOW/DENY, rate_cap)`). A frame arriving on one interface (`det_forward_ingress()`, the canonical wiring being a DMA-complete event posted onto the internal FORWARD lane of the preempting queue) is forwarded to every allowed destination by calling that destination's send callback, so the device bridges / routes between its interfaces instead of only terminating traffic. Default-deny (a pair forwards only with an ALLOW rule and no DENY; a DENY always wins), never reflects a frame to its source, and fail-closed (an exceeded rate cap or a send callback returning false drops and is counted via `det_forward_get_stats()`, never blocks). Multi-destination fan-out (several ALLOW rules for one source) gives hub behavior; a single ALLOW gives point-to-point routing. An optional ingress **ACL** filters frames by content before any forwarding rule runs: ordered entries (`det_forward_acl_add()`) match on the source interface (or DET_FWD_IF_ANY) and a byte pattern under a mask at an offset, first-match-wins, with a configurable default action (`det_forward_acl_set_default()`) - permit-by-default for a denylist, or deny-by-default for an allowlist. Static tables (zero heap): DETWS_FWD_MAX_IFACES interfaces, DETWS_FWD_MAX_RULES rules, DETWS_FWD_MAX_ACL ACL entries. Host-tested (services/forward) + HW-verified on a DevKitV1: 800k+ frames ingested over DMA, ACL-filtered, and forwarded through the plane with exact accounting (forwarded + acl_denied == frames_in), zero loss, and zero integrity errors while an HTTP server was stress-loaded on the same core. This is the generic data path the post-v5 wireless gateway bridges sit on. See src/services/forward/det_forward.h.">Interface Forwarding</a></td>
  <td align="center"><a href="FEATURES.md#ld2410" title="HLK-LD2410 24 GHz mmWave presence / motion radar (UART). Default off. services/ld2410 syncs to the module's framed serial output (256000 baud: header `F4 F3 F2 F1`, little-endian length, payload, footer `F8 F7 F6 F5`) and decodes the target report - presence state (none / moving / stationary / both), the moving and stationary target distance (cm) and energy (0-100), the overall detection distance, and, in engineering mode, the per-gate energy of all nine range gates - plus encodes the config commands (enter / exit config, enable / disable engineering, restart). Unlike a PIR sensor it detects a perfectly still person (micro-motion / breathing), in the dark, through thin walls. The `Ld2410Stream` byte-by-byte reassembler is fixed-buffer, no-heap, and resyncs cleanly on dropped bytes or noise; the frame decoder + reassembler + command encoders are host-tested (`native_ld2410`), and only the UART read/write touches hardware. Example 62.Ld2410 lights the onboard LED on presence. See src/services/ld2410/ld2410.h.">LD2410</a></td>
  <td align="center"><a href="FEATURES.md#lora" title="Opt-in LoRa radio codec + driver (v5 gateway plugin). Default off. services/lora is the southbound-radio half of a LoRa-to-web bridge (see Radio Gateway), in two layers. **Codec**: `lora_frame_parse` / `lora_frame_build` handle the RadioHead-compatible 4-byte header (`to` / `from` / `id` / `flags`) that virtually every hobby / sensor LoRa deployment lays over the header-less LoRa PHY. **Driver**: the Semtech SX127x (SX1276-79 / RFM95-96) register protocol - `lora_init` (verifies the chip id, switches to LoRa mode, programs the carrier frequency, spreading factor / bandwidth / coding rate, sync word, and PA power), `lora_send` / `lora_tx_done`, `lora_set_rx`, and `lora_recv` (reads the FIFO + packet RSSI on RxDone, drops on a CRC error) - all over a caller-supplied register-access bus (two callbacks that read/write a chip register), so the SPI + chip-select wiring is the integration's and the register sequence is portable. Bridge received frames northbound with `det_gw_uplink()`. The codec and the full register protocol are host-tested against a mock SX127x (a register file + a FIFO with the chip's auto-incrementing address pointer); the RF link itself needs the module. Example 11.LoRaGateway drives a real RFM95 over SPI. See src/services/lora/lora.h.">LoRa</a></td>
  <td align="center"><a href="FEATURES.md#mpr121" title="NXP MPR121 12-channel capacitive-touch controller (I2C). Default off. services/mpr121 decodes the touch-status word (`mpr121_touched` masks the 12 electrode bits out of the 16-bit status, which also carries the proximity electrode at bit 12 and the over-current flag at bit 15) and the chip's 10-bit filtered / baseline per-electrode data (`mpr121_word10`), and builds the whole register bring-up as `(register, value)` byte pairs (`mpr121_build_init`: soft reset, the NXP AN3944 rising/falling/touched filter defaults, per-electrode touch/release thresholds, CONFIG1/2, and the electrode-configuration register that starts it running with baseline tracking). The decode + init-sequence builder are pure and host-tested (`native_mpr121`); only the register read/write touches I2C. A cheap solder-and-test breakout for touch buttons / sliders. Example 63.Mpr121 prints which pad you touch. See src/services/mpr121/mpr121.h.">MPR121</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#nrf24" title="Opt-in nRF24L01+ radio driver (v5 gateway plugin). Default off. services/nrf24 is a driver for the Nordic nRF24L01+ 2.4 GHz module - cheap point-to-multipoint sensor links bridged to the web stack. Unlike the SX127x (plain register read/write), the nRF24 speaks an SPI command protocol (each transaction is a command byte + data with the STATUS register shifted out first) and needs a separate CE pin, so the driver runs over an `nrf_bus` that carries a full-duplex SPI transfer plus a CE-set callback - the only board-specific code. `nrf24_init` verifies the chip via a register read-back and programs the channel, data rate, power, 5-byte address, and static payload width; `nrf24_send` (zero-padded to the width) / `nrf24_tx_done`, `nrf24_set_rx`, and `nrf24_recv` (reports the receiving pipe). The chip's hardware pipe addressing means a received frame's source is the pipe number (no in-payload codec); bridge it northbound with `det_gw_uplink(port, pipe, ...)`. The command protocol is host-tested against a mock chip (register file + payload buffers + STATUS write-1-to-clear); the RF link itself needs the module. Example 12.Nrf24Gateway drives a real module over SPI. See src/services/nrf24/nrf24.h.">nRF24</a></td>
  <td align="center"><a href="FEATURES.md#pca9685" title="NXP PCA9685 16-channel 12-bit PWM / servo driver (I2C). Default off. services/pca9685 turns the ESP32's two I2C wires into sixteen hardware PWM outputs. `pca9685_prescale` computes the PRESCALE register for a PWM frequency from the 25 MHz oscillator (`round(25e6 / (4096*freq)) - 1`, clamped 3..255; 50 Hz -&gt; 121); `pca9685_channel_reg` gives a channel's register base (`0x06 + 4*channel`); `pca9685_us_to_count` converts a servo pulse width (microseconds) to a 12-bit OFF count at the configured frequency; and `pca9685_set_pwm_bytes` packs the 5-byte channel write (12-bit ON/OFF little-endian, preserving the full-on/off flag). The prescale / count math + the register encoder are pure and host-tested (`native_pca9685`); only the register writes touch I2C. A cheap solder-and-test breakout for up to 16 servos or LEDs. Example 65.Pca9685 sweeps a servo. See src/services/pca9685/pca9685.h.">PCA9685</a></td>
  <td align="center"><a href="FEATURES.md#pn532" title="PN532 NFC frame codec (v5 gateway plugin). Default off. services/pn532 is the command-frame protocol of the NXP PN532 - the ubiquitous NFC / RFID reader on I2C / SPI / HSU breakouts - so a tag read/write becomes an HTTP / MQTT event. The host and the chip exchange normal information frames `00 00 FF | LEN | LCS | TFI | PData | DCS | 00`, where TFI is 0xD4 (host) / 0xD5 (chip), LCS is the length checksum and DCS the data checksum, plus a 6-byte ACK frame. `pn532_build_frame` assembles a command (the per-command PData - GetFirmwareVersion, InListPassiveTarget, InDataExchange - is the application's), `pn532_parse_frame` frames + verifies a response (returning the length consumed, need-more, or a resync signal), and `pn532_is_ack` / `pn532_build_ack` handle the ACK. Pure - you carry the bytes over your I2C / SPI / UART - and host-tested against the documented GetFirmwareVersion command / response frames and their LCS / DCS. Example 14.NfcGateway reads a real PN532 over I2C and bridges each tag UID northbound. See src/services/pn532/pn532.h.">PN532</a></td>
  <td align="center"><a href="FEATURES.md#radio-gateway" title="Opt-in radio / wireless gateway bridge (v5 milestone). Default off. services/gateway is the generic southbound-to-northbound bridge that ties the hardware-ingest pipeline to the web stack: a southbound radio (LoRa / nRF24 / CC1101 / Zigbee / Z-Wave / ... reached over SPI / I2C / UART) is a **port**. When it receives a frame - the data-ready ISR reads it over DMA (services/dma), posts it onto the FORWARD lane (services/preempt_queue), and a per-radio codec extracts the source node address + payload - you call `det_gw_uplink()`; the gateway envelopes it (source address, port, RSSI, sequence) and publishes it northbound through the uplink callback, which you wire to MQTT / HTTP / WebSocket / UDP. A command runs the other way via `det_gw_downlink()` to the port's transmit callback (the radio's SPI / UART write). `det_gw_topic()` formats a routing key `&lt;prefix&gt;/&lt;port&gt;/&lt;addr&gt;`; a per-port uplink rate cap and fail-closed drops (no sink / unknown port / exceeded cap / refused, all counted via `det_gw_get_stats()`) keep it bounded. The radio transmit and the northbound publish are callbacks (the seam a real radio driver / protocol binding plugs into), so the bridge is host- and device-testable with no radio. Zero-heap static tables (DETWS_GW_MAX_PORTS). Host-tested (services/gateway) + HW-verified on a DevKitV1: 690k+ radio frames bridged northbound over DMA + the FORWARD lane with exact accounting (up_in == published) and zero payload-integrity errors while an HTTP server was stress-loaded on the same core. This is the framework the per-radio gateways (LoRa, Zigbee, ...) on the roadmap plug into. See src/services/gateway/det_gateway.h.">Radio Gateway</a></td>
  <td align="center"><a href="FEATURES.md#radio-power" title="Opt-in radio power controls. Default off. services/radio_power applies the WiFi modem-sleep mode and an optional max-TX-power cap in one call (esp_wifi_set_ps / esp_wifi_set_max_tx_power) - trade throughput/latency for lower average power on a battery device. The mode names are host-tested; the apply is ESP32-only.">Radio Power</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#sht3x" title="Sensirion SHT3x (SHT30/31/35) temperature / humidity sensor (I2C). Default off. services/sht3x issues the single-shot measurement command (`0x2400`), then verifies the CRC-8 on each returned 16-bit word before trusting it (`sht3x_crc8`: polynomial 0x31, init 0xFF - the Sensirion datasheet check value `0xBEEF` -&gt; `0x92` is a unit test) and converts the raw ticks with the linear map (`T = -45 + 175*raw/65535`, `RH = 100*raw/65535`) into signed integer milli-units (milli-degrees C, milli-percent RH), so there is no heap and no float printf. `sht3x_parse` rejects the whole six-byte reading on any CRC mismatch. The CRC + conversion + parse are pure and host-tested (`native_sht3x`); only the command write / data read touches I2C. A cheap solder-and-test breakout (GY-SHT31) for environmental telemetry. Example 64.Sht3x prints temperature and humidity. See src/services/sht3x/sht3x.h.">SHT3x</a></td>
  <td align="center"><a href="FEATURES.md#sigfox" title="Sigfox modem AT-command codec (v5 gateway plugin). Default off. services/sigfox is the tiny-uplink half of a Sigfox-to-web bridge: a Wisol (SFM10R) / Murata Sigfox modem is driven by AT commands over UART for ultra-low-power telemetry over the Sigfox 0G network (a message is capped at 12 bytes and ~140/day, so uplinks are rare and small). `sigfox_build_uplink` formats an `AT$SF=&lt;hex&gt;` command (uppercase hex of the payload) and `sigfox_parse_response` classifies the modem's reply as OK, ERROR, or still pending. Uplink-only (the common Sigfox use - a device sends readings up, it is not addressed downlink). Pure text codec - you carry the bytes over your UART - and host-tested (the AT$SF encoding, its 12-byte / output-buffer bounds, and the response classification). Example 15.SigfoxUplink sends a reading from a real modem. See src/services/sigfox/sigfox.h.">Sigfox</a></td>
  <td align="center"><a href="FEATURES.md#thread" title="Thread spinel / HDLC-lite framing codec (v5 gateway plugin). Default off. services/thread is the HDLC-lite data-link layer that carries spinel frames to an OpenThread radio co-processor (an nRF52840 / EFR32 RCP) over UART, so an 802.15.4 / Thread mesh is bridged to IP and the web (the basis of a Thread / Matter border router). HDLC-lite appends an FCS, byte-stuffs the reserved bytes (Flag 0x7E, Escape 0x7D, XON, XOFF), and terminates with the Flag. The FCS is the HDLC frame check sequence - CRC-16/X-25 (poly 0x1021 reflected, init 0xFFFF, reflected in/out, final XOR 0xFFFF), transmitted low byte first, distinct from Zigbee's ASH CRC. `spinel_frame_encode` wraps a payload; `spinel_frame_decode` finds the flag, removes the stuffing, and verifies the FCS (returning the bytes consumed, need-more, or a resync signal); `spinel_fcs` is the shared checksum. On top of the framing there is a **spinel command layer**: `spinel_pack_uint` / `spinel_unpack_uint` handle spinel's packed-unsigned-integer encoding (7 bits/byte, little-endian, continuation bit), and `spinel_command_build` / `spinel_command_parse` assemble and split a property command (`header | CMD | PROP | value`) - so you can issue a `PROP_VALUE_GET`/`SET` or read a `PROP_VALUE_IS` update, not just move opaque frames. Pure - you carry the bytes over your UART - and host-tested against the CRC-16/X-25 catalog check value (`0x906E`), the byte-stuffing round trip, the packed-int values (127 -&gt; `7F`, 128 -&gt; `80 01`, 1337 -&gt; `B9 0A`), and the full command -&gt; frame -&gt; decode -&gt; parse stack. Example 18.ThreadGateway bridges a real RCP. See src/services/thread/thread.h.">Thread</a></td>
  <td align="center"><a href="FEATURES.md#wi-fi-capture" title="Passive 802.11 promiscuous (monitor) capture. Default off. `promisc_begin(channel, sink)` puts the radio in promiscuous mode (`esp_wifi_set_promiscuous`) and delivers every frame - with RSSI and channel - to a sink; capture is strictly passive (no injection). Wire the sink into the forwarding plane (`DETWS_ENABLE_FORWARD`) to bridge captured Wi-Fi frames to another interface - e.g. stream them to a wired collector over Ethernet (&quot;capture on Wi-Fi, forward to Ethernet&quot;). Ships a pure 802.11 MAC-header parser (`wifi_frame_parse`: type/subtype, the to/from-DS src/dst/bssid layout, QoS, WDS 4-address, sequence number) and libpcap framing (`DLT_IEEE802_11`) so a forwarded frame is a valid PCAP a wired Wireshark / tcpdump reads. The parser and PCAP framing are pure and host-tested (`native_promisc`); the radio bring-up is ESP32-only. Example 21.WifiCapture.">Wi-Fi Capture</a></td>
  <td align="center"><a href="FEATURES.md#z-wave" title="Z-Wave Serial API frame codec (v5 gateway plugin). Default off. services/zwave is the host-side Serial API of a Silicon Labs 500 / 700-series Z-Wave controller over UART, so a Z-Wave mesh is bridged to the web. Data frames are `SOF (0x01) | LEN | Type | Command | Data | Checksum`, where LEN counts Type..Checksum, Type is 0x00 (REQ) / 0x01 (RES), and the checksum is 0xFF XOR-folded over LEN..last-data; single-byte ACK (0x06) / NAK (0x15) / CAN (0x18) frames flow-control them. `zwave_build_frame` assembles a function command (GetVersion, SendData, AddNodeToNetwork, ...), `zwave_parse_frame` frames + verifies a response (length / need-more / resync), and `zwave_is_ack` / `_nak` / `_can` / `zwave_build_ack` handle the control bytes; the per-command payload is the application's. Pull the source node id + payload out of an ApplicationCommandHandler report and bridge it northbound with `det_gw_uplink()`. Pure - you carry the bytes over your UART - and host-tested against the documented GetVersion frame (`01 03 00 15 E9`) and its XOR checksum. Example 16.ZWaveGateway bridges a real controller. See src/services/zwave/zwave.h.">Z-Wave</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#zigbee" title="Zigbee EZSP / ASH framing codec (v5 gateway plugin). Default off. services/zigbee is the ASH (Asynchronous Serial Host, UG101) data-link layer that carries EZSP frames to a Silicon Labs EmberZNet network co-processor over UART, so a Zigbee network is bridged to the web. Each ASH frame is `[control | payload | CRC-16/CCITT]`, byte-stuffed so the reserved control bytes (Flag 0x7E, Escape 0x7D, XON/XOFF, Substitute, Cancel) never appear in the body, and terminated by the Flag byte. `ash_frame_encode` wraps a control byte + payload into a stuffed, CRC'd frame; `ash_frame_decode` finds the flag, removes the stuffing, and verifies the CRC (returning the bytes consumed, need-more, or a resync signal); `ash_crc16` is the shared checksum. Bridge the EZSP payload of an incoming-message DATA frame northbound with `det_gw_uplink()`; interpreting the EZSP command itself is the application's. Pure - you carry the bytes over your UART - and host-tested against the documented ASH RST frame (`C0 38 BC 7E`), the CRC-16 (`0x38BC`), and the byte-stuffing round trip. Example 17.ZigbeeGateway bridges a real NCP. See src/services/zigbee/zigbee.h.">Zigbee</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Network (L3)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#dns-resolver" title="Opt-in DNS resolver with answer verification. Default off. services/dns_resolver resolves a hostname to an IPv4 address (lwIP dns_gethostbyname, marshalled to tcpip_thread like the http_client) and can reject suspicious answers - 0.0.0.0, broadcast, loopback, multicast - which are spoofing / DNS-rebinding indicators for a remote host. The address classifier / verifier is pure and host-tested; the resolve is ESP32-only (blocking, so call it off the request hot path).">Dns Resolver</a></td>
  <td align="center"><a href="FEATURES.md#ipv6" title="Dual-stack IPv6. Default off. The TCP and UDP listeners already bind `IPADDR_TYPE_ANY`, so the server answers over IPv6 the moment the interface has a v6 address; `DETWS_ENABLE_IPV6` turns IPv6 on for the Wi-Fi netif (`init_ipv6_physical()` -&gt; SLAAC: a `fe80::` link-local address plus a global one if a router advertises a prefix), and `net_global_ipv6()` reads the acquired global address from lwIP. The DetIp address core (`network_drivers/network/det_ip.h`) is one family-tagged type for both v4 and v6, with RFC 4291 text parsing (`::` zero-compression, embedded-v4 `::ffff:a.b.c.d`), RFC 5952 canonical formatting, and scope classification (loopback / link-local / private-ULA / multicast / global). The address core is pure and host-tested (`native_det_ip`); the netif bring-up is ESP32-only. Example 20.IPv6. Requires an lwIP built with `LWIP_IPV6=1` (the stock Arduino-ESP32 core ships it).">IPv6</a></td>
  <td align="center"><a href="FEATURES.md#proxy-protocol" title="HAProxy PROXY protocol codec - recover the real client IPv4 when the server sits behind a load balancer / reverse proxy that prepends a PROXY header. Default off. services/proxy_protocol provides `proxy_parse` (detects + decodes a v1 text `PROXY TCP4 ...\r\n` header or a v2 binary header - 12-octet signature + ver_cmd / family / address block - and reports the bytes to skip before the real stream) plus `proxy_v1_build` / `proxy_v2_build` for a TCP/IPv4 header. Handles the library's IPv4 family; IPv6 / UNIX / LOCAL headers parse to their length but yield no addresses. Format per the HAProxy PROXY protocol spec; pure and host-tested. See src/services/proxy_protocol/proxy_protocol.h.">Proxy Protocol</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Transport (L4)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#accept-throttle" title="Opt-in global accept-rate throttle (connection-flood defense). Default off (zero cost / no behavior change). When set to 1 the accept callback rejects new connections once more than DETWS_ACCEPT_THROTTLE_MAX have been accepted within a DETWS_ACCEPT_THROTTLE_WINDOW_MS fixed window (global across all listeners, two static counters - no per-IP table). This bounds connection churn (e.g. reconnect brute-force) on top of the bounded connection pool and the per-connection auth limits. mitigate finer-grained / per-IP attacks at the network layer.">Accept Throttle</a></td>
  <td align="center"><a href="FEATURES.md#ip-allowlist" title="Opt-in source-IP allowlist (accept-time firewall, keyed by source IPv4). Default off (zero cost / no behavior change). When set, the accept callback drops any connection whose source address does not match a configured CIDR rule (see listener_ip_allow_add()). An empty allowlist allows everything, so enabling the feature before adding rules never locks the device out. Rules live in a fixed BSS table of DETWS_IP_ALLOWLIST_SLOTS entries (no heap). This is a coarse first-line filter - a spoofed source address can still pass it - so combine it with the accept throttles and network-layer filtering.">IP Allowlist</a></td>
  <td align="center"><a href="FEATURES.md#keep-alive" title="HTTP/1.1 persistent connections (keep-alive). Default off (every response carries `Connection: close` and the connection is closed after one request - the long-standing behavior). When set to 1, a cleanly-parsed request is answered with `Connection: keep-alive` and the slot is recycled for the next request on the same socket: HTTP/1.1 keeps the connection open unless the client sends `Connection: close`; HTTP/1.0 closes unless the client sends `Connection: keep-alive`. Error responses (400/413/414 and any non-PARSE_COMPLETE path) always close, since the next request boundary is unknown. Idle keep-alive connections are still reclaimed by the existing conn_timeout sweep, and each connection serves at most DETWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.">Keep-Alive</a></td>
  <td align="center"><a href="FEATURES.md#mtls" title="Mutual TLS - require and verify a client certificate (mTLS). Default off. When set (requires TLS), the server can be given a trust-anchor CA via DetWebServer::tls_require_client_cert(): the TLS handshake then demands a client certificate chaining to that CA (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents none or an untrusted one. The verified peer's subject DN is available to handlers via DetWebServer::tls_client_subject(). Strong transport-level client authentication with no passwords.">MTLS</a></td>
  <td align="center"><a href="FEATURES.md#per-ip-throttle" title="Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4). Default off (zero cost / no behavior change). Complements the global accept throttle: the accept callback rejects a new connection once one source IPv4 address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within a DETWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of DETWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source addresses; when a new address arrives and the table is full, an expired or least-recently-started bucket is reused, so memory stays bounded (no heap). This bounds reconnect/brute-force churn from a single host (the gap left by the global throttle, which cannot tell one noisy client from many). It is best-effort: an attacker spreading across many source addresses can still churn the bounded connection pool, so combine it with the global throttle and network-layer filtering.">Per IP Throttle</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#tls" title="TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only). When set, the server can accept TLS connections using mbedTLS configured with MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DETWS_TLS_ARENA_SIZE) - no system heap, so the determinism guarantee is preserved. The TLS engine is compiled only on Arduino/ESP32 (mbedTLS is not part of the native build). Default off.">TLS</a></td>
  <td align="center"><a href="FEATURES.md#tls-resumption" title="TLS session resumption via RFC 5077 session tickets (requires TLS). Default off. When set, the TLS 1.2 server issues encrypted session tickets and accepts them on reconnect, so a returning client completes an abbreviated handshake (no certificate or full key exchange) - much faster and far less CPU than the ~RSA/ECDHE full handshake. Resumption is stateless: the session state lives in the client's ticket, sealed with a server-held key, so there is no growing per-session cache (the determinism / zero-heap-growth guarantee holds; only a small fixed ticket key and a little arena headroom are added). The ticket key rotates automatically on the DETWS_TLS_TICKET_LIFETIME_S schedule. Needs the mbedTLS build to provide MBEDTLS_SSL_TICKET_C (stock arduino-esp32 does).">TLS Resumption</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Session (L5)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#ssh" title="SSH server support (RFC 4253/4252/4254). Channels are multiplexed per connection (`DETWS_SSH_MAX_CHANNELS`, default 1), each routed by its recipient id with its own flow-control window. Beyond `session` channels, `direct-tcpip` (the `ssh -L` local-forward request) is parsed and routed through a normalized forwarding seam: the codec extracts the target host:port and routes channel data but does no TCP I/O itself. With `DETWS_SSH_PORT_FORWARD` set, the `ssh_forward` owner plugs into that seam and does the I/O - it opens the outbound TCP through the client transport (det_client) and bridges bytes both ways, with an optional target policy callback, a per-poll target-to-client pump bounded by the channel window, and EOF/CLOSE propagation. Forwarding is opt-in twice over (compiled out by default, and inert until the app calls `ssh_forward_begin()`) because any authenticated client could otherwise reach arbitrary hosts; with it off every `direct-tcpip` open is refused (no open relay).">SSH</a></td>
  <td align="center"><a href="FEATURES.md#telnet" title="Telnet server support (RFC 854 / IAC option negotiation).">Telnet</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Presentation (L6)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#auth" title="HTTP Basic Authentication per-route.">Auth</a></td>
  <td align="center"><a href="FEATURES.md#auth-lockout" title="Opt-in per-IP brute-force lockout for HTTP auth (requires AUTH). Default off (zero cost / no behavior change). When set, the auth gate counts consecutive failed authentications per source IPv4 in a fixed BSS table; after DETWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for DETWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to DETWS_AUTH_LOCKOUT_MAX_MS. A locked address gets 429 (Retry-After) with no credential check; a successful auth clears it. Bounded memory (no heap); the table evicts idle, then least-recently-used, addresses when full.">Auth Lockout</a></td>
  <td align="center"><a href="FEATURES.md#cbor" title="Zero-heap CBOR (RFC 8949) encoder for compact binary payloads. Default off. When set, network_drivers/presentation/cbor/cbor.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, null, and float32 into a caller-provided buffer - a compact binary alternative to the JSON writer for telemetry. Pure, no heap, host-tested against the RFC 8949 vectors.">CBOR</a></td>
  <td align="center"><a href="FEATURES.md#cloudevents" title="CloudEvents v1.0 (CNCF) event envelope. Default off. services/cloudevents makes a device's events interoperable with serverless / event-mesh consumers: `cloudevents_build_json()` emits a structured `application/cloudevents+json` envelope (the required `id` / `source` / `type` plus optional `subject` / `datacontenttype` / `data`) over the JSON writer, and `cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers. Pure and host-tested. See src/services/cloudevents.h.">CloudEvents</a></td>
  <td align="center"><a href="FEATURES.md#http11-parser" title="RFC 7230 request parser - validates method, path, header names and values byte-by-byte before storing anything. Always on.">HTTP/1.1 Parser</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#json" title="Zero-heap JSON writer/reader (json.h) for request bodies and responses. Always on.">JSON</a></td>
  <td align="center"><a href="FEATURES.md#jwt" title="JWT bearer-token authentication (HS256). Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer &lt;jwt&gt;` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a stateless token. Signature verification is constant-time.">JWT</a></td>
  <td align="center"><a href="FEATURES.md#messagepack" title="Zero-heap MessagePack encoder and decoder for compact binary payloads. Default off. When set, network_drivers/presentation/msgpack/msgpack.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, nil, and float32 into a caller-provided buffer, plus a cursor decoder (msgpack_peek / msgpack_read_\*, no-copy strings) over a caller buffer - the MessagePack-format sibling of the CBOR / JSON readers and writers. Pure, no heap, host-tested against the spec encodings and round-trip.">MessagePack</a></td>
  <td align="center"><a href="FEATURES.md#multipart" title="multipart/form-data body parser.">Multipart</a></td>
  <td align="center"><a href="FEATURES.md#protobuf" title="Protocol Buffers wire codec. Default off. services/protobuf is a zero-heap streaming Protobuf encoder + cursor reader over caller buffers (the same shape as the CBOR / MessagePack codecs): a writer for varint / ZigZag / fixed32 / fixed64 / length-delimited fields (`pb_uint64`, `pb_sint64`, `pb_fixed32`, `pb_fixed64`, `pb_float`, `pb_double`, `pb_bytes`, `pb_string`; embedded messages are built into a sub-buffer and added with `pb_bytes`), and a reader (`pb_read_field`) that decodes one field at the buffer head and reports bytes consumed, with ZigZag / float / double value decoders. Host-tested against the spec vectors. This is the standalone Protobuf deliverable; gRPC (framed Protobuf over HTTP/2) is gated on the HTTP/2 roadmap item. See src/services/protobuf/protobuf.h.">Protobuf</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#senml" title="SenML (RFC 8428) measurement-pack builder. Default off; implies CBOR. services/senml is a zero-heap SenML-JSON + SenML-CBOR encoder over the shipped JSON / CBOR writers: the caller fills a `SenmlRecord` array (optional base name / base time, name, unit, one value - number / string / boolean, optional time) and `senml_json_build` / `senml_cbor_build` emit the whole pack. SenML-JSON uses the text labels (bn/n/u/v/vs/vb/t); SenML-CBOR uses the integer labels (n=0, u=1, v=2, ..., bn=-2, bt=-3). Numbers that are integral are emitted as integers so timestamps keep full precision. The standard measurement format for CoAP / LwM2M / HTTP telemetry; verified against the RFC example, host-tested. See src/services/senml/senml.h.">SenML</a></td>
  <td align="center"><a href="FEATURES.md#sse" title="Server-Sent Events push support.">SSE</a></td>
  <td align="center"><a href="FEATURES.md#web-terminal" title="Browser &quot;web serial&quot; terminal over WebSocket (src/services/web_terminal). Serves a self-contained terminal page and a WebSocket endpoint: device output is broadcast to all connected browsers, browser input is delivered to a command callback. Requires WEBSOCKET. Default off.">Web Terminal</a></td>
  <td align="center"><a href="FEATURES.md#websocket" title="WebSocket support (RFC 6455 framing + SHA-1/base64 handshake).">WebSocket</a></td>
  <td align="center"><a href="FEATURES.md#ws-deflate" title="WebSocket permessage-deflate (RFC 7692) - bidirectional compression. When set (and WEBSOCKET is on), the server negotiates the `permessage-deflate` extension and both decompresses inbound compressed (RSV1) messages via a bounded INFLATE (network_drivers/presentation/inflate._) and compresses outbound data frames via a bounded DEFLATE (network_drivers/presentation/deflate._); both borrow their table scratch from the shared per-dispatch arena. The extension is negotiated with `{client,server}_no_context_takeover` so every message (de)compresses independently - no window is carried between messages. An outbound frame that would not shrink is sent uncompressed (the per-message RSV1 flag permits this). Default off.">WS Deflate</a></td>
</tr>
</tbody>
</table>

<table class="feature-table" width="100%">
<thead><tr><th colspan="5" align="center">Application (L7)</th></tr></thead>
<tbody>
<tr>
  <td align="center"><a href="FEATURES.md#amqp" title="AMQP 0-9-1 frame codec - the RabbitMQ wire protocol. Default off. services/amqp lets a device be an AMQP client over the outbound client transport: `amqp_protocol_header` writes the `&quot;AMQP&quot; 0 0 9 1` preamble, `amqp_build_frame` / `amqp_parse_frame` build and validate a frame (type + channel + 4-octet size + payload + the mandatory 0xCE frame-end), `amqp_build_method` / `amqp_parse_method` handle a METHOD frame's class-id / method-id / arguments, and `amqp_build_heartbeat` emits a keep-alive. Pure and host-tested; the method-argument field encoding and the connection state are the application's. See src/services/amqp/amqp.h.">AMQP</a></td>
  <td align="center"><a href="FEATURES.md#audit-log" title="Tamper-evident audit log. Default off. services/audit_log keeps an append-only, hash-chained security log: each record carries SHA-256(prev_hash || fields), so altering, deleting, or reordering any retained record breaks the chain (detws_audit_verify() detects it). Storage is a fixed RAM ring of DETWS_AUDIT_LOG_ENTRIES records (no heap); when it wraps, a moving anchor keeps the retained window verifiable. Install a sink (detws_audit_set_sink) to forward every record at creation time to a durable / remote store - SD-card file, syslog or HTTP log service, serial console - preserving the same chain off-device. Pure and host-tested.">Audit Log</a></td>
  <td align="center"><a href="FEATURES.md#bacnet" title="BACnet/IP BVLC + NPDU codec - the ASHRAE 135 building-automation network framing over UDP (47808). Default off. services/bacnet provides `bvlc_build` / `bvlc_parse` for the BACnet/IP virtual-link envelope (type 0x81, function such as Original-Unicast-NPDU 0x0A / Original-Broadcast-NPDU 0x0B, 2-octet length) and `npdu_build` / `npdu_parse` for the network layer (version 0x01 + the NPCI control octet + optional DNET/DLEN/DADR destination addressing + hop count), slicing out the APDU. Layout verified against ASHRAE 135 Annex J / Clause 6; pure and host-tested. The APDU application layer (objects / properties / services) layers on top. See src/services/bacnet/bacnet.h.">BACnet</a></td>
  <td align="center"><a href="FEATURES.md#c37118" title="IEEE C37.118.2 synchrophasor frame codec. Default off. services/c37118 is a zero-heap builder + CRC-validating parser for the PMU / PDC wide-area measurement wire protocol: `c37118_build_frame` emits a `SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK` frame (CHK = CRC-CCITT) for any payload, `c37118_build_command` handles the fixed Command frame, and `c37118_parse_frame` validates the CRC and reports the frame type / id / timestamp / payload slice (with `c37118_parse_command`). CRC verified against the canonical CRC-CCITT-FALSE check value; pure and host-tested. The fixed phasor configuration / data model layered on the framing is a future addition. See src/services/c37118/c37118.h.">C37.118</a></td>
  <td align="center"><a href="FEATURES.md#canopen" title="CANopen (CiA 301) message codec. Default off. services/canopen is a zero-heap builder + parser for the CANopen messaging set over classic CAN frames (`shared_primitives/det_can.h`): NMT node control, SYNC, TIME, the heartbeat / boot-up (NMT error control), EMCY, PDO process data, and expedited SDO read / write / abort. The 11-bit COB-ID is a 4-bit function code plus a 7-bit node id, so `canopen_build_*` compute it and `canopen_parse` classifies a received frame back to its function + node; `canopen_parse_sdo_response` decodes a server's expedited upload value, download acknowledge, or abort code. The object dictionary itself is the application's, and SDO is expedited only (segmented / block transfer is a future addition). Identifier allocation verified against CiA 301; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a CANopen field bus onto Wi-Fi. See src/services/canopen/canopen.h.">CANopen</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#chunked-responses" title="Streaming / chunked responses of unbounded length in constant memory via send_chunked(). Always on.">Chunked Responses</a></td>
  <td align="center"><a href="FEATURES.md#cip" title="CIP (Common Industrial Protocol) message codec - the message that rides inside an EtherNet/IP Unconnected Data item (DETWS_ENABLE_ENIP). Default off. services/cip builds the request - `cip_build_epath` encodes a class/instance/attribute EPATH from logical segments (`0x20 | type | format`, 8- or 16-bit ids), `cip_build_get_attr_single` / `cip_build_request` prepend the service code + path size - and `cip_parse_response` splits a reply into service / general status / additional status / data. Service codes and the segment encoding are verified against the Wireshark CIP dissector. Pure and host-tested; wrap the request with `eip_build_send_rr_data` for a working CIP read path. See src/services/cip/cip.h.">CIP</a></td>
  <td align="center"><a href="FEATURES.md#coap" title="CoAP server (RFC 7252) over UDP/5683. A zero-heap Constrained Application Protocol endpoint: a fixed resource table dispatched against the request's Uri-Path, with a pure host-testable message codec (parse/build) and an ESP32 UDP binding via the transport-layer UDP service. Default off; the codec is otherwise unit-tested standalone (env:native_coap).">CoAP</a></td>
  <td align="center"><a href="FEATURES.md#coap-block" title="CoAP block-wise transfer - RFC 7959 (requires COAP). Default off. When set, the server understands the Block2 (descriptive, responses) and Block1 (control, request uploads) options: - Block2: a representation larger than one block, or any GET that carries a Block2 option, is served one block at a time. A constrained client requests a small block size (SZX) and pages through with ascending block numbers; the server re-renders the (idempotent) resource and slices out the asked-for block, setting the More bit until the last. - Block1: a POST/PUT payload larger than one block is reassembled into a single BSS buffer. Each non-final block is acknowledged 2.31 Continue; the final block dispatches the handler with the whole reassembled payload. One block-wise transfer is reassembled at a time (deterministic, single buffer); an out-of-order or oversized block yields 4.08 / 4.13. Size1/Size2 options and the /.well-known/core listing are out of scope.">CoAP Block</a></td>
  <td align="center"><a href="FEATURES.md#coap-observe" title="CoAP resource observation - RFC 7641 (requires COAP). Default off. When set, a client GET with the Observe option registers as an observer of a resource; the application calls coap_notify(path) to push the resource's current representation to every observer (a CoAP notification from the server port with an increasing Observe sequence). Observers are dropped on a deregister GET, a client RST, or send failure.">CoAP Observe</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#cors" title="Cross-origin resource sharing with automatic preflight handling. Always on.">CORS</a></td>
  <td align="center"><a href="FEATURES.md#cotp" title="TPKT (RFC 1006) + COTP (ISO 8073 / X.224 class 0) frame codec - the &quot;ISO transport on TCP&quot; foundation under S7comm and IEC 61850 MMS. Default off. services/cotp provides `tpkt_build` / `tpkt_parse` for the 4-octet TPKT envelope (version 3 + 16-bit length), `cotp_build_dt` for a Data TPDU (`LI 0xF0 EOT|NR` + user data), `cotp_build_cr` for a Connection Request (destination/source refs, class 0, the TPDU-size parameter, plus caller TSAP parameters), and `cotp_parse` which reports the TPDU type and the DT user data or the CR/CC refs. Layout verified against RFC 1006 / ISO 8073; pure and host-tested. See src/services/cotp/cotp.h.">COTP</a></td>
  <td align="center"><a href="FEATURES.md#csrf" title="Opt-in CSRF protection for state-changing HTTP requests. Default off (zero cost / no behavior change). When set, every POST / PUT / PATCH / DELETE must carry a valid `X-CSRF-Token` header (a stateless, HMAC-signed token); requests without one get 403 Forbidden. GET / HEAD / OPTIONS are exempt (they are not state-changing). Clients fetch a token from the built-in `GET /csrf` endpoint, which also sets it as the `csrf` cookie. No server-side session storage - the token self-validates against an HMAC secret seeded from the hardware RNG at begin(); it is independent of AUTH.">CSRF</a></td>
  <td align="center"><a href="FEATURES.md#dashboard" title="Real-time SVG dashboard (DASHBOARD; requires SSE). Default off. Serves a self-contained, hand-rolled SVG dashboard page whose widgets are declared in a fixed compile-time DetwsWidget table (zero-heap, deterministic). The page fetches the widget layout as JSON and subscribes to an SSE stream of live values; detws_dashboard_set() + detws_dashboard_publish() push the current readings. The widget-table -&gt; JSON serializers are host-testable; WebSocket controls are a follow-up.">Dashboard</a></td>
  <td align="center"><a href="FEATURES.md#devicenet" title="DeviceNet link-adaptation codec. Default off. services/devicenet is the CAN-specific layer of &quot;CIP over CAN&quot;: `devicenet_encode_id` / `devicenet_decode_id` pack and unpack the 11-bit DeviceNet identifier as a Message Group (1..4) + Message ID + MAC ID (the four identifier ranges 0x000-0x3FF / 0x400-0x5FF / 0x600-0x7BF / 0x7C0-0x7EF), `devicenet_msg_header` / `devicenet_frag_octet` build the explicit-message header and fragmentation octets, `devicenet_build_explicit` emits a single-frame explicit message, and `devicenet_frag_feed` reassembles a fragmented body (first / middle / last, modulo-64 count) up to `DETWS_DEVICENET_MSG_MAX` octets. The CIP application layer (services / EPATH / data) is the same one EtherNet/IP uses, so build the message body with the existing cip codec (`DETWS_ENABLE_CIP`). Identifier allocation + fragmentation verified against the ODVA DeviceNet spec; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a DeviceNet segment onto Wi-Fi. See src/services/devicenet/devicenet.h.">DeviceNet</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#df1" title="Allen-Bradley DF1 full-duplex frame codec. Default off. services/df1 is a zero-heap framing + DLE byte-stuffing + BCC/CRC codec for the Rockwell serial PLC data-link layer (pub. 1770-6.5.16): `df1_build_frame` wraps application data in `DLE STX ... DLE ETX` (a data byte equal to DLE/0x10 is doubled) with either a BCC (the 2's complement of the modulo-256 data sum) or a CRC-16/ARC (poly 0x8005, init 0, over the data plus the ETX, sent low byte first), and `df1_parse_frame` validates the check and un-stuffs the data. Vectors verified against the manual (BCC 0x20-&gt;0xE0, CRC &quot;123456789&quot;-&gt;0xBB3D). Pure and host-tested; the PCCC application header lives inside the data. See src/services/df1/df1.h.">DF1</a></td>
  <td align="center"><a href="FEATURES.md#diag" title="Expose a diagnostic JSON endpoint via server.diag(). Disabled by default - enabling it exposes compile-time configuration (buffer sizes, feature flags) which could aid an attacker. Only enable in development or behind an authenticated route. When enabled, DETWS_DIAG_JSON is a compile-time string constant you can serve from any route handler:">Diag</a></td>
  <td align="center"><a href="FEATURES.md#dmx512" title="DMX512 + RDM (ANSI E1.20) lighting codec. Default off. services/dmx covers stage / architectural lighting over RS-485: `dmx_build` and `dmx_get_channel` assemble and read the positional DMX512 slot packet (a start code, 0x00 for dimmer data, followed by up to 512 channel slots - no checksum or in-frame addressing), and the RDM (Remote Device Management) functions build / parse the addressed management packet that shares the wire: `rdm_build` / `rdm_parse` carry 48-bit source / destination UIDs (`rdm_uid`), a transaction number, a command class (GET / SET / DISCOVERY) + parameter id, optional parameter data, and the 16-bit additive checksum (`rdm_checksum`), with named command-class / response-type / PID constants. RDM packet layout + checksum verified against ANSI E1.20; pure and host-tested. Drive a MAX485-class transceiver on a UART (250 kbit/s, 8N2; the break is the application's) and bridge a lighting rig onto Wi-Fi. See src/services/dmx/dmx.h.">DMX512</a></td>
  <td align="center"><a href="FEATURES.md#dnp3" title="DNP3 (IEEE 1815) data-link frame codec. Default off. services/dnp3 is a zero-heap builder + CRC-validating parser for the SCADA / utility outstation data-link layer: `dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC` header block plus the CRC'd 16-octet user-data blocks, and `dnp3_parse_frame` validates the header and every block CRC (CRC-16/DNP, verified against the canonical 0xEA82 check value) and de-blocks the user data. Addresses are little-endian. Pure codec, host-tested; the transport-function reassembly and the application layer (objects / function codes) layer on the de-blocked user data. See src/services/dnp3/dnp3.h.">DNP3</a></td>
  <td align="center"><a href="FEATURES.md#dns-server" title="Authoritative DNS server on UDP/53. Default off. services/dns_server answers A/IN queries from a small fixed table of `name -&gt; IPv4` records you register with `dns_server_add()`, so devices on an offline / air-gapped LAN can use names (`printer.lan`) instead of raw IPs - a companion to the NTP server for self-hosted infrastructure. It parses the question, looks the name up case-insensitively, and on a hit appends one answer record using DNS name compression (a 2-byte pointer back to the question); an unknown name returns NXDOMAIN and non-A queries return no answer. The response builder (`dns_server_build_response`) is pure and host-tested against the wire format; the binding is the transport UDP service. Zero heap. This is a general resolver, distinct from the provisioning captive-portal DNS (which points every name at the softAP) - do not enable both. Example 60.DnsServer. See src/services/dns_server/dns_server.h.">DNS Server</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#etag" title="Conditional GET via ETag for served files. When set, serve_file()/serve_static() emit a strong `ETag` (derived from the file size + last-modified time) and answer a matching `If-None-Match` with `304 Not Modified`, saving bandwidth on repeat fetches of static assets.">ETag</a></td>
  <td align="center"><a href="FEATURES.md#ethernetip" title="EtherNet/IP encapsulation codec - the ODVA EtherNet/IP transport (TCP/UDP 44818) that carries CIP. Default off. services/enip provides `eip_build` / `eip_parse` for the 24-octet encapsulation header (little-endian command / length / session handle / status / sender context / options), `eip_build_register_session` to open a session, and `eip_build_send_rr_data` / `eip_parse_send_rr_data` to wrap + unwrap a CIP message as an unconnected message via the Common Packet Format (a Null Address item plus an Unconnected Data item). Commands and CPF item types verified against the Wireshark ENIP dissector; pure and host-tested. The CIP object model inside the Unconnected Data item layers on top. See src/services/enip/enip.h.">EtherNet/IP</a></td>
  <td align="center"><a href="FEATURES.md#file-serving" title="Static file serving via Arduino FS (LittleFS, SPIFFS, SD).">File Serving</a></td>
  <td align="center"><a href="FEATURES.md#fins" title="Omron FINS frame codec. Default off. services/fins is a zero-heap command/response builder + parser for the Factory Interface Network Service (FINS/UDP): `fins_build_command` and `fins_build_memory_area_read` emit the 10-octet routing header (ICF/RSV/GCT, destination + source net/node/unit, SID) plus the MRC/SRC command code and parameters, and `fins_parse_command` / `fins_parse_response` read them back (the response MRES/SRES end code included). Talks to an Omron PLC over the shipped UDP transport (det_udp_sendto). Header layout verified against the FINS spec; pure and host-tested. See src/services/fins/fins.h.">FINS</a></td>
  <td align="center"><a href="FEATURES.md#flow-export" title="Flow-record export codec. Default off. services/flow_export is a zero-heap exporter-side codec for on-device flow accounting in three formats: NetFlow v5 (the fixed 24-octet header + 48-octet record, via `flow_v5_write_header` / `flow_v5_write_record`), NetFlow v9 (RFC 3954), and IPFIX (RFC 7011). The v9 / IPFIX side is a small cursor (`FlowWriter`): begin the message (`flow_ipfix_begin` / `flow_v9_begin`), emit a Template (`flow_export_template`), open a matching Data Set and append records (`flow_export_data_begin` / `flow_export_data_record` / `flow_export_data_end`), then `flow_export_finish()` patches the IPFIX message length or the NetFlow v9 record count (and pads each v9 FlowSet to a 4-octet boundary). Field offsets verified against RFC 7011 / RFC 3954 / the published v5 layout; pure and host-tested. The flow cache (5-tuple + counters) and the UDP send (det_udp_sendto) are the application's. See src/services/flow_export/flow_export.h.">Flow Export</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#graphql" title="GraphQL query subset. Default off. services/graphql parses a GraphQL query into a fixed AST node pool (no heap) and emits a `{&quot;data&quot;:{...}}` response shaped exactly by the requested selection. Schema-free: a field with a sub-selection is an object (the engine recurses), a leaf field calls your single resolver, and arguments collected along the path are handed to it. Supports nested selections, field arguments, and the anonymous / `query` forms; mutations, subscriptions, fragments, and variables are out of scope. Pure and host-tested; bounds are compile-time (DETWS_GQL_* in DetWebServerConfig.h). Serve it from a POST /graphql route.">GraphQL</a></td>
  <td align="center"><a href="FEATURES.md#grpc-web" title="gRPC-Web message framing. Default off. services/grpcweb is a zero-heap length-prefixed frame builder + parser for gRPC-Web, the HTTP/1.1-reachable subset of gRPC (gRPC proper needs HTTP/2): `grpcweb_frame_message` wraps a Protobuf message in the 5-octet `[flags][length BE32]` prefix, `grpcweb_frame_trailer` emits the 0x80 trailers frame (`grpc-status` / `grpc-message`), and `grpcweb_parse` reads one frame back (with `grpcweb_trailer_status` to pull the status out of a trailers frame). Wraps the Protobuf codec (DETWS_ENABLE_PROTOBUF) and rides the shipped HTTP/1.1 server/client. Pure and host-tested. See src/services/grpcweb/grpcweb.h.">gRPC-Web</a></td>
  <td align="center"><a href="FEATURES.md#host-link" title="Omron Host Link (C-mode) frame codec. Default off. services/hostlink is a zero-heap ASCII command/response codec for Omron's serial host-link protocol (the RS-232/485 sibling of FINS): `hostlink_build` emits `@UU` + 2-char header code + text + FCS + `*`CR, and `hostlink_parse` FCS-validates and splits a frame (`hostlink_end_code` reads a response's end code). The FCS is the 8-bit XOR from `@` through the text, rendered as two hex digits (verified against the `@00RD00000010` -&gt; `57` example). Pure and host-tested; the UART transport is the application's. See src/services/hostlink/hostlink.h.">Host Link</a></td>
  <td align="center"><a href="FEATURES.md#http-client" title="Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS). Default off. When set, src/services/http_client/http_client.h can issue a blocking GET/POST to a remote server: it resolves the host (DNS), opens a raw lwIP TCP connection (https:// goes through client-side mbedTLS over the same static arena as the server TLS), sends the request, and returns the status + body in caller buffers. For webhooks, telemetry push, REST calls from the device. The request builder + response parser are host-testable; the transport is ESP32-only.">HTTP Client</a></td>
  <td align="center"><a href="FEATURES.md#http-client-tls" title="HTTPS client support inside the HTTP client (needs TLS).">HTTP Client TLS</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#iec-60870" title="IEC 60870-5-101 / -104 telecontrol (SCADA) codec. Default off. services/iec60870 covers the utility-SCADA protocol in both transports: the -104 APCI over TCP (`iec104_build_i` / `_s` / `_u` and `iec104_parse` for the I / S / U formats - numbered information transfer with 15-bit send/receive sequence numbers, the supervisory acknowledge, and the STARTDT / STOPDT / TESTFR unnumbered commands), the shared ASDU header (`iec_asdu_build_header` / `iec_asdu_parse_header` - type id, variable structure qualifier, cause of transmission, common address) with the 3-octet Information Object Address (`iec_put_ioa` / `iec_get_ioa`), and the -101 FT1.2 serial link frames (`iec101_build_fixed` / `_variable` and `iec101_parse`, 8-bit sum checksum). Named type-id and cause-of-transmission constants are provided; the per-type information elements are the application's. Frame + APCI + ASDU layout verified against IEC 60870-5-101/-104; pure and host-tested. Run -104 over the shipped TCP stack or -101 over a UART / RS-485 transceiver to bridge an RTU or outstation onto Wi-Fi. See src/services/iec60870/iec60870.h.">IEC 60870</a></td>
  <td align="center"><a href="FEATURES.md#io-link" title="IO-Link (SDCI, IEC 61131-9) data-link message codec. Default off. services/iolink implements the point-to-point smart-sensor link's data-link message layer: `iol_mc` (with decoders) builds the M-sequence Control octet (read/write, communication channel, address), `iol_ckt` builds a master message's checksum / M-sequence-type octet, `iol_cks` builds a device reply's checksum / status octet (Event + PD-valid flags), and `iol_checksum6` / `iol_finalize` / `iol_verify` implement the SDCI message checksum straight from the IO-Link Interface and System Specification v1.1.4 Annex A.1.6 (a 0x52 seed XORed octet by octet with the check octet's checksum bits zeroed, then the 8-to-6-bit compression of equation A.1). The checksum is verified against a vector hand-derived from the spec formula. Lay the per-type M-sequence and ISDU octets out per your device profile, then finalize / verify with this codec. Pure and host-tested. The wire is a UART through an IO-Link transceiver (MAX14819 / L6360 class); bridge sensor data onto Wi-Fi. See src/services/iolink/iolink.h.">IO-Link</a></td>
  <td align="center"><a href="FEATURES.md#j1939" title="SAE J1939 codec. Default off. services/j1939 is a zero-heap codec for the heavy-duty-vehicle / agriculture / marine / genset CAN higher-layer protocol over 29-bit extended frames (`shared_primitives/det_can.h`): `j1939_encode_id` / `j1939_decode_id` pack and unpack the priority / PGN / source / destination identifier (both the PDU1 peer-to-peer and PDU2 broadcast forms), `j1939_build_message` emits single frames, `j1939_build_request` and `j1939_build_address_claim` (with `j1939_build_name` for the 64-bit NAME) handle the Request PGN and Address Claimed messages, and the Transport Protocol (BAM announce + TP.DT data packets) reassembles multi-packet messages up to `DETWS_J1939_TP_MAX` octets via `j1939_tp_feed`. Identifier layout, NAME bit fields, and the TP control bytes verified against SAE J1939-21 / -81; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a J1939 bus onto Wi-Fi. See src/services/j1939/j1939.h.">J1939</a></td>
  <td align="center"><a href="FEATURES.md#log-buffer" title="Opt-in fixed-RAM rotating log buffer with severity traps. Default off. When set, services/logbuf keeps the last DETWS_LOG_LINES log lines in a fixed ring (oldest pruned on overflow - no heap, bounded), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback when a line is logged at/above a severity threshold (forward criticals as an SNMP trap / webhook). The ring + trap logic is pure and host-tested.">Log-Buffer</a></td>
  <td align="center"><a href="FEATURES.md#lwm2m" title="OMA LwM2M TLV codec. Default off. services/lwm2m is a zero-heap writer + cursor reader for the LwM2M `application/vnd.oma.lwm2m+tlv` resource encoding carried over the shipped CoAP service for device management: `lwm2m_tlv_write` (with typed `lwm2m_tlv_write_int` shortest-form / `lwm2m_tlv_write_bool` / `lwm2m_tlv_write_string` / `lwm2m_tlv_write_float` helpers), `lwm2m_tlv_read`, and `lwm2m_tlv_value_int`. Handles 8-/16-bit identifiers, inline / 8- / 16- / 24-bit lengths, and the Object-Instance / Resource / Multiple-Resource / Resource-Instance kinds; type-byte layout verified against the LwM2M spec. Pure and host-tested. The registration interface and the standard object model layer on top. See src/services/lwm2m/lwm2m_tlv.h.">LwM2M</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#m-bus" title="Wired M-Bus (Meter-Bus, EN 13757) frame codec. Default off. services/mbus is a zero-heap builder + parser for the M-Bus link-layer frames used by utility meters (water / gas / heat / electricity): `mbus_build_ack` (the single-character 0xE5), `mbus_build_short` (`10 C A CS 16`), and `mbus_build_long` (`68 L L 68 C A CI ... CS 16`, with convenience `mbus_build_snd_nke` / `mbus_build_req_ud2`), plus `mbus_parse` which validates the start / stop octets, the doubled length, and the 8-bit sum checksum. `mbus_record_next` walks the EN 13757-3 variable-data records, skipping the DIFE / VIFE extension chains and decoding the data length from the DIF coding (incl. the LVAR variable form) so the app can read each value. Frame formats + checksum verified against EN 13757-2; pure and host-tested. Talk to the powered two-wire bus over a UART through an M-Bus level converter (a TSS721-based master) and bridge meter readings onto Wi-Fi. See src/services/mbus/mbus.h.">M-Bus</a></td>
  <td align="center"><a href="FEATURES.md#mdns" title="mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS.">MDNS</a></td>
  <td align="center"><a href="FEATURES.md#melsec" title="Mitsubishi MELSEC MC protocol (binary 3E frame) codec. Default off. services/melsec builds + parses the QnA-compatible binary 3E frames for MELSEC PLCs over TCP/UDP: `melsec_build_read` emits a batch-read (word units) request (little-endian fields, request subheader 0x5000, command 0x0401, a device code - D 0xA8 / M 0x90 / X / Y / R / ... - plus a 24-bit head device number and a point count), and `melsec_parse_response` validates the 0xD000 response and reports the end code (0x0000 = success) and the read data. Frame layout + device codes verified against a third-party MC implementation; pure and host-tested. Completes the major-vendor PLC read set alongside FINS / Host Link (Omron), DF1 (Allen-Bradley), and S7comm (Siemens). See src/services/melsec/melsec.h.">MELSEC</a></td>
  <td align="center"><a href="FEATURES.md#metrics" title="Prometheus `/metrics` endpoint (text exposition format 0.0.4). Default off (requires STATS for the underlying counters). When set, DetWebServer::metrics() emits the runtime stats as Prometheus metrics (`detws_uptime_seconds`, `detws_http_requests_total`, `detws_http_responses_total{class=...}`, `detws_active_connections`, `detws_free_heap_bytes`,...) so a Prometheus server can scrape the device.">Metrics</a></td>
  <td align="center"><a href="FEATURES.md#middleware" title="Composable use() pipeline with a fixed-window rate limiter. Always on.">Middleware</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#modbus" title="Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502. Default off. When set, listen(502, PROTO_MODBUS) serves a fixed data model (coils, discrete inputs, holding + input registers, all in BSS) over Modbus TCP: Read/Write Coils (FC 1/5/15), Read Discrete Inputs (FC 2), Read/Write Holding Registers (FC 3/6/16), and Read Input Registers (FC 4). The codec (MBAP framing + PDU dispatch) is pure and host-tested; the TCP transport is ESP32-only. The application reads/writes the model with the accessor functions and is notified of client writes via modbus_on_write(). Modbus has no authentication or encryption - run it only on a trusted control network.">Modbus</a></td>
  <td align="center"><a href="FEATURES.md#modbus-master" title="Opt-in Modbus master codec + register scanner. Default off. services/modbus/modbus_master builds Modbus TCP read-request ADUs and parses the responses (register values or exception), so an app can poll / auto-discover a slave's registers. Pure and host-tested as a full round-trip against the slave codec (modbus_process_adu); the actual send is the app's TCP.">Modbus Master</a></td>
  <td align="center"><a href="FEATURES.md#modbus-rtu" title="Modbus RTU framing (serial / RS-485). Default off; implies MODBUS. Adds the RTU ADU codec `modbus_rtu_process_adu()` - a `[slave addr][PDU][CRC16]` frame (CRC16-Modbus, little-endian) wrapped around the existing host-tested PDU dispatch: a CRC mismatch or a non-matching unit address is dropped silently (no reply, per the spec), and a broadcast (address 0) is processed with no reply. Pure and host-tested; feed it from a UART/RS-485 driver (the serial transport, framed by the 3.5-character inter-frame idle, is the application's). See src/services/modbus/modbus.h.">Modbus RTU</a></td>
  <td align="center"><a href="FEATURES.md#mqtt" title="MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS). Default off. When set, src/services/mqtt/mqtt.h provides a persistent outbound client: connect to a broker, PUBLISH (QoS 0/1/2) and SUBSCRIBE to topics, receive incoming messages via a callback, with keep-alive pings - the dominant IoT messaging pattern, for telemetry push and remote command. The packet codec is host-testable; the transport (DNS + raw lwIP TCP, MQTTS via client-side mbedTLS) is ESP32-only. Full QoS 0/1/2 (outbound DUP retransmit, inbound QoS-2 de-duplication by packet id) and Last-Will are supported.">MQTT</a></td>
  <td align="center"><a href="FEATURES.md#mqtt-sn" title="MQTT-SN v1.2 wire codec. Default off. services/mqtt/mqtt_sn is a zero-heap codec for MQTT for Sensor Networks - the UDP / non-TCP MQTT variant for constrained, lossy links (numeric topic IDs instead of strings, gateway discovery, sleeping-client keep-alive). Builders for CONNECT / REGISTER / PUBLISH / SUBSCRIBE (by name or pre-defined id) / PINGREQ / DISCONNECT / SEARCHGW, plus `mqttsn_parse_header()` (the 1- and 3-octet Length forms, big-endian fields) and typed parsers for CONNACK / REGACK / PUBACK / SUBACK / PUBLISH / REGISTER, with a `mqttsn_make_flags()` helper (DUP / QoS / retain / will / clean / TopicIdType). Wire bytes verified against the spec and the Eclipse Paho reference; pure and host-tested. The datagram send (det_udp_sendto), topic-ID registry, and sleep / retransmit state are the application's. See src/services/mqtt/mqtt_sn.h.">MQTT SN</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#mqtt-tls" title="MQTTS: run the MQTT client over client-side TLS (needs TLS).">MQTT TLS</a></td>
  <td align="center"><a href="FEATURES.md#nats" title="NATS client protocol codec - the text-based NATS pub/sub messaging protocol. Default off. services/nats lets a device be a NATS client over the outbound client transport: builders for `CONNECT`, `PUB` (with optional reply-to), `SUB` (with optional queue group), `UNSUB`, `PING`, and `PONG`, plus `nats_parse` which decodes an inbound `MSG` / `INFO` / `PING` / `PONG` / `+OK` / `-ERR` (a MSG yields subject / sid / reply-to / payload). Line-oriented (CRLF, space-delimited); only PUB and MSG carry a payload. Pure and host-tested; the connection and subscription state are the application's. See src/services/nats/nats.h.">NATS</a></td>
  <td align="center"><a href="FEATURES.md#nmea-0183" title="NMEA 0183 sentence codec. Default off. services/nmea0183 is a zero-heap codec for the marine / GPS ASCII protocol (sentences like `$GPGGA,123519,4807.038,N,...*47`): `nmea0183_build` emits a sentence (adding the `$`, the XOR checksum, and CR/LF), `nmea0183_checksum` computes the XOR check, `nmea0183_parse` validates the `*HH` checksum and splits the comma-separated fields (deriving the talker id + sentence type from the address field), and `nmea0183_field_float` / `nmea0183_field_int` decode field values (the field substrings are delimited so `det_strtof` / `det_strtol` stop cleanly). Sentence framing + checksum verified against the NMEA 0183 standard (the canonical GGA example checks to 0x47); pure and host-tested. GPS / marine receivers are cheap UART breakouts, so this is a plain HardwareSerial link; bridge position / wind / depth data onto Wi-Fi. See src/services/nmea0183/nmea0183.h.">NMEA 0183</a></td>
  <td align="center"><a href="FEATURES.md#nmea-2000" title="NMEA 2000 codec. Default off; implies J1939 (NMEA 2000 is J1939 at the transport layer). services/nmea2000 is a zero-heap codec for the marine instrumentation network over CAN: it reuses the J1939 29-bit identifier codec and adds the NMEA-specific Fast Packet transport. `n2k_fastpacket_num_frames` sizes a transfer, `n2k_fastpacket_build_frame` emits frame N of a 9..223-octet message (a control octet of sequence counter + frame counter, the first frame carrying the total length and 6 data octets, continuations carrying 7), and `n2k_fastpacket_feed` reassembles a sequence (matching source / PGN / sequence counter, rejecting out-of-order frames and ignoring interleaved sequences); `n2k_build_single` wraps a single-frame message. Fast Packet framing verified against the NMEA 2000 / J1939 layout; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge an NMEA 2000 backbone (GPS, wind, depth, engine PGNs) onto Wi-Fi. See src/services/nmea2000/nmea2000.h.">NMEA 2000</a></td>
  <td align="center"><a href="FEATURES.md#ntp" title="SNTP wall-clock time sync via the ESP-IDF SNTP client.">NTP</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#ntp-server" title="NTP/SNTP time server (RFC 5905 / RFC 4330 server mode) on UDP/123. Default off. services/ntp_server turns the device into a local time source: it answers client NTP requests from its own clock, so an offline or air-gapped LAN can keep its devices in sync without reaching the public pool. The 48-octet response builder (`ntp_server_build_response`) is pure - it echoes the request's protocol version, copies the client's transmit timestamp into the origin field (so the client can compute round-trip delay), and stamps reference/receive/transmit times - and is host-tested against the wire format. `ntp_server_begin(stratum, refid)` binds the port via the transport UDP service and drives it from `detws_time_now()` (seconds) plus a `detws_millis()`-derived sub-second fraction; while the device has no time it stays silent rather than serve a wrong clock. Pair it with a GPS receiver (parsed via the NMEA 0183 codec into a stratum-1 time source) and an upstream-NTP fallback for a self-hosted, offline-capable time server. Example 58.NtpServer. See src/services/ntp_server/ntp_server.h.">NTP Server</a></td>
  <td align="center"><a href="FEATURES.md#oauth2" title="OAuth2 token-endpoint client. Default off. services/oauth2 obtains tokens - the counterpart to the OIDC ID-token verifier. It builds the percent-encoded form body for the authorization_code and refresh_token grants (RFC 6749), supporting a confidential client (client_secret) or a public client with PKCE (code_verifier, RFC 7636), and parses the JSON token response (reusing the zero-heap JSON reader). The build + parse core is pure and host-tested; the POST to the token endpoint uses the HTTP(S) client (needs HTTP_CLIENT). No heap, no stdlib.">OAuth2</a></td>
  <td align="center"><a href="FEATURES.md#observability" title="Transport-layer observability: connection-event hook + counters. Default off (zero cost when unset - the notify points compile to nothing). When set, the transport (L4) fires an application callback on every connection state transition - `det_conn_on_event(slot, old_state, new_state, reason)` - and maintains lock-free counters (accepts, closes by reason, idle timeouts, RX backpressure events, dropped deferred events, and a live CONN_CLOSING gauge) readable via `det_conn_counters()`. The only state-transition trace the L4/L5 core exposes; pair it with STATS for request-level metrics.">Observability</a></td>
  <td align="center"><a href="FEATURES.md#oidc" title="OpenID Connect ID-token verification, RS256. Default off. services/oidc verifies an OIDC ID token (JWT) as a relying party: requires alg RS256, selects the issuer key by kid from a JWKS, verifies the RSASSA-PKCS1-v1.5 SHA-256 signature (real RSA modexp via ssh_rsa, mbedTLS- accelerated on ESP32), and checks iss / aud / exp / nbf, extracting sub / email. Pure and host-tested; the caller fetches + caches the JWKS over HTTPS (off the request hot path) and passes the JSON in. Builds on the SSH RSA primitive, not the HS256 JWT module (services/jwt), so the two are independent.">OIDC</a></td>
  <td align="center"><a href="FEATURES.md#opc-ua" title="OPC UA Binary server. Default off. services/opcua provides an OPC UA (IEC 62541) Binary server: the little-endian built-in-type codec (incl. NodeId / ExtensionObject / DateTime / Variant / DataValue / ReferenceDescription), UA-TCP (UACP) message framing, the Hello/Acknowledge handshake, the SecureChannel (OpenSecureChannel, SecurityPolicy None), the Session (CreateSession + ActivateSession), GetEndpoints, the Read, Write and Browse services (registered resolvers map a NodeId to a value / accept a written value / list child references), plus CloseSession + CloseSecureChannel and a ServiceFault for unsupported services, served on TCP via PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`). The MSG framing is spec-faithful (incl. SecureChannelId), so standard clients interoperate (verified with python asyncua: connect + browse + read + write/read-back). All pure and host-tested. No heap, no stdlib.">OPC-UA</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#opc-ua-client" title="OPC UA Binary client. Default off (requires OPC-UA, shares the codec). services/opcua_client builds the client-side requests (Hello, OpenSecureChannel, CreateSession, ActivateSession, Read, Browse, CloseSession, CloseSecureChannel) and parses the server responses, reusing the opcua.h codec. Transport-agnostic - the app supplies the outbound socket (e.g. an Arduino WiFiClient). No heap, no stdlib.">OPC-UA Client</a></td>
  <td align="center"><a href="FEATURES.md#ota" title="Authenticated OTA firmware update (streaming POST to the ESP32 Update API).">OTA</a></td>
  <td align="center"><a href="FEATURES.md#ota-rollback" title="Opt-in OTA rollback protection / soft-brick safeguard. Default off. After an OTA update the new image boots in PENDING_VERIFY; this service confirms it (esp_ota_mark_app_valid) once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses without success - so a bad update self-heals instead of soft-bricking. The decision logic is pure and host-tested; the commit / rollback use esp_ota_ops. Requires the bootloader's app-rollback support (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE).">OTA Rollback</a></td>
  <td align="center"><a href="FEATURES.md#partition-monitor" title="Opt-in flash partition-map monitor endpoint. Default off. When set, services/partition_monitor reports the device's flash partition table (label, kind, type / subtype, offset, size, and which app slot is running) as JSON, for diagnostics and OTA dashboards. The partition walk uses esp_partition / esp_ota_ops; the JSON serializer and the kind classifier are pure and host-testable.">Partition Monitor</a></td>
  <td align="center"><a href="FEATURES.md#provisioning" title="First-boot WiFi provisioning: softAP + captive-portal credentials form.">Provisioning</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#range" title="HTTP Range requests / 206 Partial Content for served files. Default off. When set (requires FILE_SERVING), serve_file() / serve_static() honor a single-range `Range: bytes=...` request header: they answer `206 Partial Content` with a `Content-Range` header and stream only the requested bytes (seeking the file to the start offset), advertise `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range with `416 Range Not Satisfiable`. This enables resumable downloads and media seeking. Multi-range (multipart/byteranges) requests are not supported - the server falls back to a full 200 response, which is RFC 7233 §3.1 compliant.">Range</a></td>
  <td align="center"><a href="FEATURES.md#redis" title="Redis RESP2 wire codec. Default off. services/redis_resp lets a device drive a Redis server over the shipped outbound client transport: `resp_encode_command` builds a command (an array of bulk strings, binary-safe via explicit arg lengths) and `resp_parse` is a cursor reply decoder (simple / error / integer / bulk / array / nil). Pure and host-tested; the connection is the application's. See src/services/redis_resp.h.">Redis</a></td>
  <td align="center"><a href="FEATURES.md#routing" title="Exact, wildcard (/*), :param path parameters, bounded allocation-free regex routes, and per-interface STA/softAP route filters. Always on.">Routing</a></td>
  <td align="center"><a href="FEATURES.md#s7comm" title="Siemens S7comm PDU codec. Default off. services/s7comm builds + parses the S7-300/400 communication PDUs carried inside a COTP Data TPDU (DETWS_ENABLE_COTP) over ISO-on-TCP (port 102): `s7_build_setup` (Setup Communication), `s7_build_read_request` (Read Var with S7-ANY items over the DB / I / Q / M areas, encoding the byte address as a 24-bit bit-address), `s7_parse_header`, and `s7_read_next_item` which walks the response data items honoring the length-in-bits transport sizes (BIT/BYTE/INT) and the even-item padding. All constants (protocol id 0x32, ROSCTR, function/area/transport codes) are verified against the Wireshark S7comm dissector. Pure and host-tested; wrap the PDU with `cotp_build_dt` + `tpkt_build`. See src/services/s7comm/s7comm.h.">S7comm</a></td>
  <td align="center"><a href="FEATURES.md#sdi-12" title="SDI-12 sensor-bus codec. Default off. services/sdi12 is a zero-heap command / response codec for the 1200-baud single-wire ASCII bus used by environmental / agricultural sensors (soil moisture, water level, weather). `sdi12_build` and the `sdi12_build_measure` / `_concurrent` / `_data` / `_identify` / `_ack` / `_change_address` / `_query_address` helpers emit the standard `&lt;addr&gt;&lt;command&gt;!` requests; `sdi12_parse_measure` reads the `atttn` measurement response (seconds-until-ready + value count, both the 1-digit `aM!` and 2-digit `aC!` forms); `sdi12_parse_values` splits a data response into floats; and `sdi12_crc16` / `sdi12_crc_encode` / `sdi12_check_crc` implement the SDI-12 CRC (poly 0xA001, encoded as 3 printable octets) for the CRC-protected `aMC!` / `aCC!` variants. Command set + CRC verified against the SDI-12 specification; pure and host-tested. Drive the single 1200-baud line over a UART and bridge sensor readings onto Wi-Fi. See src/services/sdi12/sdi12.h.">SDI-12</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#smtp" title="Outbound SMTP client (RFC 5321) for device email alerts. Default off. services/smtp runs a blocking one-shot send over the shared outbound client transport (det_client): read the greeting, EHLO, optional AUTH LOGIN (username/password base64-encoded), then MAIL FROM / RCPT TO / DATA the message and QUIT. The body is dot-stuffed (RFC 5321 sec 4.5.2) and CRLF-normalized so it can never end early, and the whole exchange is zero-heap (fixed DETWS_SMTP_* buffers). Implicit TLS (SMTPS, typically port 465) is used when the message config sets tls and DETWS_ENABLE_TLS is on. The dialogue engine (smtp_run) is written against a send/recv seam, so the full protocol - reply codes, AUTH, dot-stuffing, the terminating dot - is host-tested with a scripted mock server (env:native_smtp). SMS fallback needs no extra code (an email-to-SMS carrier gateway address). Example 57.SmtpAlert ships a from-scratch beginner walkthrough (including standing up a Postfix server). See src/services/smtp/smtp.h.">SMTP</a></td>
  <td align="center"><a href="FEATURES.md#snmp" title="SNMP agent (v1/v2c, + v3 USM when SNMP_V3) over lwIP UDP. Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER codec itself is gated by this flag and is otherwise unit-tested standalone (env:native_snmp).">SNMP</a></td>
  <td align="center"><a href="FEATURES.md#snmp-trap" title="Outbound SNMP notifications - traps and informs (requires SNMP). Default off. When set, src/services/snmp/snmp_notify.h sends SNMPv2c (and, with SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over UDP - so the agent can push alerts instead of only answering polls. Reuses the BER codec and the transport-layer UDP service; the PDU builder is host-testable.">SNMP Trap</a></td>
  <td align="center"><a href="FEATURES.md#snmp-v3" title="Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off.">SNMP V3</a></td>
  <td align="center"><a href="FEATURES.md#sparkplug" title="Sparkplug B payload + topic codec - the Eclipse Sparkplug B industrial-IoT format over MQTT. Default off; implies PROTOBUF (the payload is a Protobuf message). services/sparkplug builds the topic (`spb_build_topic` -&gt; `spBv1.0/group/type/node[/device]`) and serializes the payload over the protobuf codec: `spb_build_metric` emits one Tahu Metric (name / alias / timestamp / datatype + a value - int / long / float / double / boolean / string), and `spb_build_payload` wraps the timestamp + metrics + seq. Field numbers and datatype codes are verified against the Eclipse Tahu sparkplug_b.proto. Pure and host-tested; publish the payload with the MQTT client. See src/services/sparkplug/sparkplug.h.">Sparkplug</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#stats" title="Runtime stats endpoint (uptime, request/error counts, pool usage, heap).">Stats</a></td>
  <td align="center"><a href="FEATURES.md#statsd" title="StatsD metrics client - push counters/gauges/timings/sets to a StatsD collector. Default off. services/statsd emits metrics in the StatsD wire format (`name:value|type`, e.g. `api.hits:1|c`) over UDP via det_udp_sendto to any StatsD-speaking backend (Graphite/StatsD, Telegraf, Datadog, InfluxDB), with counters (`c`), gauges (`g`, absolute or a signed `+`/`-` delta), timings (`ms`), and sets (`s`), plus an optional sample rate (`|@0.1`) and DogStatsD tags (`|#env:prod`). This is the push counterpart to the pull-based Prometheus `/metrics` endpoint - useful behind NAT/firewalls where nothing can reach in to scrape. The value/rate are rendered without printf float/64-bit formatting; the line builder (`statsd_format`) is pure and host-tested, and the emit helpers are host-tested through the transport UDP capture seam. Zero heap. Example 59.StatsdMetrics. See src/services/statsd/statsd.h.">StatsD</a></td>
  <td align="center"><a href="FEATURES.md#stomp" title="STOMP 1.2 frame codec. Default off. services/stomp is a zero-heap codec for the Simple/Streaming Text Oriented Messaging Protocol: `stomp_build_frame()` writes a frame (command + escaped `key:value` headers + blank line + NUL-terminated body) and `stomp_parse_frame()` is a non-mutating cursor reporting the command, header key/value slices, and body (honoring `content-length`, tolerating `\r\n` line endings, and skipping broker heart-beats), with `stomp_header()` lookup and `stomp_unescape()` for the header escapes (`\r` `\n` `\c` `\\`). Drives CONNECT / SEND / SUBSCRIBE / MESSAGE / ACK against a broker (ActiveMQ / RabbitMQ / Artemis) over the shipped outbound client transport, or STOMP-over-WebSocket via the WS client. Pure and host-tested; the connection and subscription state are the application's. See src/services/stomp.h.">Stomp</a></td>
  <td align="center"><a href="FEATURES.md#sunspec" title="SunSpec Modbus device-information-model codec. Default off. services/sunspec is a zero-heap codec for the SunSpec Alliance register maps layered on the holding-register model: a model-chain walker (`sunspec_check_marker` / `sunspec_begin` / `sunspec_next_model` - verify the `SunS` 0x53756E53 marker, then iterate each model's id / length / body to the 0xFFFF end model) plus typed point readers (`sunspec_u16`, `sunspec_i16`, `sunspec_u32`, `sunspec_i32`, `sunspec_string`), and a map writer (`sunspec_write_marker`, `sunspec_write_model_header`, the point writers, `sunspec_write_end_model`) for a device exposing its own map. Makes a solar inverter / meter / battery interoperable. Marker and header format verified against the SunSpec spec; pure and host-tested. Pairs with the Modbus service. See src/services/sunspec/sunspec.h.">SunSpec</a></td>
  <td align="center"><a href="FEATURES.md#syslog" title="Syslog client (RFC 5424 over UDP). Default off. When set, the device can ship log lines to a remote syslog server (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the transport-layer UDP service - a zero-heap structured-logging sink for fleets of constrained devices. See src/services/syslog/syslog.h.">Syslog</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#telemetry" title="Telemetry math helpers (moving-window stats, rate-of-change, totalizer). Default off. When set, src/services/telemetry/telemetry.h provides zero-heap pure-computation helpers over caller-supplied storage: a moving-window stats accumulator (mean / variance / stddev / min / max), a derivative / rate-of- change tracker, and a trapezoidal run-time totalizer. No ESP32 dependency, so the whole cluster is host-testable; it feeds dashboards, alert triggers, and odometer-style counters.">Telemetry</a></td>
  <td align="center"><a href="FEATURES.md#templating" title="{{var}} response templating via send_template(). Always on.">Templating</a></td>
  <td align="center"><a href="FEATURES.md#totp" title="Opt-in TOTP two-factor auth (RFC 6238). Default off. services/totp computes and verifies time-based one-time passwords (HMAC-SHA1 over the existing SHA-1, Google Authenticator compatible) and decodes base32 shared secrets, for a second factor on top of password / JWT auth. Pure and host-tested against the RFC 6238 vectors; the verifier checks a +/- step window for clock skew.">TOTP</a></td>
  <td align="center"><a href="FEATURES.md#udp-telemetry" title="Opt-in fire-and-forget UDP telemetry cast. Default off. When set, services/udp_telemetry casts metric lines (InfluxDB line protocol: `measurement field=val,field2=val2`) to a configured collector over UDP via det_udp_sendto - zero-heap, fire-and-forget (no ACK, no retry), ideal for shipping device metrics to Telegraf/InfluxDB/a log sink. The line builder is pure and host-tested; only the send touches the network.">UDP Telemetry</a></td>
  <td align="center"><a href="FEATURES.md#upload" title="Streaming file upload: POST a body straight to a file on the filesystem. Default off. When set, src/services/upload_service.h registers a POST route that streams the request body directly into an Arduino FS file (LittleFS / SPIFFS / SD) - the upload never has to fit in RAM. Reuses the same parser streaming-body hook as OTA. For reliable uploads set RX_BUF_SIZE above the largest inbound TCP segment (TCP_MSS, ~1460): the transport refuses-and-redelivers a segment that will not fit the receive ring (lossless backpressure), but a ring smaller than one segment would stall. The 1024 default suits ordinary requests, not uploads.">Upload</a></td>
</tr>
<tr>
  <td align="center"><a href="FEATURES.md#wamp" title="WAMP messaging codec. Default off. services/wamp is a zero-heap codec for the Web Application Messaging Protocol (unified RPC + PubSub over WebSocket, subprotocol `wamp.2.json`): builders for HELLO / SUBSCRIBE / UNSUBSCRIBE / PUBLISH / CALL / REGISTER / YIELD / GOODBYE (JSON arrays emitted via the shared JsonWriter - Options/Details default to `{}`, Arguments / ArgumentsKw are passed as JSON literals), plus a nesting-aware positional parser (`wamp_get_type`, `wamp_get_uint`, `wamp_get_uri`, `wamp_element`) that pulls the message type, ids, and URIs out of an inbound WELCOME / SUBSCRIBED / EVENT / RESULT / INVOCATION / ERROR. Message codes verified against the WAMP spec; pure and host-tested. It rides the shipped WebSocket layer; the session / subscription / registration tables are the application's. See src/services/wamp/wamp.h.">WAMP</a></td>
  <td align="center"><a href="FEATURES.md#webdav" title="WebDAV server (RFC 4918, class 1 + advisory locks) over the file system. Default off. When set (requires FILE_SERVING), dav() mounts an FS subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1), PROPPATCH, GET, HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK - so a client (rclone, cadaver, curl, or a mounted network drive) can browse and edit files. PROPFIND returns a 207 Multi-Status document built into a fixed buffer (DETWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at DETWS_WEBDAV_MAX_ENTRIES children. PROPPATCH returns a 207 with each requested property refused 403 Forbidden (read-only live properties, no dead-property store) so Explorer/Finder - which PROPPATCH a timestamp right after a PUT - do not error on a 405. PUT streams the request body straight to the file (via the shared streaming-body sink), so an upload is not bounded by BODY_BUF_SIZE. Locks are advisory (a synthetic token is issued but not enforced). See docs/SECURITY.md before exposing it.">WebDAV</a></td>
  <td align="center"><a href="FEATURES.md#webhook" title="Opt-in outbound webhooks / IFTTT. Default off. Requires HTTP_CLIENT. services/webhook builds an IFTTT Maker URL and a value1/value2/value3 JSON payload (pure, host-tested) and fires them - or any JSON to any URL - via the outbound http_client (POST). Use it to push an event from the device to IFTTT, a Slack/Discord hook, or your own API.">Webhook</a></td>
  <td align="center"><a href="FEATURES.md#ws-client" title="Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS). Default off. When set, src/services/ws_client/ws_client.h connects to a remote WebSocket endpoint (ws://, or wss:// over client-side mbedTLS), performs the RFC 6455 client handshake (Sec-WebSocket-Key/Accept), and sends masked text / binary frames + receives server frames via a callback - for streaming to cloud dashboards or bidirectional control. The frame/handshake codec is host-testable.">WS Client</a></td>
  <td align="center"><a href="FEATURES.md#ws-client-tls" title="wss://: run the WebSocket client over client-side TLS (needs TLS).">WS Client TLS</a></td>
</tr>
</tbody>
</table>

<!-- END GENERATED FEATURE TABLES -->


## Build Footprint

Measured on `esp32dev` (Arduino core, `pio ci`). The baseline → server jump is almost entirely the WiFi/lwIP stack; the library and most HTTP features add little on top. Each indented row enables one optional subsystem over the default server.

| Build                                                                               | Flash (bytes) | RAM (bytes) |
| ----------------------------------------------------------------------------------- | ------------: | ----------: |
| Empty sketch (no WiFi, no library) - _RTOS/Arduino baseline_                        |       233,257 |      21,032 |
| Minimal REST server (WS/SSE/multipart/file/auth stripped)                           |       734,745 |      57,936 |
| **Default server** (HTTP + WebSocket + SSE + multipart + file serving + Basic auth) |       745,133 |      64,264 |
| &nbsp;&nbsp;+ HTTPS / TLS (static-pool mbedTLS)                                     |       847,185 |     115,164 |
| &nbsp;&nbsp;+ SSH 2.0 server                                                        |       798,005 |      76,556 |
| &nbsp;&nbsp;+ SNMP agent (v1/v2c)                                                   |       751,277 |      76,648 |
| &nbsp;&nbsp;+ CoAP server (RFC 7252, UDP)                                           |       747,921 |      66,760 |
| &nbsp;&nbsp;+ mDNS                                                                  |       768,037 |      66,160 |
| &nbsp;&nbsp;+ SNTP                                                                  |       768,861 |      66,808 |
| &nbsp;&nbsp;+ OTA update                                                            |       748,417 |      64,544 |
| &nbsp;&nbsp;+ Captive-portal provisioning                                           |       750,709 |      65,836 |
| &nbsp;&nbsp;+ Static files via LittleFS (incl. ETag)                                |       784,361 |      64,288 |
| &nbsp;&nbsp;+ Telnet console                                                        |       745,137 |      64,784 |
| &nbsp;&nbsp;+ Web terminal (WebSocket)                                              |       747,613 |      64,336 |
| SSH crypto self-test (Serial only, no WiFi)                                         |       269,585 |      21,476 |

TLS's larger RAM is the fixed mbedTLS arena ([`DETWS_TLS_ARENA_SIZE`](@ref DETWS_TLS_ARENA_SIZE), 48 KB default). Small HTTP features (CORS, JSON, middleware, regex / path / form params, templating, chunked, response headers, Digest auth, stats, diagnostics, accept-throttle) stay within a few KB of the default server. The outbound HTTP client ([`DETWS_ENABLE_HTTP_CLIENT`](@ref DETWS_ENABLE_HTTP_CLIENT)) links no code unless a sketch actually calls [`http_get()`](@ref http_get) / [`http_post()`](@ref http_post); the standalone client example builds to 732,961 B flash / 46,752 B RAM, and adding `https://` (which pulls in mbedTLS) makes it 827,853 B / 100,620 B. The MQTT client ([`DETWS_ENABLE_MQTT`](@ref DETWS_ENABLE_MQTT)) example builds to 734,293 B flash / 48,896 B RAM; `mqtts://` makes it 830,285 B / 108,732 B. The WebSocket client ([`DETWS_ENABLE_WS_CLIENT`](@ref DETWS_ENABLE_WS_CLIENT)) example builds to 734,329 B / 48,824 B; `wss://` makes it 830,165 B / 108,660 B. ESP32 capacity: 1,310,720 B flash / 327,680 B RAM.

## Installation

**PlatformIO:**

```ini
lib_deps = https://github.com/dstroy0/DeterministicESPAsyncWebServer.git
```

**Arduino IDE:** Download the repository as a ZIP and use _Sketch → Include Library → Add .ZIP Library_.

## Quick Start

```cpp
#include <WiFi.h>
#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"

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

See `examples/Foundation/05.Configuration/05.Configuration.ino` for a full reference of every configurable flag and constant.

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
L7  src/DeterministicESPAsyncWebServer.h/cpp     Route table, dispatch, send()
L6  src/network_drivers/presentation/
        presentation.h/cpp                        Drains ring buffer → parser
        http_parser.h/cpp                         RFC 7230 byte-stream state machine
        sha1.h/cpp  base64.h/cpp                  mbedTLS hardware-accelerated helpers
        websocket.h/cpp  sse.h/cpp                WS frame parser; SSE connection pool
        multipart.h/cpp                           Multipart form-data parser
L5  src/network_drivers/session/
        session.h/cpp                             FreeRTOS event queue drain
L4  src/network_drivers/transport/
        transport.h/cpp                           lwIP callbacks, ring buffers, timeouts
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

(Representative, not exhaustive - the full file set is under `src/`; each optional
service is gated by a `DETWS_ENABLE_*` flag.)

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

Here are the available compile-time feature flags and their default values:

| Flag                                                          | Default | Description                                                   |
| :------------------------------------------------------------ | :------ | :------------------------------------------------------------ |
| [`DETWS_ENABLE_WEBSOCKET`](@ref DETWS_ENABLE_WEBSOCKET)       | `1`     | WebSocket support (RFC 6455, SHA-1/base64 via mbedTLS)        |
| [`DETWS_ENABLE_SSE`](@ref DETWS_ENABLE_SSE)                   | `1`     | Server-Sent Events push support                               |
| [`DETWS_ENABLE_MULTIPART`](@ref DETWS_ENABLE_MULTIPART)       | `1`     | `multipart/form-data` body parser                             |
| [`DETWS_ENABLE_FILE_SERVING`](@ref DETWS_ENABLE_FILE_SERVING) | `1`     | Static file serving via Arduino `FS`                          |
| [`DETWS_ENABLE_AUTH`](@ref DETWS_ENABLE_AUTH)                 | `1`     | HTTP Basic Auth per-route                                     |
| `DETWS_ENABLE_DIAG`                                           | `0`     | JSON build-config diagnostic endpoint (disable in production) |
| [`DETWS_ENABLE_MDNS`](@ref DETWS_ENABLE_MDNS)                 | `0`     | mDNS/DNS-SD advertisement via ESPmDNS                         |
| [`DETWS_ENABLE_NTP`](@ref DETWS_ENABLE_NTP)                   | `0`     | SNTP wall-clock time synchronization                          |
| [`DETWS_ENABLE_OTA`](@ref DETWS_ENABLE_OTA)                   | `0`     | Authenticated OTA firmware updates                            |
| [`DETWS_ENABLE_PROVISIONING`](@ref DETWS_ENABLE_PROVISIONING) | `0`     | WiFi provisioning wizard (SoftAP + captive portal)            |
| [`DETWS_ENABLE_TELNET`](@ref DETWS_ENABLE_TELNET)             | `0`     | RFC 854 Telnet server                                         |
| [`DETWS_ENABLE_SSH`](@ref DETWS_ENABLE_SSH)                   | `0`     | RFC 4253/4252/4254 SSH server                                 |

Illegal combinations (e.g. `MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS`) produce `#error` messages at compile time with a descriptive reason string.

## Configuration Overrides

All constants can be overridden using compiler build flags (e.g. `-DMAX_CONNS=6`). Default limits and sizes reside in [DetWebServerConfig.h](@ref DetWebServerConfig.h).

<details>
<summary><b>Expand Configuration constants and options</b></summary>

**Capacity**

| Constant                                    | Default | Description                                           |
| ------------------------------------------- | ------- | ----------------------------------------------------- |
| [`MAX_CONNS`](@ref MAX_CONNS)               | 4       | Simultaneous TCP connections (1–255)                  |
| [`EVT_QUEUE_DEPTH`](@ref EVT_QUEUE_DEPTH)   | 16      | FreeRTOS event queue depth; must be ≥ `MAX_CONNS * 4` |
| [`RX_BUF_SIZE`](@ref RX_BUF_SIZE)           | 1024    | Ring buffer bytes per connection                      |
| [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE)       | 256     | Request body bytes; must be ≤ `RX_BUF_SIZE`           |
| [`MAX_ROUTES`](@ref MAX_ROUTES)             | 16      | Registered route handlers                             |
| [`MAX_HEADERS`](@ref MAX_HEADERS)           | 8       | Headers stored per request                            |
| [`MAX_PATH_LEN`](@ref MAX_PATH_LEN)         | 64      | URL path bytes including leading `/`                  |
| [`MAX_KEY_LEN`](@ref MAX_KEY_LEN)           | 32      | Header field-name bytes                               |
| [`MAX_VAL_LEN`](@ref MAX_VAL_LEN)           | 48      | Header field-value bytes                              |
| [`MAX_QUERY_LEN`](@ref MAX_QUERY_LEN)       | 128     | Raw query string bytes (after `?`)                    |
| [`MAX_QUERY_PARAMS`](@ref MAX_QUERY_PARAMS) | 8       | Parsed query key=value pairs                          |
| [`QUERY_KEY_LEN`](@ref QUERY_KEY_LEN)       | 24      | Query parameter key bytes                             |
| [`QUERY_VAL_LEN`](@ref QUERY_VAL_LEN)       | 48      | Query parameter value bytes                           |

**Response Buffers**

| Constant                                      | Default | Minimum | Description                                                           |
| --------------------------------------------- | ------- | ------- | --------------------------------------------------------------------- |
| [`RESP_HDR_BUF_SIZE`](@ref RESP_HDR_BUF_SIZE) | 768     | 128     | Stack buffer for HTTP response headers                                |
| [`WS_HDR_BUF_SIZE`](@ref WS_HDR_BUF_SIZE)     | 256     | 128     | Stack buffer for WebSocket 101 response                               |
| [`CORS_HDR_BUF_SIZE`](@ref CORS_HDR_BUF_SIZE) | 192     | 64      | Buffer for pre-built CORS header block; must be ≤ `RESP_HDR_BUF_SIZE` |

**WebSocket (DETWS_ENABLE_WEBSOCKET)**

| Constant                              | Default | Description                                         |
| ------------------------------------- | ------- | --------------------------------------------------- |
| [`MAX_WS_CONNS`](@ref MAX_WS_CONNS)   | 2       | WebSocket slots; each consumes one `MAX_CONNS` slot |
| [`WS_FRAME_SIZE`](@ref WS_FRAME_SIZE) | 512     | Max WebSocket frame payload bytes                   |

**SSE (DETWS_ENABLE_SSE)**

| Constant                              | Default | Description                                   |
| ------------------------------------- | ------- | --------------------------------------------- |
| [`MAX_SSE_CONNS`](@ref MAX_SSE_CONNS) | 2       | SSE slots; each consumes one `MAX_CONNS` slot |
| [`SSE_BUF_SIZE`](@ref SSE_BUF_SIZE)   | 256     | Stack buffer for one formatted SSE event      |

**File Serving (DETWS_ENABLE_FILE_SERVING)**

| Constant                                  | Default | Description                                                        |
| ----------------------------------------- | ------- | ------------------------------------------------------------------ |
| [`FILE_CHUNK_SIZE`](@ref FILE_CHUNK_SIZE) | 512     | Bytes read from FS per `tcp_write()` call; must be ≤ `RX_BUF_SIZE` |

**Auth (DETWS_ENABLE_AUTH)**

| Constant                            | Default | Description                                               |
| ----------------------------------- | ------- | --------------------------------------------------------- |
| [`MAX_AUTH_LEN`](@ref MAX_AUTH_LEN) | 32      | Max username or password length including null terminator |

**Multipart (DETWS_ENABLE_MULTIPART)**

| Constant                                    | Default | Description                |
| ------------------------------------------- | ------- | -------------------------- |
| `MAX_MULTIPART_PARTS`                       | 4       | Max form parts per request |
| [`MAX_BOUNDARY_LEN`](@ref MAX_BOUNDARY_LEN) | 72      | Max MIME boundary length   |

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
| `begin(port, cfg = nullptr)`            | Bind and listen. Returns `+1` on success, `-1` on lwIP error.                   |
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

The HTTP/1.1 parser enforces RFC 7230 byte-by-byte; the dispatcher returns the
correct status codes (400/404/405/413/414/426/501) with `Allow`/`Sec-WebSocket-Version`
headers where required; the WebSocket layer enforces RFC 6455 framing rules.

See **[RFC.md](RFC.md)** for the full conformance tables (HTTP, WebSocket,
automatic error responses).

## SSH Support

DeterministicESPAsyncWebServer includes a **complete SSH-2.0 server protocol** -
banner exchange → `KEXINIT` negotiation → DH-group14-SHA256 key exchange →
`NEWKEYS` → user authentication (**publickey** and password) → `ssh-connection`
session channel, with transparent in-session re-keys. All state is static (BSS),
the RSA private key never touches static memory, and password auth can be
compiled out (`DETWS_SSH_ALLOW_PASSWORD=0`) for publickey-only hardening.

See **[SSH.md](SSH.md)** for the feature summary, RFC/FIPS compliance
table, authentication/hardening details, and memory footprint, and
**[SECURITY.md](SECURITY.md)** for the security treatment.

## Memory Table

<details>
<summary><b>Memory Usage Breakdown Table</b></summary>

```
Feature            Pool / symbol                    Size (bytes)    Notes
────────────────── ──────────────────────────────── ─────────────── ──────────────────────────
Transport          conn_pool[4 × (1024 + 22)]             4,184     Ring bufs + TCP state
                   listener_pool[3 × ~218]                  654     Per-port listener state
                   _queue_storage[16 × sizeof(TcpEvt)]      384     FreeRTOS queue backing
                   _queue_struct (StaticQueue_t)             ~88     FreeRTOS struct

HTTP               http_pool[4 × ~1,667]                  6,668     HttpReq + body buf

WebSocket          ws_pool[2 × (512 + 29)]                1,082     WS frame + state

SSE                sse_pool[2 × (64 + 3)]                   134     SSE path + state

SSH                ssh_pkt[1 × ~1,034]                    1,034     Pkt state + RX buf
                   ssh_keys[1 × ~240]                       240     AES CTR ctx + MAC keys
                   ssh_dh[1 × ~800]                          800     DH scalars + H
                   crypto_work[1536]                        1,536     Bignum scratch (zeroed)
                   group14_p + group14_g                     512     RFC 3526 constants
                   ssh_host_pubkey                            260     RSA-2048 public key

Application        _routes[16 × ~32]                        512     Route table

────────────────── ──────────────────────────────── ─────────────── ──────────────────────────
GRAND TOTAL (all features, default config)               ~18,088 B  ≈ 18 KB
```

</details>

### Default Build Footprint

The **Default server** in the Build Footprint table above - HTTP + WebSocket + SSE + multipart + file serving + Basic auth, including the Arduino/ESP-IDF framework core, ESP32 WiFi drivers, and the lwIP TCP/IP stack - measures:

- **Flash**: **745,133 bytes** (~56.8% of a 1.31 MB application partition); the WiFi/lwIP stack dominates (an empty no-WiFi sketch is already ~233 KB).
- **Static RAM**: **64,264 bytes** (~19.6% of the 320 KB internal SRAM), all statically placed.
- **Zero dynamic allocations**: the library requests exactly **0 bytes** of heap after `begin()`, managing every session and packet buffer statically (event queues via `xQueueCreateStatic()`).

Optional subsystems (HTTPS/TLS, SSH, SNMP, ...) add to this - see the per-row deltas in the Build Footprint table above.

## Utility Tools

A set of Python utility tools for formatting documentation, managing the test report directories, and styling/compressing web page assets.

<details>
<summary><b>Expand Utility Tools and Scripts Guide</b></summary>

**1. Interactive Theme Wizard**

The wizard guides developers through styling choices, compiles the customized assets, and prints the gzipped C++ hex array to the console.

```bash
python src/utilities/theme_wizard.py
```

**2. HTML Beautification and Decoration Tool**

Processes raw HTML source files to beautify, minify, or inject modern premium dark-mode styling.

```bash
# Beautify, minify, and inject CSS theme
python src/utilities/process_html.py --input src/html/index.html --output src/html/index_processed.html --minify --decorate-css
```

**3. Gzip HTML to Hex Array Converter**

Compresses a processed HTML template and outputs a C++ hex byte array in a timestamped text file to prevent naming collisions.

```bash
python src/utilities/gzip_html_to_hex.py --input src/html/index_processed.html
```

**4. Test Documentation Deep Dive Generator**

Scans Unity C++ test suites and auto-generates a nested, collapsible directory of test cases inside `TEST_DOCUMENTATION.md`.

```bash
python docs/utilities/generate_deep_dive.py
```

**5. Changelog Collapsible Decorator**

Parses `CHANGELOG.md` and wraps individual release versions in collapsible details sections. Used dynamically inside the CI pipeline.

```bash
python docs/utilities/decorate_changelog.py
```

</details>

## Testing

**600+ Unity tests** across the native suites, all runnable on a native x86/x64 host
(no hardware required). See **[TEST_REPORT.md](TEST_REPORT.md)** for the current
per-suite breakdown and totals. Run a representative subset with:

```
pio test -e native -e native_app -e native_ssh \
         -e native_ssh_hardened -e native_ssh_conn -e native_compliance
```

See **[TEST_DOCUMENTATION.md](TEST_DOCUMENTATION.md)** for the suite
breakdown and environment descriptions, and
**[TEST_REPORT.md](TEST_REPORT.md)** for the latest results
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
| [TEST_DOCUMENTATION.md](TEST_DOCUMENTATION.md) | Test suites, environments, and how to run them                    |
| [TEST_REPORT.md](TEST_REPORT.md)               | Latest test results (auto-generated)                              |
| [TODO.md](TODO.md)                             | Outstanding fixes and maintenance                                 |
| [ROADMAP.md](ROADMAP.md)                       | Forward-looking feature backlog (sized S/M/L)                     |
| [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md)   | Deliberate constraints and caveats                                |
| [TUNING.md](TUNING.md)                         | Performance tuning: worker count, core/affinity, poll knobs       |
| [MIGRATION.md](MIGRATION.md)                   | Breaking-change migration guide (3.x to 4.0.0)                    |
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
