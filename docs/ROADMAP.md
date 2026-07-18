# Roadmap

Forward-looking feature ideas for DeterministicESPAsyncWebServer, organized by
theme and sized **S / M / L**. This is a backlog of possibilities, not a
commitment or a schedule.

- `(shipped)` - already exists in the library today.
- a leading `*` - explicitly requested but not yet built.

Bugfixes, maintenance, known limitations, and the record of shipped work live in
[TODO.md](TODO.md); released changes are in [CHANGELOG.md](CHANGELOG.md). Every
item here must keep the library's core guarantees: no heap after `begin()`,
fixed-size buffers, a host-testable core where possible, and a `DETWS_ENABLE_*`
flag (default off) so it costs nothing when unused.

## Recommended near-term order (value vs. risk)

1. **Quick wins (all done):** Cache-Control beside ETag, runtime build-flag
   endpoint (`server.diag()`), MAC-derived UUID, raw-UDP telemetry cast.
2. **Security hardening cluster:** IP allowlist + brute-force lockout + CSRF
   tokens - all shipped.
3. **Partition-map monitor** (done) + config export / restore - the latter still
   needs NVS enumeration added to the config store first.
4. **Dashboard configurator** - done: telemetry math, the SVG dashboard over SSE,
   and the WebSocket controls + Canvas chart.
5. **Architectural (deliberate):** egress-interface reporting done (the stack
   already owns failover); next Ethernet PHY, GraphQL, OPC UA.

## Concurrency / performance

- [x] Thread-safe transport boundary _(shipped)_ - the cross-thread connection
      fields (state, RX ring head/tail) are `DetAtomic` (acquire/release), so the
      single-producer/single-consumer ring is race-free by construction; proven
      under ThreadSanitizer in CI (env `native_tsan`).
- [x] Dedicated server task _(shipped)_ - `begin()` runs the pipeline in its own
      pinned FreeRTOS task (Core 1 by default) instead of the user's `loop()`,
      which is freed; this also removes the first-connection-after-idle stall that
      came from `loop()`-cadence coupling.
- [x] Core-partitioned parallel workers _(shipped)_ - `DETWS_WORKER_COUNT` (default 1) workers each own a disjoint set of connection slots (round-robin at
      accept) with their own event queue + scratch arena: shared-nothing, no
      hot-path locks, so both cores process disjoint connections in parallel with
      bounded latency (determinism preserved). N=1 is byte-for-byte the original
      single pipeline.
- [x] Thread-safe app push path _(shipped)_ - `DetWebServer::defer(slot, fn, arg)`
      runs a callback in the slot's owning worker, so application code on `loop()`
      or another task can push (SSE/WS sends) without racing the worker.
- [x] Throughput benchmark _(measured)_ - a CPU-bound handler (~0.2s) under 4-way
      concurrency: N=1 ~5.9 req/s vs N=2 ~9.1 req/s (~1.5x; not a full 2x because
      worker 1 shares Core 0 with WiFi/lwIP), single-request latency unchanged.
      Confirms real parallel scaling with determinism intact (no hot-path locks).
- [x] Pluggable monotonic clock _(shipped)_ - `services/clock.h`: all library
      timing flows through `detws_millis()` (a single 1000 Hz source). Feed your own
      clock with `detws_set_clock(fn, ticks_per_second)` and it is divided down to
      the internal 1000 Hz, so timeouts/polling keep the tested 1 ms granularity
      whatever your clock's rate; default is the platform `millis()` (host-tested).
- [x] Notification-driven worker drain _(shipped)_ - the worker blocks on its
      FreeRTOS task notification instead of free-running the `vTaskDelay` poll;
      producers (`listener_enqueue`, `detws_defer`) nudge it the moment work is
      queued, so events are serviced immediately. This decouples event latency from
      the idle-sweep cadence: `DETWS_WORKER_POLL_TICKS` is now purely the idle
      interval (default 1, unchanged), so raising it cuts idle wakeups (CPU/power)
      at no latency cost (HW: GET latency identical at `POLL_TICKS` 1 vs 100). Zero
      heap (notifications, not a QueueSet).
- [x] Tuning _(shipped)_ - leaner tcpip recv callback (a received segment is now
      bulk-copied into the slot ring with a single SPSC publish instead of a
      per-byte loop with a per-byte atomic store), and a worker count / core /
      poll-knob tuning guide with the measured latency and idle-wakeup data in
      [TUNING.md](TUNING.md).

### v5 milestone: user-configurable preempting task queue + hardware ingest

The next big concurrency step (the **v5 milestone**): turn the worker model into a
real-time, hardware-ingest pipeline where data lands in a queue and the scheduler
preempts immediately to process it, with the priorities exposed to the user.

- [x] \*Preempting task queue (L) _(shipped)_ - `DETWS_ENABLE_PREEMPT_QUEUE`:
      `services/preempt_queue`, static (zero-heap) queues feeding dedicated
      core-pinned tasks. **From a task**, `detws_pq_post()` /
      `detws_pq_post_urgent()` (`xQueueSendToBack` / `xQueueSendToFront`) with a
      wait timeout; the scheduler preempts the lower-priority producer the instant
      the item is queued. **From an ISR**, `detws_pq_post_from_isr()`
      (`xQueueSendFromISR` + `portYIELD_FROM_ISR`) switches right after posting,
      not on the next tick. Fail-closed on a full queue. **Named lanes**: one USER
      lane exposed to the app plus internal DMA / FORWARD / DEVICE lanes that run
      above it (DMA highest, below tcpip / WiFi), so internal ingest preempts user
      work without starving networking. HW-verified (~12 us ISR-to-handler latency;
      DMA + USER lanes ran continuously with zero errors under an HTTP flood);
      host-tested via the per-lane ring core (examples Foundation/06.PreemptQueue +
      08.PreemptLanes).
- [x] \*User-configurable task priorities / affinity (M) _(shipped)_. Each lane's
      task priority + core pinning are set at `detws_pq_start[_lane]()` and the queue
      depth is compile-time (`DETWS_PQ_DEPTH`); the no-arg API drives the USER lane so
      users own their task priorities, while internal lanes default above the user lane.
- [x] \*DMA UART / I2C / SPI transfer (L) _(shipped)_ - `DETWS_ENABLE_DMA`:
      `services/dma`, channels moving peripheral bytes to a static ping-pong (RX) /
      staging (TX) buffer while the CPU is free; a DMA-complete event carries the
      bytes to a callback that posts them into the preempting queue. The ingest path
      for the cheap-breakout field-bus codecs (CAN over SPI, RS-485 UART, IO-Link).
      Zero-heap, fail-closed. The ingress/egress **simulator** (`DETWS_DMA_SIMULATE`,
      default on) runs the whole pipeline with no physical loopback - host bench and
      on-device; a real silicon driver plugs into `det_dma_hw_*`. Host-tested
      (`native_dma`) + HW-verified (2.2M+ frames, zero integrity errors, under HTTP
      stress). Remaining: the UHCI-UART / `spi_master`-DMA silicon backend.
- [x] \*Deeper clock-awareness (M) _(shipped)_ - `services/det_clock` gains a
      microsecond time base beside the 1000 Hz `detws_millis()`: `detws_micros()`
      (pluggable like the ms clock, ISR-safe, for hardware-event timestamps) plus a
      `DetwsLatencyStat` budget helper (`detws_lat_begin` / `_end` / `_avg_us`:
      count, min/max/mean, over-budget count). Header-only, zero heap. HW-verified
      measuring the preempting queue's ISR-to-handler latency against a budget
      (avg 12 us, 0 over a 50 us budget). Host-tested in `native_clock`.
- [x] \*Interface forwarding (L), **DMA-driven** _(shipped)_ - `DETWS_ENABLE_FORWARD`:
      `services/forward`, a forwarding plane over the ingest pipeline. Register
      interfaces (Wi-Fi STA / AP, Ethernet, a peripheral bus / radio), each with an
      egress send callback, then add per-pair rules (`src -> dst`, allow / deny, rate
      cap). A frame arriving on one interface (`det_forward_ingress()`, wired from a
      DMA-complete event on the FORWARD lane) is forwarded to every allowed
      destination, so the device bridges / routes instead of only terminating traffic.
      Default-deny, never reflects to the source, fail-closed (exceeded cap / refused
      send drops and is counted, never blocks); multi-destination fan-out for hub
      behavior. Zero-heap static tables. Host-tested (`native_forward`) + HW-verified
      (600k+ frames ingested over DMA and forwarded with zero loss / zero integrity
      errors under an HTTP flood; example Foundation/09.InterfaceForward). This is the
      generic data path the wireless gateway bridges below sit on top of.

### post-v5: RF / wireless gateway bridges

Once the hardware-ingest pipeline lands (the preempting queue + DMA peripheral
transfers above), the same codec-to-northbound pattern that bridges the wired field
buses (CAN over SPI, RS-485 / IO-Link over UART) extends to **wireless radios**: each
radio is a southbound peripheral reached over **SPI / I2C / UART**, and a gateway
function bridges its frames northbound to the existing WiFi / MQTT / HTTP / WebSocket
stack. Deterministic, zero-heap, fixed buffers; the DMA-complete / data-ready ISR posts
into the preempting queue, so wireless ingest rides the same real-time path as the wired
codecs. (ESP32 Wi-Fi / classic + BLE are already on-chip; everything below is an
external module we wire to a bus.)

The **generic gateway framework is shipped** (`DETWS_ENABLE_GATEWAY`, `services/gateway`):
ports, address-aware northbound enveloping + topic (`det_gateway_uplink` / `det_gateway_topic`),
bidirectional up/down-link, a per-port rate cap, and stats - all fail-closed and zero-heap,
HW-verified end to end over DMA + the FORWARD lane. Each radio below is now a per-module
**codec + driver** (frame parse, SPI/UART register access) plugged into that framework's
callbacks; the items stay open until verified against the real hardware.

SPI radios:

- [~] \*LoRa / LoRaWAN gateway (L) - Semtech SX127x / SX126x, RFM95/96 over SPI.
  **Codec + SX127x driver shipped** (`DETWS_ENABLE_LORA`, `services/lora`): the
  RadioHead 4-byte header codec + the SX1276 register protocol (init / send / recv /
  RSSI) over a portable register-access bus, host-tested against a mock chip; example
  11.LoRaGateway drives a real RFM95 and bridges frames northbound via the gateway.
  Remaining: verify the RF link on the module, an SX126x variant, and a bounded
  LoRaWAN Class A uplink/downlink codec.
- [~] \*nRF24L01+ gateway (M) - Nordic 2.4 GHz over SPI; cheap point-to-multipoint
  sensor links bridged to the web stack. **Driver shipped** (`DETWS_ENABLE_NRF24`,
  `services/nrf24`): the SPI command protocol (init / send / recv, pipe-addressed,
  static payload) over an SPI + CE bus, host-tested against a mock chip; example
  12.Nrf24Gateway bridges frames northbound via the gateway. Remaining: verify the
  RF link on the module (and optional dynamic-payload / auto-ack).
- [~] \*CC1101 sub-GHz gateway (M) - TI 300-928 MHz OOK / 2-FSK over SPI; generic
  ISM-band remotes and sensors. **Driver shipped** (`DETWS_ENABLE_CC1101`, `services/cc1101`): the CC1101
  SPI header protocol (config registers, the 13 command strobes, status registers, TX/RX FIFO) - reset +
  apply a caller-supplied SmartRF register table + set channel + verify VERSION (`cc1101_init`),
  variable-length `cc1101_send`, TX-done poll, `cc1101_set_rx`, and `cc1101_recv` with the appended
  RSSI/LQI status + `cc1101_rssi_dbm` decode, over a portable SPI bus; host-tested against a mock chip
  (`native_cc1101`). Bridges northbound via the gateway framework. Remaining: verify the RF link on the
  module (and an OOK/ASK preset variant).
- [~] \*Thread / Matter RCP (L) - OpenThread radio co-processor (nRF52840 / EFR32)
  over SPI (spinel framing); 802.15.4 mesh bridged to IP. **spinel / HDLC-lite framing
    - command codec shipped** (`DETWS_ENABLE_THREAD`, `services/thread`): the byte-stuffed
      framing + CRC-16/X-25 FCS (encode/decode) that carries spinel, plus the spinel command
      layer - the packed-uint encoding and `spinel_command_build` / `_parse` for a
      `header | CMD | PROP | value` property command - host-tested against the X-25 check value
    - the packed-int values; example 18.ThreadGateway bridges a real RCP. Remaining: the
      spinel property registry / value semantics + verify against an RCP.

UART radios:

- [~] \*Zigbee NCP gateway (L) - Silicon Labs EZSP (EFR32) / Digi XBee / TI ZNP over
  UART; join as coordinator and bridge Zigbee devices to MQTT. **ASH framing codec
  shipped** (`DETWS_ENABLE_ZIGBEE`, `services/zigbee`): the byte-stuffed, CRC-16/CCITT
  ASH data-link (encode/decode) that carries EZSP, host-tested against the documented
  RST frame; example 17.ZigbeeGateway bridges a real NCP. Remaining: the EZSP command
  layer (version negotiation, incomingMessageHandler) + verify against an NCP.
- [~] \*Z-Wave Serial API gateway (M) - Silicon Labs 500 / 700-series over UART.
  **Serial API frame codec shipped** (`DETWS_ENABLE_ZWAVE`, `services/zwave`):
  SOF/LEN/Type/Cmd/Data frames + the XOR checksum + ACK/NAK/CAN, host-tested against
  the documented GetVersion frame; example 16.ZWaveGateway bridges a real controller.
  Remaining: verify against a controller + inclusion / SendData sequences.
- [~] \*EnOcean gateway (M) - energy-harvesting 868 MHz ESP3 protocol over UART.
  **ESP3 codec shipped** (`DETWS_ENABLE_ENOCEAN`, `services/enocean`): telegram
  parse/build + the CRC-8 (poly 0x07), host-tested with known-answer CRCs and
  malformed/resync framing; example 13.EnOceanGateway bridges a real TCM 310 over
  UART. Remaining: verify against a module + EEP profile decoding.
- [~] \*Sigfox uplink (S) - Wisol / Murata module over UART; tiny low-power uplinks.
  **AT-command codec shipped** (`DETWS_ENABLE_SIGFOX`, `services/sigfox`): the
  `AT$SF=<hex>` uplink formatter + OK/ERROR/pending response classifier, host-tested;
  example 15.SigfoxUplink sends a reading from a real modem. Remaining: verify against
  a module + subscription.
- [~] \*Wi-SUN FAN **connector** (L) - NOT a radio-module driver: the direct FAN
  UART modules (Rohm BP35A1 class) are obsoleted, so Wi-SUN is reached as a
  **connector to a border router / devboard** that already terminates the FAN mesh.
  Wi-SUN FAN is an IPv6 / UDP / CoAP network, so this rides the existing IP stack
  (CoAP / UDP client to the border router) rather than a byte-level radio codec.
  **Connector shipped** (`DETWS_ENABLE_WISUN`, `services/wisun`): since the CoAP service ships only a
  _server_, the connector adds the CoAP **client** request builder (`wisun_build_coap`: RFC 7252 header +
  delta-encoded Uri-Path options with extended-length + payload) plus the FAN node registry (register /
  find / joined-count) keyed on `DetIp`, and `wisun_nodes_json` for the web. The app sends the built PDU
  to a node's IPv6 address over `det_udp`; pure + host-tested (`native_wisun`). _Remaining:_ point it at a
  chosen border-router devboard and verify the mesh end to end (the devboard choice is the only external
  dependency; the connector code is board-agnostic).

I2C / SPI / UART:

- [~] \*NFC / RFID gateway (M) - PN532 (I2C / SPI / UART) or MFRC522 (SPI); tag
  read / write bridged to an HTTP / MQTT event. **PN532 frame codec shipped**
  (`DETWS_ENABLE_PN532`, `services/pn532`): normal-information-frame build/parse +
  ACK with the LCS / DCS checksums, host-tested against the documented
  GetFirmwareVersion frames; example 14.NfcGateway reads a real PN532 over I2C.
  Remaining: verify against a reader + the MFRC522 variant.

Built-in radio:

- [~] \*BLE GATT bridge (M) - on-chip ESP32 BLE (and external HCI-UART modules):
  scan / expose GATT characteristics and bridge them to the web stack. **ATT codec + bridge shipped**
  (`DETWS_ENABLE_BLE_GATT`, `services/ble_gatt`): the Attribute Protocol PDUs under GATT (Bluetooth Core
  Vol 3 Part F) - build/parse read / write / notify / error with little-endian handles - plus a GATT
  characteristic-table JSON serializer for the web, pure + host-tested (`native_ble_gatt`). Remaining:
  wire it to the NimBLE / Bluedroid radio (scan + a GATT server) and an HCI-UART transport variant.

### post-v5: promiscuous / monitor capture

A read-only capture mode across the same interfaces: instead of joining a network and
terminating traffic, listen to **every** frame on a channel and surface it northbound
(live over WebSocket, or batched to a PCAP / log) for diagnostics, an on-device IDS, and
field debugging. Capture is strictly passive (no injection on the capture path) and
fail-closed: a full capture queue drops frames rather than stalling the live data path.

- [x] \*Wi-Fi promiscuous / monitor mode (M) _(shipped, v4.84.0)_ - on-chip ESP32 raw 802.11
      capture (`DETWS_ENABLE_PROMISC`, `services/promisc`): `promisc_begin(channel, sink)` over
      `esp_wifi_set_promiscuous`, a pure `wifi_frame_parse()` 802.11 header decoder (to/from-DS
      src/dst/bssid, QoS, WDS), and libpcap `DLT_IEEE802_11` framing. The sink feeds the
      forwarding plane, so captured frames bridge to another interface - **capture on Wi-Fi,
      forward to Ethernet** to a wired PCAP collector (example 21.WifiCapture). Host-tested
      (`native_promisc`); both cores compiled. Northbound-over-WebSocket is a follow-on wiring.
- [x] \*Bus listen-only capture (M) _(CAN shipped, v4.85.0)_ - `DETWS_ENABLE_BUS_CAPTURE`
      (`services/bus_capture`): the TWAI controller in listen-only mode decodes every CAN frame
      without ACKing and feeds the forwarding plane, so captured frames bridge to another
      interface. `can_to_socketcan()` + libpcap `DLT_CAN_SOCKETCAN` make the stream
      Wireshark-readable (example 22.CanCapture, host-tested `native_bus_capture`; the SocketCAN
      framing shares `shared_primitives/pcap.h` with the Wi-Fi capture). Remaining: RS-485
      receive-only decode into the same sink.
- [~] \*Radio channel sniff (L) - the RF gateways above in receive-only mode (sniff a
  LoRa / sub-GHz / 802.15.4 channel without joining) feeding the capture pipeline. **Capture framing
  shipped** (`DETWS_ENABLE_RADIO_SNIFF`, `services/radio_sniff`): wraps each received 802.15.4 MAC frame
  in the Wireshark IEEE 802.15.4 TAP pseudo-header (per-frame RSSI + channel TLVs, an exact int->float32
  RSSI encode) and a pcap record (new `DET_DLT_IEEE802_15_4_TAP` / `_NOFCS` link types in
  `shared_primitives/pcap.h`), so a sniffed channel opens directly in Wireshark; pure + host-tested
  (`native_radio_sniff`). Remaining: put the CC1101 / LoRa / Thread drivers into receive-only and route
  their frames through this into the existing forwarding sink on hardware.

### post-v5: field-perturbation sensing

Sensors that read the environment by measuring a perturbation in an emitted or ambient
field, the same southbound-peripheral pattern pointed at sensing instead of comms: the
device reads the sensor over SPI / I2C / UART and bridges the readings northbound to the
dashboard, telemetry math, and MQTT / WebSocket. The data-ready ISR posts into the
preempting queue, so sensing shares the real-time ingest path.

- [~] \*EM / radar presence + motion (M) - mmWave radar (24 / 60 GHz: LD2410 / MR60BHA
  over UART, Infineon BGT60 over SPI) and Doppler motion (HB100 / RCWL-0516) for
  presence, motion, distance, and vital-sign (breathing / heart-rate) sensing. **LD2410 shipped**
  (`DETWS_ENABLE_LD2410`, `services/ld2410`): the framed 256000-baud report codec (`ld2410_parse_report`
  basic + engineering mode, a byte-by-byte `Ld2410Stream` reassembler that resyncs on noise, presence /
  distance helpers) and the `FD FC FB FA` config-command encoders, pure + host-tested (`native_ld2410`),
  with an ESP32 UART binding (example wired). Remaining: an MR60BHA 60 GHz vital-sign variant, the
  Infineon BGT60 SPI radar, and the analog Doppler (HB100 / RCWL-0516) presence path.
- [~] \*Capacitive / proximity field sensing (S) - FDC2x14 / MPR121 (I2C): touch,
  proximity, liquid level, and material sensing from capacitance shifts. **Both drivers shipped**:
  MPR121 12-channel touch/proximity (`DETWS_ENABLE_MPR121`, `services/mpr121`) and the FDC2114/2214
  capacitance-to-digital field sensor (`DETWS_ENABLE_FDC2214`, `services/fdc2214`): the 28-bit data
  combine + error flags + frequency scale + single-channel config-sequence builder, pure codec
  host-tested (`native_fdc2214`), with an ESP32 I2C binding that verifies the device id, applies the
  config, and reads the channel. Remaining: verify capacitance shift on a real electrode / bench.
- [~] \*Inductive / EM field sensing (S) - LDC1614 (I2C) inductance-to-digital for
  metal / displacement, plus magnetometer-based EM-field perturbation detection. **Driver shipped**
  (`DETWS_ENABLE_LDC1614`, `services/ldc1614`): the LDC1614 28-bit data combine + error flags + frequency
  scale (`data/2^28 * fref`) + single-channel config-sequence builder, pure codec host-tested
  (`native_ldc1614`), with an ESP32 I2C binding that verifies the device id, applies the config, and reads
  the channel. Remaining: verify eddy-current shift on a real coil / bench, and the magnetometer EM path.
- [~] \*Time-of-flight ranging (S) - VL53L0X / VL53L1X (I2C) optical ToF distance and
  gesture, bridged to the same sink. **Driver shipped** (`DETWS_ENABLE_VL53L0X`, `services/vl53l0x`): the
  documented ranging registers - `vl53l0x_range_mm` combines the range byte pair, `vl53l0x_data_ready`
  decodes the interrupt-status byte, `vl53l0x_range_valid` checks the device range-status field - pure
  codec host-tested (`native_vl53l0x`), with an ESP32 I2C binding that verifies the model id, starts
  continuous ranging, and reads the distance. Remaining: apply ST's tuning blob for best accuracy, a
  VL53L1X variant, and a bench range check.

## Web / API / UI

- [x] WebSocket permessage-deflate, inbound and outbound _(shipped)_ - bounded fixed-Huffman DEFLATE compresses server-to-client data frames (RSV1), with an uncompressed fallback when the result would not shrink.
- [x] REST substrate, AJAX _(shipped)_.
- [x] Real-time **dashboard** _(shipped, phase 1)_ - `DETWS_ENABLE_DASHBOARD`: a compile-time `DetwsWidget` table served as hand-rolled SVG (gauge / value / bar / sparkline, no external JS) via the `web_assets` pipeline, live over SSE; `detws_dashboard_set` / `_publish` (example 62.Dashboard). **Flagship.**
- [x] Dashboard phase 2 _(shipped)_ - WebSocket controls (button / toggle / slider widgets send values to a `detws_dashboard_on_control` callback) and a Canvas chart widget for dense series.
- [x] **Telemetry math** cluster _(shipped)_ - `services/telemetry`: moving-window stats (mean / variance / stddev / min / max), a rate-of-change tracker, and a trapezoidal run-time totalizer (example 61.Telemetry).
- [x] HTTP caching: `Cache-Control` beside ETag _(shipped)_ - `set_cache_control()` injects it into serve_file / serve_static responses.
- [~] HTTP delivery (S-M): stale-while-revalidate, service-worker cache injection, delta/offset log fetching
  _(cores shipped)_ - `DETWS_ENABLE_HTTP_DELIVERY` (`services/http_delivery`): RFC 5861
  `detws_delivery_swr` freshness decision + `detws_delivery_cache_control` header (stale-while-revalidate),
  RFC 7233 `detws_delivery_range` byte-range parse (`X-Y` / `X-` / `-N`, clamped, 416/multi-range rejected)
  plus `detws_delivery_content_range` for a 206 (delta/offset log fetch), and `detws_delivery_sw_manifest`
  emitting the versioned `{"version":..,"precache":[..]}` a service worker consumes (SW cache injection).
  Pure, host-tested (`native_http_delivery`). _Remaining:_ wiring the cores into the served endpoints +
  shipping the static service-worker asset (M).
- [x] CBOR encoder + decoder _(shipped)_ - `DETWS_ENABLE_CBOR`: a zero-heap RFC 8949 writer plus a cursor decoder (`cbor_peek` / `cbor_read_*`, no-copy strings) over caller buffers - ints / strings / bytes / arrays / maps / bool / null / float; host-tested against the spec vectors + round-trip (example 65.Cbor).
- [x] MessagePack encoder + decoder _(shipped)_ - `DETWS_ENABLE_MSGPACK`: a zero-heap streaming writer over a caller buffer - shortest-form ints (fixint / 8 / 16 / 32 / 64) / str / bin / arrays / maps / bool / nil / float32; overflow tracked, fails closed - plus a cursor decoder (`msgpack_peek` / `msgpack_read_*`, no-copy strings, fail-closed on malformed/truncated input, ext reported as INVALID) host-tested against spec vectors + round-trip and fuzzed in the pentest harness (example 66.MsgPack, both directions). Remaining (M-L): Protobuf / FlatBuffers zero-copy.
- [x] GraphQL bounded subset _(shipped)_ - `DETWS_ENABLE_GRAPHQL`: `services/graphql` parses a query into a fixed AST pool (no heap) and emits `{"data":{...}}` shaped by the selection; schema-free (sub-selection = object, leaf = one resolver call, args collected along the path), with nesting + arguments (example 81.GraphQL). Feature-dependent schema generation remains open (M).
- [x] Browser diag tools _(shipped, GPIO mapper)_ - `DETWS_ENABLE_GPIO_MAP`: a compile-time pin table (number / label / direction / live level) served at GET `/gpio` as JSON, with a POST control (`pin`, `level`) that drives a mapped output; the serializer + control parser are host-tested, and the example ships a zero-dependency browser panel (example 67.GpioMap). Remaining (M): ping / tracert panel, web logic analyzer.
- [~] **SPA micro-routing** + conditional UI streaming (M) _(routing decision shipped)_ -
  `DETWS_ENABLE_SPA_ROUTER`: `services/spa_router` `detws_spa_route()` decides, from a request path,
  whether to serve a real asset file, serve the SPA shell (`index.html`) for an extensionless
  client-side route, or pass through to the app under an API prefix - so a single-page UI's client
  routing works over static file serving. Pure decision core, host-tested (`native_spa_router`).
  _Remaining:_ conditional UI streaming + a local SCADA/HMI fallback (M).
- [x] WS MTU-aligned chunking / fragmentation control (M) _(shipped)_ - `DETWS_WS_FRAG_SIZE` (0 = off,
      default) / the runtime `ws_set_frag_size()`: an outbound data message longer than the set payload
      size is split into that-sized WebSocket frames (RFC 6455 sec 5.4: opcode+RSV1 on the first,
      CONTINUATION on the rest, FIN on the last), so each frame maps to whole TCP segments (MTU-aligned)
      and a peer with a bounded per-frame reassembly buffer can receive an arbitrarily long message.
      Compression (RFC 7692) applies to the whole message first, then the compressed bytes are split.
      Host-tested (`test_ws_outbound_fragmentation`); default 0 keeps the single-frame behavior unchanged.

## Protocols & integrations

- [x] OPC UA server + client _(shipped, SecurityPolicy None)_ - `DETWS_ENABLE_OPCUA`: the OPC UA Binary built-in-type codec, UA-TCP (UACP) framing, Hello/Acknowledge handshake, OpenSecureChannel, CreateSession/ActivateSession, GetEndpoints, Read/Write (Variant/DataValue), and Browse, served on PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`; example 55.OpcUa), plus an OPC UA client (example 56.OpcUaClient). Host-tested (`native_opcua`, `native_opcua_client`) and HW-verified end-to-end against the `asyncua` reference stack via the interop harness (`opcua-client` 3/3: connect+session, browse Objects, read node). Remaining (L): a secure SecurityPolicy (Basic256Sha256 encryption/signing), subscriptions / monitored items, and Node-RED integration (M).
- [x] Modbus master codec + register scanner _(shipped)_ - `DETWS_ENABLE_MODBUS_MASTER`: `services/modbus/modbus_master` builds read-request ADUs and parses responses (register values or exception) so an app can poll / auto-discover a slave's registers; pure, host-tested as a full round-trip against the slave codec, HW-verified via self-scan (example 72.ModbusScan).
- [~] Southbound protocol-driver framework _(framework shipped)_ - `DETWS_ENABLE_SOUTHBOUND`
  (`services/southbound`): the uniform seam every field-device driver plugs into so the app polls/drives
  any southbound device (Modbus slave, BACnet controller, raw SPI/I2C/UART sensor) through one facade -
  a bounded driver registry (`detws_southbound_register` / `_find` / `_count` / `_clear`) plus name-dispatched
  `detws_southbound_read` / `_write` / `_read_block` / `_write_block`, where the block calls are the atomic
  multi-point (register-matrix) path. Each driver owns its transport (a read/write vtable + ctx); Modbus
  master is the one such driver today. Pure registry + dispatch, host-tested (`native_southbound`).
  _Remaining:_ a Modbus adapter binding modbus_master into a SouthboundDriver + the concrete atomic
  register matrix (M), both transport-gated.
- [x] Webhooks + IFTTT _(shipped)_ - `DETWS_ENABLE_WEBHOOK`: `services/webhook` builds an IFTTT Maker URL + value1/2/3 JSON payload (pure, host-tested with JSON escaping) and fires it - or any JSON to any URL (Slack/Discord/your API) - via the outbound http_client; HW-verified by a self-loopback POST (example 75.Webhook).
- [~] Email + SMS fallbacks (SMTP + gateway) (M) - **SMTP shipped** (`DETWS_ENABLE_SMTP`): `services/smtp` runs a blocking RFC 5321 send over the shared `det_client` transport (EHLO, optional AUTH LOGIN, MAIL/RCPT/DATA with dot-stuffing, QUIT), with implicit TLS (SMTPS, port 465) when the message sets `tls` and `DETWS_ENABLE_TLS` is on; zero heap, fixed buffers. The dialogue engine (`smtp_run`) is split behind a send/recv seam and host-tested against a scripted mock server (`native_smtp`, 11 cases: happy path, AUTH, dot-stuffing, multi-line replies, every error). SMS needs no new code (email-to-SMS carrier gateway address). Example 57.SmtpAlert ships a full beginner walkthrough that stands up a Postfix server. _Remaining:_ STARTTLS on the submission port (587) upgrade, and a live over-the-wire HW send (blocked in the current lab by AP client isolation + ISP port-25 filtering, same as the concurrent-TLS soak - the code compiles + boots + connects on the modern core).
- [x] ESP-NOW peer messaging _(shipped)_ - `DETWS_ENABLE_ESPNOW`: `services/espnow` adds a typed 3-byte envelope (demux by type + length check) over connectionless ESP-NOW, a bounded peer registry, and the esp_now radio binding (begin / add_peer / send / broadcast, decoded frames to a callback for bridging to WS/SSE) (example 82.EspNow). Codec + registry host-tested; ESP-NOW<->MQTT auto-bridge remains open (M).

## Networking / connectivity

- [x] Egress-interface reporting _(shipped)_ - `det_net_egress()` / `det_net_egress_ip()` read the live lwIP default route so the app always knows which interface (WiFi STA / softAP / Ethernet) its traffic is leaving on; the stack owns the actual failover, so no manager or polling tick is added (example 63.NetEgress).
- [~] Ethernet PHY bring-up (L, greenlit) - a wired-PHY init alongside WiFi; failover + the egress report
  above already work once both links exist. Multi-interface bridge / graceful escalation (L)
  _(escalation policy shipped)_ - `DETWS_ENABLE_LINK_MANAGER` (`services/link_manager`): a table of
  interfaces (kind + priority + up/down) with deterministic best-link-up selection, graceful escalation to
  a higher-priority interface when it comes up, failover to the next best when it drops, and change
  detection so the app reconfigures the netif only on a real transition. Pure, host-tested
  (`native_link_manager`). _Remaining:_ the `esp_eth` PHY init on hardware + the multi-interface bridge /
  packet forwarding (the v5 interface-forwarding milestone).
- [~] IPv6 dual-stack + fallback (L); VPN tunneling + reverse-SSH tunnel to a relay (L) _(dual-stack +
  fallback shipped)_ - the address plumbing is `DetIp` (RFC 4291 parse / RFC 5952 format / scope classify
  / CIDR) and the netif bring-up is `DETWS_ENABLE_IPV6` (physical layer; listeners bind
  `IPADDR_TYPE_ANY`). The client-side fallback is `DETWS_ENABLE_HAPPY_EYEBALLS` (`services/happy_eyeballs`):
  `detws_he_pref` scores a destination (RFC 6724 scope + family), `detws_he_order` sorts a candidate list
  and interleaves the address families (RFC 8305) so successive attempts alternate v6/v4, and
  `detws_he_attempt_due` gates the next attempt by the Connection Attempt Delay - fast IPv6, quick IPv4
  fallback. Pure, host-tested (`native_happy_eyeballs`). _Remaining:_ VPN tunneling + the reverse-SSH
  tunnel to a relay (L; the `ssh -R` tcpip-forward seam already landed).
- [ ] **IPsec / IKEv2** (XL, the concrete VPN for the item above - **secure machine bridge over untrusted
      networks**) - IKEv2 (RFC 7296) over UDP 500 / 4500 (NAT-T) plus the ESP datapath (RFC 4303). Splits
      into three tractability tiers, built in order:
    1. **IKEv2 message + payload codec** (M, tractable, matches our zero-heap codec pattern) - the 28-octet
       header (initiator/responder SPIs, next-payload, MjVer/MnVer, exchange type, flags, message id, length)
       and the generic payload chain (next-payload + critical + length) for the payloads: **SA** (proposals ->
       transforms: ENCR / PRF / INTEG / D-H / ESN), **KE**, **Ni/Nr** nonce, **AUTH**, **IDi/IDr**, **CERT** /
       **CERTREQ**, **TSi/TSr** traffic selectors, **N** notify, and **SK** encrypted-payload framing. Pure
       build/parse into caller buffers, host-tested with RFC vectors. **Reuses crypto we already ship**:
       `ssh_curve25519` (D-H group 31 = X25519, and MODP groups via `ssh_bignum`), `ssh_chachapoly`
       (ChaCha20-Poly1305 per RFC 7634), `ssh_sha256/512` (PRF/INTEG HMAC), AES-GCM (the `quic_aead` core) -
       so the primitive surface is largely done; this is mostly framing + the key-derivation (SKEYSEED /
       the SK_* chain).
    2. **IKE SA handshake state machine** (L) - IKE_SA_INIT -> IKE_AUTH (PSK and/or certificate auth) ->
       CREATE_CHILD_SA / INFORMATIONAL, rekeying, DPD (dead-peer detection), and the fixed-BSS SA/SPI tables.
       Still host-testable against a reference peer (strongSwan / libreswan) with no datapath.
    3. **ESP datapath** (XL, the genuinely hard part - architecturally invasive) - RFC 4303 ESP packet
       encapsulation is a **network-layer transform**, not an app service: it must hook lwIP's IP input/output
       (a custom netif or an ip4/ip6 hook) to encrypt/decrypt + (anti-replay) sequence every datagram, with
       the SAD/SPD. This is the real weight and the open architectural question (lwIP has no native IPsec);
       tunnel mode first (whole-packet, simplest SPD). Gate behind its own flag; the codec + handshake tiers
       ship and test independently first. Refs: RFC 7296 (IKEv2), 4303 (ESP), 7634 (ChaCha20-Poly1305 for
       IKEv2/ESP), 8247 (algorithm requirements).
- [~] WiFi (M): sniffer / traffic analyzer / RF diag, channel-agility roaming _(decode + decision shipped)_ -
  `DETWS_ENABLE_WIFI_SNIFFER` (`services/wifi_sniffer`): `detws_wifi_parse` decodes an 802.11 MAC header
  (frame-control type/subtype + flags and the addresses whose roles depend on the ToDS/FromDS bits),
  `detws_wifi_stats_*` tallies frames by type for a traffic panel, and `detws_wifi_should_roam` is the
  RSSI-hysteresis channel-agility roaming decision. Pure, host-tested (`native_wifi_sniffer`).
  _Remaining:_ the promiscuous-mode radio callback + a live channel-hop scan loop on hardware (M).
- [x] DNS resolver + answer verification _(shipped)_ - `DETWS_ENABLE_DNS_RESOLVER`: `services/dns_resolver` resolves a hostname to IPv4 (lwIP dns_gethostbyname marshalled to tcpip_thread, dotted-quad fast path) and verifies the answer - rejecting 0.0.0.0 / broadcast / loopback / multicast as spoof / DNS-rebinding indicators; classifier + verifier host-tested, HW-verified against live DNS (example 77.DnsResolver). Remaining (M): captive-portal DNS-spoof mitigation, captive-portal auto-teardown timer.
- [x] mDNS TXT / `_https._tcp` / extra services _(shipped)_ - `det_mdns_txt` / `det_mdns_add_service`.
- [~] mDNS adaptive / auto-sleep beacons + a continuous refresher for crowded RF (M) _(scheduler shipped)_ -
  `DETWS_ENABLE_MDNS_ADAPTIVE` (`services/mdns_adaptive`): `det_mdns_beacon_adapt` backs the announce
  interval off toward a ceiling under RF contention and recovers it toward the nominal cadence when the
  air is quiet, `det_mdns_refresh_interval` gives the TTL/2 continuous-refresher cadence,
  `det_mdns_beacon_due` says when an announce is due, and `det_mdns_beacon_presleep_due` is the
  auto-sleep beacon (announce before a sleep window that would let the record lapse). Wrap-safe time math,
  pure, host-tested (`native_mdns_adaptive`). _Remaining:_ wiring the schedule to the mDNS transmit + a
  live contention counter from the WiFi driver (M).
- [x] Raw-UDP telemetry cast _(shipped)_ - `DETWS_ENABLE_UDP_TELEMETRY`: `services/udp_telemetry` builds InfluxDB line-protocol records (`measurement field=val,...`, host-tested) and casts them to a collector over UDP via `det_udp_sendto`, zero-heap fire-and-forget (example 68.UdpTelemetry).
- [~] **Static-IP fallback + TCP window auto-scaling by free RAM (M)** _(decision core shipped)_ -
  `DETWS_ENABLE_NETADAPT`: `services/netadapt` `detws_netadapt_window()` sizes the TCP receive window
  from the free heap (a reserve left untouched, then a quarter of the spare, clamped to [min, max]) so
  a RAM-rich device gets throughput and a tight one stays alive; `detws_netadapt_dhcp_fallback()`
  decides when to give up on DHCP and use a static IP (elapsed timeout or retry budget). Pure cores,
  host-tested (`native_netadapt`); the app applies the results (lwIP window / netif config). Dynamic
  socket recycling (the transport-pool concern) shipped separately as `DETWS_ENABLE_SOCKPOOL`
  (`services/sockpool`): a fixed LRU connection-slot pool that, when saturated, recycles the
  least-recently-used slot for a new peer and reports the evicted id so the transport closes it (acquire /
  touch / release / find), pure + host-tested (`native_sockpool`). _Remaining:_ none (cores complete).

## Power & radio management

- [x] Radio power _(shipped)_ - `DETWS_ENABLE_RADIO_POWER`: `services/radio_power` applies a WiFi modem-sleep mode (`DETWS_RADIO_WIFI_PS` none/min/max) + an optional max-TX cap (`DETWS_RADIO_MAX_TX_DBM`) in one call (esp_wifi_set_ps / set_max_tx_power), trading throughput for lower average power; mode names host-tested, apply/readback HW-verified (example 76.RadioPower). Remaining: BT-coexistence preference (only relevant on a BT-enabled build).
- [~] Dynamic network sleep modes / **sleep-cycle scheduler (M)** _(scheduler shipped)_ -
  `DETWS_ENABLE_SLEEP_SCHED`: `services/sleep_sched` `detws_sleep_next()` decides, from the idle time,
  how long a low-power device should sleep (0 = awake), ramping the window from a floor up to a
  ceiling (doubling every `ramp_ms`) the longer the idle streak runs. Pure wrap-safe decision core,
  fully host-tested (`native_sleep_sched`); the app applies the window via light / modem / deep sleep.
  Complements `services/radio_power`. _Remaining (need real hardware to verify):_ dynamic power
  scaling, thermal throttling, brownout recovery, peripheral power gating (M).

## Security & auth

- [x] Source-IP allowlist / firewall in the accept callback _(shipped)_ - `listener_ip_allow_add_cidr("192.168.1.0/24")` / `listener_ip_allowed` (IPv4 + IPv6 CIDR rules matched on the full address, `DETWS_ENABLE_IP_ALLOWLIST`; example 58.IpAllowlist).
- [x] Brute-force per-IP exponential lockout _(shipped)_ - `DETWS_ENABLE_AUTH_LOCKOUT`; `auth_lockout_*` table (keyed on the full IPv4/IPv6 address) issues 429 + Retry-After on the HTTP auth gate (example 59.AuthLockout).
- [x] CSRF token verification _(shipped)_ - `DETWS_ENABLE_CSRF`; global enforcement on POST/PUT/PATCH/DELETE via a stateless HMAC-signed `X-CSRF-Token` (built-in `GET /csrf` issues it; example 60.Csrf).
- [x] Granular API-token scoping _(shipped)_ - `jwt_claim_str()` reads string claims (sub / role / scope) and `jwt_scope_allows()` matches a space-separated OAuth2 scope claim, so a handler can authorize per role/scope on the verified JWT (example 21.JWTAuth).
- [x] MFA - TOTP two-factor _(shipped)_ - `DETWS_ENABLE_TOTP`: `services/totp` computes / verifies RFC 6238 time-based one-time passwords (HMAC-SHA1 on the software SHA-1, Authenticator-compatible) and decodes base32 secrets, for a second factor on top of password / JWT; host-tested against the RFC vectors, HW-verified (example 74.Totp). An external-API verifier can also be called from a handler via the http_client.
- [x] OIDC ID-token verification + OAuth2 token exchange _(shipped)_ - RS256 relying-party verifier (`services/oidc`: JWKS key selection by kid, real RSA-2048 signature check, iss/aud/exp/nbf, claim extraction) plus the OAuth2 token-endpoint client (`services/oauth2`: authorization_code + refresh_token grants, confidential-client or PKCE, JSON token parsing; example 83.OAuth2). SAML remains open (heavy XML/canonicalization - poor fit) (L).
- [x] Secure boot + flash encryption _(shipped, docs)_ - [docs/SECURE_BOOT.md](SECURE_BOOT.md): a hardening guide for ESP32 Secure Boot v2 + Flash Encryption (and NVS encryption) mapped to the secrets this library holds (SSH host key, TLS key, SNMPv3/JWT secrets, config blobs, audit-log sink). Encrypted config handshake during onboarding remains open (M).
- [x] MAC-derived UUID _(shipped)_ - `detws_uuid_from_mac` / `detws_device_uuid` (RFC 4122 v5; example 57.DeviceUuid).
- [x] **Modern SSH ECC: Curve25519 key exchange + Ed25519 keys** (L) _(shipped v4.94.0)_ - the SSH server now negotiates a crypto-agnostic KEX: `curve25519-sha256` + `ssh-ed25519` host key + ed25519 client auth, alongside the `diffie-hellman-group14-sha256` + `rsa-sha2-256` set, with the preference runtime-selectable (`ssh_kex_set_prefer_rsa`, default RSA). The curve backend is a bespoke constant-time X25519 / Ed25519 that fits the zero-heap / no-stdlib idiom (no micro-ecc / donna / wolfSSL dependency), with the ESP32 MPI accelerator driving field inversion on-device and a software fallback retained. RSA stays as a fallback; ed25519 host-key provisioning sits alongside the RSA DER path; crypto KATs cover both suites and the OpenSSH interop test passes with zero forced algorithms. HW-verified on an ESP32-S3 (both suites; worker stack raised to 12288 when SSH is enabled, curve peaks ~10.5 KB).

### Crypto parity vs CycloneSSL / CycloneSSH (ESP32-relevant gaps)

Comparison against Oryx Embedded's **CycloneSSL** (TLS/DTLS) and **CycloneSSH** - the closest portable,
zero-external-dependency C stacks to ours. Only gaps that matter on an ESP32 are tracked (their
Camellia / SEED / ARIA / Serpent / Twofish / RC4 / Blowfish / DSS ciphers are deliberately skipped).
**Benchmark caveat:** Oryx publishes their numbers at **-O3**, so any apples-to-apples throughput /
handshake comparison must build our crypto at `-O3` too (our default examples build `-Os`); measure the
same core on the same S3 at `-O3` before claiming a delta. Our current surface: **TLS** = mbedTLS 1.2/1.3
(ECDHE curve-pref, AES-GCM, ChaCha20-Poly1305, tickets/resumption); **SSH** = curve25519-sha256 +
dh-group14, ssh-ed25519 + ecdsa-sha2-nistp256 + rsa-sha2-512/256 host keys, chacha20-poly1305@openssh.com +
aes256-gcm + aes256-ctr, hmac-sha2-256/512 (+ETM), zlib@openssh.com s2c, password + publickey auth.

- [x] **Post-quantum hybrid key exchange - SSH** (SHIPPED v6.6.0, `DETWS_ENABLE_PQC_KEX`) - the **headline
      gap is closed for SSH**. Shipped a constant-time, zero-heap **ML-KEM-768** core (FIPS 203, the novel
      primitive: software NTT over q=3329 + a Keccak/SHA-3/SHAKE sponge) in `network_drivers/presentation/pqc`,
      Encaps-only (the device is always the responder). The SSH transport advertises **mlkem768x25519-sha256**
      (draft-ietf-sshm-mlkem-hybrid-kex) first, runs the existing X25519 alongside ML-KEM-Encaps, and combines
      `K = SHA256(K_PQ || K_CL)` (RFC 9370). Byte-exact vs the FIPS 203 reference (kyber-py) + a decisive
      end-to-end host test (independent ML-KEM Decaps client, string-K exchange hash / KDF, ed25519 signature).
      Peak ~7 KB worker stack (`DETWS_WORKER_STACK_PQC_MIN`). _Pending:_ interop vs real OpenSSH 9.9+ on the S3 rig.
- [x] **Post-quantum hybrid key exchange - HTTP/3 / TLS 1.3** (SHIPPED v6.7.0) - wired the same ML-KEM-768
      core into the QUIC hand-rolled TLS 1.3 as the **X25519MLKEM768** group (IANA 0x11ec): added the group to
      `tls13_msg` supported_groups + key_share (client share = ek(1184) || X25519(32); server share =
      ciphertext(1088) || X25519(32) - ML-KEM first per draft-ietf-tls-ecdhe-mlkem), concatenated the two secrets
      (ML-KEM || X25519, 64 B) into the `tls13_kdf` handshake secret, and enlarged the QUIC ServerHello flight
      buffer for the ~1.1 KB key_share. A PQC-capable browser negotiates it; others fall back to X25519. Verified
      by a full hybrid handshake host test (independent ML-KEM Decaps client + both Finished MACs). The classical
      HTTPS path stays on mbedTLS (whose hybrid support tracks its version). _Pending:_ curl/browser HW interop.
- [ ] **PQC hybrid follow-ons** (S) - **sntrup761x25519-sha512@openssh.com** for SSH parity with OpenSSH's
      other default, and a **HelloRetryRequest** path so a client that lists X25519MLKEM768 but sends only a
      classical key_share still gets the hybrid (today we serve whatever key_share it sent).
- [x] **SSH cipher / host-key breadth** _(shipped)_ - closed the easy deltas vs CycloneSSH:
    - [x] **`aes256-gcm@openssh.com`** _(shipped)_ - AES-256-GCM AEAD (RFC 5647): length-in-clear as AAD,
          per-packet invocation counter, no separate MAC. Self-contained `ssh/crypto/ssh_aesgcm` (AES block HW
          via mbedTLS on ESP32, GHASH in software; AES-256, so not the AES-128 `http3/quic_aead` core). Seal/open
          KAT vs the NIST/McGrew Test Case 16 vector + a packet-layer round-trip; negotiated second after chacha.
    - [x] **`rsa-sha2-512`** host-key signatures _(shipped)_ - RFC 8332 SHA-512 PKCS#1 v1.5 over the exchange
          hash, hash-parameterized `ssh_rsa_sign`/`_verify` (SHA-512 DigestInfo). The one `ssh-rsa` key now
          advertises both rsa-sha2-512 (preferred) and rsa-sha2-256 in `server_host_key_algorithms` and
          `server-sig-algs`; client-auth picks the verify hash from the signature name. SHA-512 KAT byte-exact
          vs openssl + hash-binding, e2e KEXDH reply, and a genuine client-auth signature all tested.
    - [x] **`ecdsa-sha2-nistp256`** host key + client auth _(shipped)_ - ECDSA over NIST P-256 (RFC 5656,
          SHA-256). New `ssh/crypto/ssh_ecdsa`: ESP32 mbedTLS (HW, side-channel-hardened); native is a
          self-contained software P-256 (field/Jacobian point math) with RFC 6979 deterministic signing, so
          the sign path is byte-exact against the RFC 6979 A.2.5 vectors. Installed via `ssh_hostkey_ecdsa_set()`;
          negotiated in `server_host_key_algorithms` + `server-sig-algs`; the KEXDH reply carries the P-256 blob
          and an `mpint(r)||mpint(s)` signature; client-auth verifies. KAT + e2e KEXDH + genuine client-auth
          all tested. _(aes128-gcm skipped: 256 is the ESP32-relevant one.)_
    - [x] **`ecdh-sha2-nistp256`** key exchange _(shipped)_ - completes RFC 5656 (its §4 ECDH alongside the §3
          ECDSA host key). The shared secret is the X coordinate of `d_S * Q_C`, hashed as an mpint with
          `Q_C`/`Q_S` as 65-byte point strings; the peer point is on-curve checked. Reuses the one P-256 curve
          in `ssh/crypto/ssh_ecdsa` (ESP32 `mbedtls_ecdh_compute_shared`, HW; native software P-256), added
          `ssh_ecdsa_p256_ecdh()`. Negotiated in `kex_algorithms`. Byte-exact vs the RFC 5903 §8.1 shared-secret
          KAT + a negotiation test + an e2e KEXDH that verifies the host signature over the reconstructed hash.
- [x] **SFTP + SCP subsystem over SSH** (XL, high value - ties to G-code deployment) _(done, HW-verified)_ -
      Cyclone has both SFTP and SCP client+server; we now have the **server** side. `DETWS_ENABLE_SSH_SFTP`
      (services/sftp + server/ssh_sftp) - an SFTP v3 server subsystem (`SSH_FXP_*` over a session channel) with
      a fixed-BSS handle table, streamed reads/writes, no heap, serving an `fs::FS` mount via
      `det_ssh_sftp_begin(fs, root)`: OPEN/READ/WRITE/OPENDIR/READDIR/STAT/MKDIR/RMDIR/REMOVE/RENAME/REALPATH,
      the `..` traversal guard (server/fs_path). `DETWS_ENABLE_SSH_SCP` (services/scp + server/ssh_scp) - the
      rcp SINK direction (`scp file device:/path`) via `exec "scp -t"`. The standards-track southbound path for
      **secure machine-agent G-code push** (drop NC programs on the device, or drip them to a controller via
      `services/dnc`) over the one authenticated SSH port. Pure codecs host-tested (native_ssh_sftp, native_scp);
      HW-verified vs the OpenSSH `sftp`/`scp` clients on an ESP32-S3 + SD card (60 KB byte-exact). The HW run
      caught + fixed a latent SSH transport bug (a TCP read carrying several pipelined packets - a large SFTP
      write - overflowed the single-packet receive buffer; now drained incrementally). _Follow-ups:_ the SCP
      SOURCE (download) direction, recursive SCP, and a graceful `SSH_MSG_DISCONNECT` on teardown.
- [ ] **TLS Raw Public Keys (RFC 7250)** (M) - Cyclone supports RPK; a cert-less TLS credential (bare
      SubjectPublicKeyInfo, no X.509 chain) is a natural fit for constrained, provisioned ESP32 fleets where a
      pinned key beats a full PKI - smaller handshakes, no cert parsing. Expose/enable it on the TLS path.
- [x] **DTLS 1.3** (L) _(full server stack + CoAP-over-DTLS front-end shipped)_ - Cyclone does DTLS 1.0/1.2/1.3;
      we have QUIC (HTTP/3) but no DTLS service. DTLS 1.3 secures CoAP-over-UDP and other datagram telemetry
      without a TCP+TLS session - the missing secure UDP transport for the IoT-device families. Layers on the
      existing TLS 1.3 record/handshake crypto. **Shipped (`DETWS_ENABLE_DTLS`, network_drivers/presentation/dtls):**
      the RFC 9147 §4 record layer (dtls_record) - DTLSPlaintext + DTLSCiphertext (unified header,
      AEAD_AES_128_GCM, §4.2.3 sequence-number encryption via an AES-ECB mask + §4.2.2 reconstruction, the
      TLS 1.3 nonce, §4.5.1 anti-replay window); the §5 + §7 handshake framing / reliability (dtls_handshake) -
      the 12-byte handshake header, overlap-tolerant reassembly, the ACK message, and the stateless
      HelloRetryRequest cookie; and the **§5-6 server handshake state machine** (dtls_conn) - the one-round-trip
      full handshake (TLS_AES_128_GCM_SHA256 / X25519 / Ed25519), epoch 0→2→3 transitions, reusing the TLS 1.3
      messages + key schedule. Byte-exact host KATs (`native_dtls`, `native_dtls_hs`, `native_dtls_tls13`) plus
      an end-to-end handshake proof (`native_dtls_conn`: a from-scratch peer completes it, both sides install
      matching app keys) and **wolfSSL DTLS 1.3 interop** (direct + HelloRetryRequest group renegotiation).
      **HelloRetryRequest** group renegotiation (§5.1, address-bound cookie) and **ACK/timeout retransmission**
      (§5.8, fresh-seq flight re-send on exponential backoff) are shipped, and the **CoAP-over-DTLS front-end**
      is complete - the `coaps_process` bridge plus `coaps_server`, a per-peer DtlsConn pool binding UDP 5684
      (`coaps://`), driven by one `coaps_server_poll()` (`native_coaps` / `native_coaps_server`; example
      78.CoapSecure). **Connection IDs** (§9 / RFC 9146) are shipped too - the connection_id extension is
      negotiated in the handshake, every DTLSCiphertext carries the peer's CID, and `coaps_server` routes by
      CID so a session survives a client address change / NAT rebinding (`native_dtls*` + wolfSSL `--cid`
      interop). _Remaining:_ full HW-rig interop.
- [ ] **SSH keyboard-interactive auth (RFC 4256)** (S) - Cyclone supports it; a minor addition to our
      password + publickey methods (enables server-driven MFA prompts / password-change flows over SSH).

## Storage & config

- [x] Typed NVS config store _(shipped)_.
- [x] Partition-map status monitor endpoint _(shipped)_ - `DETWS_ENABLE_PARTITION_MONITOR`: `detws_partition_monitor_begin()` serves the flash partition table (label, kind, type/subtype, offset, size, running app slot) as JSON via `esp_partition` / `esp_ota_ops`; kind classifier + serializer host-tested (example 64.PartitionMonitor).
- [x] Config export / restore _(shipped, schema-driven)_ - `DETWS_ENABLE_CONFIG_IO`: `services/config_io` serializes a declared schema of fields to a portable `key=value` text blob and parses one back into the NVS config store - backup / migrate / bulk-provision, deterministic and zero-heap (host-tested round-trip; example 71.ConfigExport). Remaining (M): full enumeration-based export (needs NVS key iteration), ZTP multi-stage provisioning.
- [x] Unified VFS wrapper _(shipped)_ - `DETWS_ENABLE_VFS`: `services/vfs` gives one file API (open/read/write/close, exists/size/remove/rename, whole-file helpers) over a pluggable backend - a zero-heap RAM pool (deterministic, host-tested) or a real `fs::FS` (LittleFS / SD / SPIFFS) on ESP32 - so features target storage without knowing the medium (example 80.Vfs).
- [~] **Wear leveling** + log offload (server/SD) (M) _(wear-leveling shipped)_ - `DETWS_ENABLE_WEARLEVEL`:
  `services/wearlevel` `detws_wearlevel_pick()` returns the least-worn slot (from per-slot write
  counts the app persists) so repeated flash/NVS writes spread evenly and the region ages together
  instead of burning out one block, plus a `detws_wearlevel_spread()` imbalance metric. Pure core,
  host-tested (`native_wearlevel`; a 4000-write pick+mark loop levels every slot exactly). Log
  offload rides the existing `services/logbuf` pluggable sink (SD / syslog / HTTP). _Remaining:_
  hot-swap storage safeties (M).
- [x] OTA rollback protection + soft-brick safeguard _(shipped)_ - `DETWS_ENABLE_OTA_ROLLBACK`: `services/ota_rollback` commits a freshly-updated image once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses, so a bad update self-heals; decision logic pure + host-tested, commit/rollback via esp_ota_ops, HW-verified (example 73.OtaRollback). Remaining: modular partition swapping (M).
- [~] PSRAM web buffers / zero-copy net buffers (`heap_caps_calloc(MALLOC_CAP_SPIRAM)` at begin) + asset
  offloading + COMPONENT_EMBED_TXTFILES (M); SPI DMA ping-pong buffers (M) _(placement policy shipped)_ -
  `DETWS_ENABLE_PSRAM_POOL` (`services/psram_pool`): `detws_psram_place` picks DRAM vs PSRAM for a buffer
  by size, DMA requirement, and free-heap headroom (large/cold to PSRAM, small/hot + DMA to DRAM, always
  leaving an internal-DRAM reserve), and `detws_pingpong_*` keeps the SPI DMA double-buffer bookkeeping
  (CPU fills one buffer while DMA drains the other; swap flips their roles). Pure, host-tested
  (`native_psram_pool`). _Remaining:_ the `heap_caps_calloc` allocation glue at begin + the
  COMPONENT_EMBED_TXTFILES asset-offload build wiring (M).

## Observability, diagnostics & reliability

- [x] Immutable audit logs _(shipped)_ - tamper-evident SHA-256 hash chain in a fixed RAM ring (moving anchor keeps the retained window verifiable), with a pluggable sink for store-and-forward to SD / syslog / an HTTP log service. (FDA 21 CFR Part 11 attestation/e-sign workflow still open.)
- [x] Rotating log buffer + severity traps _(shipped)_ - `DETWS_ENABLE_LOGBUF`: `services/logbuf` keeps the last `DETWS_LOG_LINES` lines in a fixed RAM ring (oldest pruned, no heap), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback on lines at/above a severity threshold (forward critical lines as an SNMP trap / webhook); pure + host-tested (example 70.LogBuffer).
- [~] Core dump to SD/FTP + live exception-decoder panel (M) _(decoder shipped)_ -
  `DETWS_ENABLE_EXC_DECODER` (`services/exc_decoder`): `detws_exc_parse` extracts a captured Guru
  Meditation panic dump (cause, register PC + EXCVADDR, and the `Backtrace: pc:sp ...` frames, tolerating
  missing fields + a trailing `|<-CORRUPTED`) into a structured ExcInfo, and `detws_exc_json` serializes
  it for a live `/exception` panel; the browser / build server resolves the PCs to file:line against the
  firmware ELF (addr2line lives off-device). Pure, host-tested (`native_exc_decoder`). _Remaining:_ the
  core-dump-partition read + SD/FTP offload transport, and zero-overhead abstract logging (M).
- [x] Runtime heap/stack guardrails _(shipped)_ - `DETWS_ENABLE_GUARDRAILS`: `services/guardrails` samples free heap, the heap low-water mark, the largest free block (fragmentation), and a task's remaining stack, and fires a breach callback when any crosses its `DETWS_GUARDRAIL_*` floor - a proactive fail-safe hook on top of the passive /metrics numbers; evaluator + JSON host-tested, served at `/health` (example 69.Guardrails).
- [x] Fail-safe safe-state + deadlock-detection WDT + watchdog-protected coroutine lifelines (M)
      _(shipped)_ - `DETWS_ENABLE_FAILSAFE`: `services/failsafe`, a software watchdog. Register a
      "lifeline" (a task / worker / control loop) that must check in (`detws_failsafe_feed`) within its
      deadline; `detws_failsafe_check()` detects one that wedged (hang / deadlock) via a wrap-safe time
      delta and fires a breach callback **once per stuck episode** so the app drives a known-safe state.
      App-defined and per-lifeline, on top of the hardware task WDT; the pure core takes an explicit
      `now` so it is fully host-tested (`native_failsafe`), plus a `/health`-style JSON serializer.
      Zero heap, no stdlib.
- [~] Hardware health (M): power-rail voltage-drop logger, SPI-bus CRC audit + clock backoff, GPIO
  short-circuit test, capacitor-leakage diag _(decision cores shipped)_ - `DETWS_ENABLE_HW_HEALTH`
  (`services/hw_health`): `detws_hwhealth_rail_sample` tracks a rail's worst droop + sag/brownout counts
  (with a `/health` JSON serializer), `detws_hwhealth_spi_result` is a hysteretic clock-backoff state
  machine (halve on a CRC-fail streak, step up on an ok streak, clamped to a floor/ceiling),
  `detws_hwhealth_gpio_short` classifies a driven-vs-readback mismatch as a short to ground / Vcc, and
  `detws_hwhealth_cap_leak` compares a measured RC decay to expected (too fast = leak, too slow = high
  ESR). Pure, host-tested (`native_hw_health`). _Remaining:_ the ADC / SPI / GPIO sampling glue that
  feeds them on real hardware (M).

## Build / tooling

- [x] Runtime build-flag reporter _(shipped)_ - `server.diag()` / `DETWS_ENABLE_DIAG` serves a build-info JSON (example 42.Diagnostics); the feature enumeration could be extended.
- [~] Hierarchical build-flag tree (M); virtual protocol-mocking toggles (M) _(tree shipped)_ - the
  `docs/utilities/gen_configurator.py` generator parses `src/ServerConfig.h` and builds the flags
  as a hierarchy grouped under the file's section titles, with the hard dependency edges (`#if child &&
!parent` guards) encoded - the build-flag tree, kept from drifting by the `check` CI gate. _Remaining:_
  the virtual protocol-mocking toggles (swap a real driver for a mock transport at build time).
- [~] **Real-protocol interop test harness** (M, per protocol) - _harness shipped_ in
  [test/servers/](../test/servers/): one CLI,
  `python test/servers/interop.py <protocol>`, drives the device against the _real_
  reference implementation, not just our own round-trip. Peers so far: HTTP (stdlib
  client), WebSocket (`websockets`), SNMP (`net-snmp`, `pysnmp` fallback), Modbus
  client + server (`pymodbus`), CoAP (`aiocoap`), MQTT broker (`mosquitto` + `paho`),
  OPC UA client + server (`asyncua`). Each peer says whether the device is the server
  (the harness probes it, `--host ...`) or the client (the harness serves a reference
  peer the device connects to), reports uniform PASS/FAIL, and exits 0/1/2.
  HW-verified against the board on real third-party stacks: HTTP 4/4, WebSocket 3/3,
  CoAP 2/2, Modbus 6/6, SNMP 3/3, MQTT (device client -> real `mosquitto`), and OPC UA
  (`asyncua`) 3/3 - all seven protocol families. Adding a protocol is one module in
  `peers/` (documented in its README). Remaining: wiring it into CI containers, and a
  peer per new protocol as it lands.
- [x] **Pentesting / adversarial suite** _(shipped)_ - a separately-runnable harness (env `native_pentest` + a nightly `Pentest` CI job, _not_ part of the per-commit unit-test run) that fuzzes the untrusted-input parsers (HTTP request line/headers/body, Modbus ADU, base32) with malformed, oversized, partial slowloris-style, binary/protocol-confusion, and deterministically-random input, asserting the device's safety invariants: fixed footprint (no buffer index past its bound), fail-closed (defined error states only), and liveness (no hang/over-read). Plus a documented on-device stress playbook (slowloris / floods / brute-force vs the throttle / lockout / allowlist defenses). Full guide: [PENTEST.md](PENTEST.md). Extend it to the remaining codecs (CBOR / SNMP / CoAP / WS / multipart) as you go.
- [~] **Server build configurator (CLI + GUI in `configurator/`)** (L) _(GUI + source-of-truth shipped)_ - a guided front end for the ~200 `DETWS_ENABLE_*` and sizing flags so a user assembles a firmware build without hand-editing `build_flags`. Shipped as `docs/utilities/gen_configurator.py` -> `docs/configurator.html`: it parses [ServerConfig.h](../src/ServerConfig.h) (the single source of truth, so it never drifts - a `check` CI gate fails on staleness) for every feature flag + tuning knob + section group + the hard `#if child && !parent` dependencies, and emits one self-contained page that ticks features, tunes knobs, resolves dependencies (mutual-exclusion), and copies out a `platformio.ini` `build_flags` block or a `#define` set (only the values that differ from the defaults). Beginner-friendly, ships to Pages. _Remaining:_ a live per-option build-footprint estimate (flash + RAM from the FEATURES tables), advisory "this is unwise, but here you go" guardrails, and a standalone CLI backend (the emission is client-side today).

## Protocol & transport versions

The big-ticket transport/protocol upgrades, sequenced last. Each is large (L) and
gated on the internal-piping straightening (a clean transport read/write API) so a
new framing/record layer slots in behind one owner instead of threading through
every layer. The current HTTP/1.1 core already tracks the modern HTTP specs
(RFC 9110 semantics, RFC 9112 messaging - which obsolete RFC 7230/7231).

- [~] **TLS 1.2** (L, RFC 5246) - explicit TLS 1.2 record/handshake support with a
  pinned, audited cipher suite set, session resumption, and the static-pool
  mbedTLS integration; make the negotiated version observable and configurable.
  The record + handshake run in `network_drivers/tls` (mbedTLS, floored at TLS 1.2). The version +
  cipher **policy** shipped as `DETWS_ENABLE_TLS_POLICY` (`services/tls_policy`):
  `detws_tls_negotiate_version` picks the version server-style, `detws_tls_version_name` makes it
  observable, `detws_tls_select_cipher` pins the suite allowlist by server preference, and
  `detws_tls_is_aead` classifies suites - pure + host-tested (`native_tls_policy`). _Remaining:_ wire the
  policy into the mbedTLS conf (cert/cipher list) + session-resumption tickets.
- [~] **TLS 1.3** (L, RFC 8446) - TLS 1.3 handshake (1-RTT, optional 0-RTT early
  data with replay safeguards), modern AEAD-only suites, after TLS 1.2 lands. TLS 1.3 is already
  negotiated by the mbedTLS transport (no max-version cap), and the AEAD-only cipher policy for the
  hardened profile is the shared `services/tls_policy` (`detws_tls_is_aead` + the 1.3 suites 0x1301-0x1303
  in the allowlist). _Remaining:_ optional 0-RTT early data with replay safeguards, and exposing a
  1.3-only profile toggle.
- [x] **HTTP/2** (L, RFC 9113) _(shipped, PSRAM-gated)_ - HPACK header compression
      (RFC 7541), stream multiplexing + flow control, and `h2` ALPN over the TLS
      layer above; streams mapped onto the deterministic per-connection model
      without per-stream heap.
- [x] **HTTP/3** (L, RFC 9114) - QUIC transport (UDP) + HTTP/3 with QPACK
      (RFC 9204); the largest item, **DONE and the trigger for v5.0.0**. The whole stack
      is built, host-tested end-to-end, HW-validated, and **proven against a real
      third-party client**: the codecs (QUIC varint RFC 9000 sec 16, packet /
      packet-number coding sec 17, frame codec sec 19, HTTP/3 framing RFC 9114 sec 7,
      QPACK RFC 9204); the QUIC packet crypto (RFC 9001 - HKDF-Expand-Label key
      schedule, AEAD_AES_128_GCM, header protection, Retry tag); a hand-rolled TLS 1.3
      handshake (RFC 8446, TLS_AES_128_GCM_SHA256 + X25519 + Ed25519, pinned to the
      RFC 8448 traces - mbedTLS has no QUIC-TLS API); the transport-parameters codec
      (sec 18); the stateful QUIC v1 connection engine (per-level AEAD, CRYPTO / STREAM
      reassembly, ACKs, coalescing, HANDSHAKE_DONE); the HTTP/3 application engine
      (control + QPACK streams, SETTINGS, request -> response); the UDP-facing
      `quic_server` pool (DCID routing, ingest ring); and the DetWebServer bridge that
      serves HTTP/3 through the same routes as HTTP/1.1 and HTTP/2 (`server.h3_cert(...)` + `begin()`). **curl 8.14.1 (OpenSSL 3.5.6 QUIC + nghttp3) completes the handshake
      and serves GET / POST at ~14 ms/request** (see `tools/interop/`); the ESP32-S3
      mbedTLS hardware-crypto path is byte-identical to the software path. Remaining as
      v5.x hardening (not correctness blockers): PTO loss recovery, CONNECTION_CLOSE on
      idle / error, and the PSRAM build guard for a device deployment.

### Supporting HTTP specs (smaller, fold in alongside the above)

- [x] **Cookies** (M, RFC 6265) _(shipped)_ - emission via `set_cookie()` (name/value
      plus a freeform attributes string for `Secure` / `HttpOnly` / `SameSite` /
      `Max-Age` / `Expires` / `Path` / `Domain`), and inbound reading via
      `http_get_cookie(req, name, out, cap)` - parses the request `Cookie` header
      (case-sensitive names, DQUOTE-stripped values, `=` preserved in values),
      mirroring `http_get_header()`. Host-tested (`test_http_parser`, 4 cookie cases);
      pairs with the session / CSRF / auth features.
- [x] **HTTP caching** (RFC 9111) _(shipped)_ - conditional GET on served files via
      `DETWS_ENABLE_ETAG`: a strong `ETag` + `Last-Modified`, and `304 Not Modified`
      for a matching `If-None-Match` or (when no `If-None-Match`) an `If-Modified-Since`
      not older than the file; plus `Cache-Control` via `set_cache_control()`. Remaining
      (optional): response-side freshness heuristics / `Age`, request `Cache-Control`.
- [~] **Forwarded header** (S, RFC 7239) - _parser shipped._
  `http_forwarded_client()` recovers the original client address + scheme from `Forwarded`
  (RFC 7239) or the de-facto `X-Forwarded-For` / `X-Forwarded-Proto` (leftmost = the
  client). Both IPv4 (optional `:port`) and IPv6 (bracketed `for="[2001:db8::1]:port"` or a
  bare `X-Forwarded-For` literal) are recovered and canonicalized (RFC 5952); the candidate
  is validated with `det_ip_parse`, so `unknown` / obfuscated `_id` tokens are rejected.
  Host-tested (`test_http_parser`, 5 cases). It is a helper (like `http_get_cookie`) the app
  reads in a handler; auto-wiring the recovered IP into the per-IP auth lockout / audit,
  gated on a configured trusted upstream (the header is client-spoofable), is the optional
  remaining step. The IP allowlist stays accept-time (the proxy's real TCP source).

## Maintenance

- [x] **API directional symmetry pass** _(audited)_ - swept the paired ingress/egress
      surfaces (codec encode/decode, transport read/send, server/client, HTTP
      request/response, the protocol-service parse/build pairs) against the rule that a
      value at a position on ingress sits at the same position on egress. Result: the
      surfaces are symmetric, with one real fix applied - the egress transport API
      (`det_conn_send` / `det_conn_sndbuf` / `det_conn_flush`, plus the private
      `resp_end`) no longer threads a `tcp_pcb *`; it resolves the slot's pcb
      internally, exactly as the RX read path and the client surface already do, so a
      caller can no longer pass a pcb that disagrees with the slot. The remaining LOW
      findings (CBOR `text` vs MessagePack `str`; builder-returns-length vs
      parser-returns-bool) are spec-faithful, consistent house style - left as is.
- [x] **Refresh build footprints** (S) _(automated)_ - the per-feature / per-example flash + RAM
      footprint tables now track the current build automatically: the **ESP32 Build** workflow
      (`.github/workflows/esp32-build.yml`) rebuilds every example on each push to main, merges the
      totals into `docs/footprints.json`, regenerates `docs/FOOTPRINTS.md` and the per-feature ranges
      (`tools/ci/example_footprints.py` + `feature_budget.py`), and commits them
      (`docs: update ESP32 build footprints [skip ci]`). So the documented numbers stay current after a
      batch of changes with no manual step - the opt-in codec services added since track through their
      flags, and the auto-commit lands right after each release bump.
- [x] **Audit the library against its standards** (L) _(audit performed + tracked)_ - every standard in
      [STANDARDS.md](STANDARDS.md) was reviewed against its key MUST / SHOULD requirements, with the
      verdict and the backing evidence recorded per standard in **[AUDIT.md](AUDIT.md)**. The audit
      leverages the strongest available evidence: the major protocols (HTTP, WebSocket, CoAP, MQTT,
      Modbus, SNMP, OPC UA, SSH) are each verified against a **real reference implementation** in the
      interop harness, and the crypto primitives are pinned to FIPS/RFC test vectors. No open conformance
      defects were found in this pass (the HTTP request-smuggling and CoAP-Observe-clock fixes from earlier
      reviews are regression-tested); the remaining `🔷` entries are documented scope boundaries tracked
      here, not violations. The audit is re-run per subsystem change - the verdict + evidence columns are
      kept current.

## Low-level networking (raw Layer 2)

The ESP32 _does_ expose raw Layer-2 access on both interfaces, and the hard-real-time
cyclic timing is reachable too - so the earlier "PHY can't do raw L2 / can't hit the
timing" caveats are corrected here. **Timing model:** build the device as a
_single feature + the base web server_, and run the protocol's cyclic loop as a
hardware-timer / RMT ISR feeding a **high-priority preempting task** (a top-priority,
core-pinned FreeRTOS task that preempts the web server) off a small interrupt queue.
The web server runs underneath at lower priority; the cyclic deadline owns the CPU
when it fires. With the whole device budgeted to one cyclic loop, the sub-millisecond
isochronous deadlines are achievable - the determinism guarantees (fixed buffers, no
heap, bounded work) are exactly what makes a hard real-time loop tractable here.

**Front-end assumption (applies to every "hardware-gated" item below):** the user can
attach an external adapter on **SPI / UART / I2C** to supply any missing PHY, radio,
or line transceiver - so the deliverable is always the protocol + a small adapter
driver, never "impossible." Canonical examples: an **EtherCAT Slave Controller**
(e.g. LAN9252) over SPI, a **HART FSK modem** over UART, a **DSRC / C-V2X radio**
module over SPI/UART, an **IO-Link master IC** over SPI, **RS-485 / CAN / SDLC /
MBP / LON** transceivers, etc. **Analog I/O** is native: a **4-20 mA** current loop
(or 0-10 V) is read straight off the ESP32 **ADC** over a known-range sense resistor
(scale = known full-scale range), and driven out via the DAC / PWM - so analog
instrument variables (incl. HART's 4-20 mA primary value) need no special front end.

- [~] **Raw L2 frame TX/RX** (M, platform enabler) _(Ethernet frame codec shipped)_ -
  `DETWS_ENABLE_RAWL2` (`services/rawl2`): the host-testable core - build/parse Ethernet II + 802.1Q
  VLAN frames and the 802.3 FCS (CRC-32, check value 0xCBF43926); host-tested (`native_rawl2`). The
  raw-frame API the raw-L2 protocols build on. _Remaining (device transport):_
    - **Wi-Fi (802.11):** `esp_wifi_80211_tx()` injects arbitrary management / data
      frames (custom beacons, proprietary MAC headers) - bypasses the Wi-Fi state
      machine. Also the basis for the pentest/observability angle (beacon/mgmt crafting).
    - **Wired (802.3):** `esp_eth_transmit()` + `esp_eth_update_input_path()` detach the
      Ethernet MAC from lwIP so the app reads/writes raw Ethernet frames at L2 directly -
      this is what makes PROFINET-RT / EtherCAT / GOOSE / POWERLINK / SERCOS framing
      reachable - the cyclic deadline is met by the high-priority preempting-ISR/task
      model above (single-feature build), not blocked by the platform.
    - **L2 transparent bridge:** capture raw L2 on Wi-Fi and map onto the wire via the
      Ethernet MAC (and back) so the device acts as an unmanaged Wi-Fi-to-wired L2 switch.
      Fixed BSS frame buffers, no heap; one build flag, off by default (raw injection is
      powerful - keep it opt-in).

## Industrial / standards protocols

- [x] **umati / OPC UA for Machine Tools** (M, OPC 40501-1) _(MachineTool model shipped)_ -
      `DETWS_ENABLE_UMATI` (`services/umati`, requires `DETWS_ENABLE_OPCUA`): the umati companion-spec
      MachineTool information model - Identification, Monitoring (MachineTool / Channel / Spindle /
      Axis_X..Z), Production, and Notification - served through the OPC UA Browse + Read resolvers out of a
      caller-owned `UmatiMachineTool` struct, with faithful BrowseNames per OPC 40501-1 (namespace
      `http://opcfoundation.org/UA/MachineTool/`). Host-tested (`native_umati`, 11/11) and ESP32-compiled
      (example 72.Umati); a read-only monitoring model any umati / OPC UA client browses and reads by
      BrowseName. _Remaining:_ companion-spec TypeDefinitions + the MachineTool namespace in the server
      NamespaceArray (needs array-Variant support), multiple channels/spindles, and Equipment (tool magazine).
- [~] **MTConnect** (L, ANSI/MTC1.4-2018) _(streams/error response codec shipped)_ -
  `DETWS_ENABLE_MTCONNECT` (`services/mtconnect`): the MTConnect agent response documents over the
  existing HTTP stack - an incremental **MTConnectStreams** builder (the `current` / `sample`
  response: header with instanceId + nextSequence, then per-DataItem `<Samples>` / `<Events>` /
  `<Condition>` observations) and the **MTConnectError** document, XML-escaped into a caller buffer;
  host-tested (`native_mtconnect`). _Remaining:_ the `probe` **MTConnectDevices** doc, `MTConnectAssets`,
  the fixed-capacity Devices->Components->DataItems model + the circular sample buffer keyed by
  instanceId + sequence (the `from`/`count`/`interval` long-poll semantics). Pairs with gpio_map /
  dashboard as DataItems.
- [~] **Industrial Ethernet** (XL, EtherNet/IP, PROFINET, EtherCAT) - _EtherNet/IP + CIP
  messaging shipped._ `DETWS_ENABLE_ENIP` (`services/enip`): the encapsulation layer (TCP/UDP 44818) - `eip_build` / `eip_parse` (the 24-octet header), `eip_build_register_session`, and
  `eip_build_send_rr_data` / `eip_parse_send_rr_data` (wrap/unwrap a CIP message via the Common
  Packet Format). `DETWS_ENABLE_CIP` (`services/cip`): the CIP message inside it -
  `cip_build_epath` (class/instance/attribute logical segments), `cip_build_get_attr_single` /
  `cip_build_request`, and `cip_parse_response`. Together they form a working CIP read path
  (wrap the CIP request with `eip_build_send_rr_data`); both verified against the Wireshark
  ENIP/CIP dissectors, host-tested. Remaining: the broader CIP object dictionary +
  implicit/IO messaging. **PROFINET** (RT, raw L2
  frames) and **EtherCAT** rely on raw L2, which the ESP32 _does_ provide (see
  Low-level networking above - `esp_eth_transmit` / `esp_eth_update_input_path`), and
  the IRT/isochronous cyclic deadline is met by the high-priority preempting-ISR/task
  model in a single-feature build (this protocol + base web server only). So the full
  cyclic RT/IRT stack is on the table, not just a discovery subset; PROFINET `DCP` is
  the easy first milestone, EtherCAT/IRT the hard one. Each: fixed BSS
  object/process-image model, no heap.
- [~] **Fieldbuses** (L, classic serial/CAN buses) - the pre-Ethernet field protocols
  built on the existing zero-heap codec pattern _(the named buses are shipped)_: **CANopen** (CiA 301:
  object dictionary, PDO/SDO, NMT, heartbeat) over the ESP32 TWAI/CAN controller is
  `DETWS_ENABLE_CANOPEN` (`services/canopen`); **PROFIBUS-DP** is `DETWS_ENABLE_PROFIBUS`
  (`services/profibus`); **DeviceNet** (CIP-over-CAN) is `DETWS_ENABLE_DEVICENET`; Modbus TCP + RTU ship
  as [modbus](../src/services/modbus/) (`DETWS_ENABLE_MODBUS` / `_MODBUS_RTU`). All host-tested behind
  their own build flags. _Remaining:_ the per-bus serial/CAN PHY drivers + HW-over-RS-485/CAN verify
  (tracked per protocol above).
- [~] **Modbus RTU** (M, serial / RS-485) - _codec shipped._ `DETWS_ENABLE_MODBUS_RTU`
  adds `modbus_rtu_process_adu()`: the `[unit-id][PDU][CRC-16/Modbus]` serial frame
  around the existing data model + function-code dispatch - a CRC mismatch or a
  non-matching unit address is dropped silently, a broadcast (address 0) executes
  with no reply. Host-tested (`test_modbus`, 5 RTU cases incl. the 0xCDC5 CRC vector + read round-trip + bad-CRC/wrong-addr/broadcast). The pure codec is fed a complete
  frame; a UART/RS-485 driver (3.5-char inter-frame idle) + HW-over-RS-485 verify are
  the remaining transport step. (Modbus ASCII is a lower-priority follow-on.)
- [~] **PROFINET / PROFIBUS** (XL, Siemens automation) _(PROFINET DCP + PROFIBUS-DP FDL codecs shipped)_ -
  **PROFINET DCP**: `DETWS_ENABLE_PROFINET` (`services/profinet`) - the DCP frame codec (10-octet header +
  option/suboption blocks with even-padding, built by `detws_pn_dcp_header` / `_block`, walked by
  `detws_pn_dcp_walk`) for Identify + Set (assign NameOfStation / IP) over raw L2 (ethertype 0x8892);
  host-tested (`native_profinet`). **PROFIBUS-DP**: `DETWS_ENABLE_PROFIBUS` (`services/profibus`) - the
  FDL telegram codec - SD1 (no-data) and SD2 (variable-data: `SD2 LE LEr SD2 DA SA FC data FCS ED`,
  arithmetic-sum FCS) that a DP master exchanges with slaves over RS-485; host-tested (`native_profibus`).
  _Remaining:_ the PROFINET IO cyclic data exchange + GSDML, and the DP-V0 master state machine + GSD
  slave model. Fixed BSS process image, no heap.
- [ ] **S7comm+** (L, Siemens S7-1200/1500) - the modern S7CommPlus protocol, the successor to the
      classic S7comm (`services/s7comm`, S7-300/400) already shipped, over the same COTP/TPKT transport
      (`services/cotp`): the S7CommPlus PDU framing + the integrity/session handshake (protocol
      versioning + the anti-replay integrity id) and read/write of tag values by symbolic access. Harder
      than classic S7comm (session keying); zero-heap codec first, then the session state machine.
- [ ] **SIMATIC serial family** (M, Siemens legacy point-to-point) - the pre-Ethernet SIMATIC link +
      messaging protocols as zero-heap codecs, pairing with the existing S7comm/COTP stack for the full
      Siemens set: **3964(R)** (the byte-oriented point-to-point link protocol - STX/DLE/ETX framing with
      priority arbitration and a BCC block-check byte on the "R" variant), **RK512** (the message /
      interpreter layer carried over 3964R - FETCH/SEND telegrams addressing DB/data blocks, words and
      bits), plus the **MPI** (Multipoint Interface bus) and **PtP IF** (point-to-point CP serial
      interface) link layers. Serial-first (RS-232/RS-485 transport step tracked with the other serial buses).
- [x] **Fanuc FOCAS** (L, CNC data collection) - SHIPPED (`services/focas`, `DETWS_ENABLE_FOCAS`). The
      FANUC Open CNC API's Ethernet wire protocol (TCP 8193) as a zero-heap client codec - no `fwlib32`
      (the licensed FANUC binary, part # A02B-0207-K732/K737, which cannot run on an ESP32) is used or
      required. `focas_build_*` emit the on-wire frames (a 10-octet big-endian magic/version/type/length
      envelope + payload) for the open/close handshake and the documented read functions (SysInfo, alarm
      status, CNC parameters, macro variables, position/axis data, actual feed / spindle), and
      `focas_parse_*` decode the responses (echoed selector + FOCAS return code + data), including ODBSYS
      and the FANUC 8-octet `data / base^exp` value encoding. Frame layout, selector set, and value
      decoding reverse-engineered vs `diohpix/pyfanuc`; host-tested (`native_focas`, 11/11) and
      **byte-cross-checked against an independent Python reference** (`test/servers/peers/focas_peer.py`,
      which also serves a mock FANUC CNC for on-device interop). HW verification against a real FANUC
      control pending a rig. Fixed BSS, no heap. Refs: pyfanuc, node-red-contrib-fanuc-focas.
- [x] **Beckhoff ADS / AMS** (L, TwinCAT) - SHIPPED (`services/ads`, `DETWS_ENABLE_ADS`). The
      Automation Device Specification over TCP 48898: the AMS/TCP + AMS header (target-before-source
      AMSNetId + port, command id, state flags, cbData, invoke id) plus the ADS commands (ReadDeviceInfo,
      Read, Write, ReadWrite, ReadState, WriteControl, Add/Delete/DeviceNotification) that read + write PLC
      symbols by index group/offset or by handle (name -> handle via 0xF003, value via 0xF005). Header
      layout + command ids verified against the Beckhoff InfoSys spec; payloads cross-checked with
      Beckhoff's ADS library, `pyads`, and Apache PLC4X. Host-tested (`native_ads`), example 73.AdsClient.
      Fixed BSS, no heap.
- [ ] **Heidenhain LSV/2** (M, CNC DNC) - the LSV/2 protocol Heidenhain TNC controls use for DNC + data
      access (block-numbered telegrams with a BCC, file transfer + run/status inspection). A CNC-native
      southbound source for the common European control, over serial and Ethernet.
- [ ] **EUROMAP 77 / 83** (M, OPC UA for plastics) - the injection-moulding / rubber-machine OPC UA
      companion specs (EUROMAP, euromap.org NodeSet), the plastics analogue of **umati**. Layers on the
      OPC UA server exactly like `services/umati` (the machine/mould/job model through the Browse + Read
      resolvers out of a caller struct) - reuses the umati pattern, low risk, high value for moulding shops.
- [ ] **CiA 402 drive profile** (M, IEC 61800-7-201) - the standardised servo/stepper drive + motion
      profile over the existing **CANopen** (`services/canopen`) / EtherCAT-CoE: the state machine
      (Controlword / Statusword), Modes of Operation (Profile Position / Velocity / Torque + Cyclic Sync),
      and the target/actual position-velocity-torque objects. Turns the CAN stack into a motion master;
      pairs with the dshot / ESC actuator set.
- [ ] **PROFIdrive** (L, Siemens drive profile) - the Siemens PROFIBUS/PROFINET drive profile (parameter
      access via PNU, the standard STW1/ZSW1 control/status telegrams + setpoint/actual) over the PROFINET stack.
- [ ] **PackML / OMAC** (M, ISA-88 / OMAC PackTags) - the packaging-machine state model: the PackML state
      machine (Idle/Execute/Held/Suspended/Aborted/...) plus the Admin/Status/Command PackTags, usually
      surfaced over OPC UA - the state engine + tag model as a fixed-BSS service.
- [ ] **Haas Machine Data Collection** (S, serial Q commands) - the documented Haas MDC serial protocol
      (the `Q` command set: Q100 machine serial, Q500 program+status, Q600 macro-variable read) that Haas
      CNC controls answer over RS-232 / the network port. A small, fully-documented CNC read source.
- [ ] **OPC UA Robotics (OPC 40010)** - the VDMA Robotics OPC UA companion spec (the MotionDevice model:
      axes, controller, safety state), another umati-pattern model on the OPC UA server; pairs with the
      Siemens **Run MyRobot** item and **umati** for robot cells.
- [ ] **FANUC robot interop** (M, R-30iB / R-30iA - **next machine-tool step**) - the robot counterpart
      to the shipped FANUC **FOCAS** CNC codec. FANUC robot controllers expose several Ethernet
      interfaces; the tractable, documented ones first: **PROFINET / EtherNet-IP** I/O (already partly
      covered by the shipped `services/enip` + the PROFINET roadmap item - map the robot's assembly
      objects), the **FANUC Robot Ethernet / Stream Motion** UDP position stream, and the **KAREL / R-30iB
      web + socket messaging** (SNPX / the RPC the robot's PC interface uses). Start with a zero-heap codec
      byte-checked against a public reference, then HW-verify against a real controller. Note: "Run MyRobot"
      (above) is the Siemens SINUMERIK route to running a robot from a CNC - a distinct path we also want.
      (Robot cell interop is currently **unimplemented** - this is the tracked entry point.)
- [ ] **Secure machine agent: G-code deployment over a single secure port** (L) - the device as a
      secure local agent that deploys CNC part programs (G-code) to a machine tool over ONE authenticated,
      encrypted port: NC-program transfer/staging multiplexed on the existing TLS endpoint (or a secured
      OPC UA channel / an OPC UA for Machine Tools program-transfer method), so a shop pushes and stages
      programs to controllers through a single connection instead of per-machine FTP/USB. Northbound
      monitoring = **umati** / **MTConnect** / **FOCAS**; southbound program push builds on TLS + file
      upload (`services/upload_service`) and the existing **DNC** drip-feed (`services/dnc`) for controllers
      with no network stack. The unifying goal behind the machine-tool protocol set.
- [~] **DNP3** (L, IEEE 1815) - _data-link frame codec shipped._ `DETWS_ENABLE_DNP3`
  (`services\dnp3`): a zero-heap builder + CRC-validating parser for the data-link layer - `dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC` header block plus the
  CRC'd 16-octet user-data blocks, and `dnp3_parse_frame` validates the header and every
  block CRC (CRC-16/DNP, verified against the canonical 0xEA82 check value) and de-blocks
  the user data. The remaining layers (transport-function segmentation, the application
  objects with groups/variations, Class 0/1/2/3 polling, unsolicited responses,
  time-sync) layer on the de-blocked user data; fixed BSS point database, no heap.
  Optional Secure Authentication (IEEE 1815 SAv5) later.
- [~] **HART** (M, FieldComm) _(frame + HART-IP codec shipped)_ - `DETWS_ENABLE_HART` (`services/hart`):
  the HART command-frame codec - `detws_hart_build` / `_parse` with the longitudinal XOR checksum and
  short (polling) + long (unique-ID) addressing - and the 8-octet **HART-IP** message header
  (version / type / id / status / seq / length) for the front-end-free UDP/TCP 5094 path; host-tested
  against hand-verified vectors (`native_hart`, command-0 checksum 0x82). _Remaining:_ the command set
  (universal + common-practice) + device-variable/status model + burst mode, and the FSK modem
  (over UART) / 4-20 mA-off-the-ADC transports. Fixed BSS, no heap.
- [~] **CC-Link / CC-Link IE** (L, CLPA) _(cyclic frame codec shipped)_ - `DETWS_ENABLE_CCLINK`
  (`services/cclink`): the CC-Link cyclic frame - `detws_cclink_build` / `_parse` for
  `[station][command][RX/RY bit data][RWr/RWw word data][sum checksum]` that a Mitsubishi CC-Link
  master exchanges with remote stations over RS-485, plus the bit/word process-image accessors
  (`get_bit` / `set_bit` / `get_word`); host-tested (`native_cclink`). _Remaining:_ the master
  poll/refresh state machine + the RS-485 timing, and **CC-Link IE Field** (Gbit PHY-gated). Fixed
  BSS station model, no heap.
- [~] **PROFIBUS PA** (M, process automation) _(rides the shipped DP FDL codec)_ - PROFIBUS PA runs the
  same DP-V0/V1 application (the shipped `services/profibus` FDL telegrams) over the MBP (Manchester
  Bus Powered, IEC 61158-2) physical layer for hazardous areas. The FDL/DP telegram layer is done;
  _remaining:_ the PA device profiles (transmitters/valves) + the MBP physical layer (hardware-gated -
  couples to a DP segment via a segment coupler in practice).
- [~] **CANopen** (M, CiA 301) _(message codec shipped)_ - `DETWS_ENABLE_CANOPEN`
  (`services/canopen`): a zero-heap CiA 301 message codec over the shared CAN frame
  (`shared_primitives/can.h`) - NMT, SYNC, heartbeat, EMCY, PDO, and expedited SDO
  read/write/abort plus the COB-ID classifier; host-tested (`native_canopen`, 17 cases).
  _Remaining:_ a first-class object dictionary + node-guarding, the DS401 generic-I/O device
  profile, and the ESP32 TWAI/CAN transport binding. Fixed BSS, no heap.
- [~] **IO-Link** (M, IEC 61131-9) _(data-link codec shipped)_ - `DETWS_ENABLE_IOLINK`
  (`services/iolink`): the SDCI data-link message codec - the MC / CKT / CKS control octets and
  the SDCI checksum (seed 0x52 + the 8->6 compression of IO-Link spec A.1.6), against a
  hand-computed known-answer vector; host-tested (`native_iolink`). _Remaining:_ the on-request
  ISDU parameter service, the IODD-described device model + state machine, and the 3-wire physical
  layer (hardware-gated). Fixed BSS, no heap.
- [~] **POWERLINK** (XL, EPSG) _(basic frame codec shipped)_ - `DETWS_ENABLE_POWERLINK`
  (`services/powerlink`): the EPL basic frames - `detws_epl_build` / `_parse` (and the SoC / PReq /
  PRes convenience builders) for `[messageType][dest][source][payload]`, the four cyclic message
  types (SoC start-of-cycle, PReq poll-request, PRes poll-response with process data, SoA
  start-of-async) that make the isochronous managed-node cycle, over raw L2 (ethertype 0x88AB, on the
  shipped services/rawl2); host-tested (`native_powerlink`). _Remaining:_ the MN slot-schedule state
  machine + async SDO + object dictionary, and the isochronous timing (the preempting-task model).
  Fixed BSS, no heap.
- [~] **SERCOS III** (XL, motion bus) _(telegram + IDN codec shipped)_ - `DETWS_ENABLE_SERCOS`
  (`services/sercos`): the cyclic **MDT/AT** telegram codec - `detws_sercos_build` / `_parse` for
  `[type][phase][cycle][cyclic data]` (the master's setpoint telegram + the drive's actual-value
  telegram) over raw L2 (ethertype 0x88CD, on the shipped services/rawl2) - and the 16-bit **IDN**
  encode/decode (S/P bit + parameter-set + data-block, the "S-0-0100" drive-parameter addressing);
  host-tested (`native_sercos`). _Remaining:_ the hard-real-time slot schedule + the IDN
  service-channel transfer state machine, and the isochronous timing (preempting-task model). Fixed
  BSS, no heap.
- [~] **DeviceNet** (L, CIP over CAN) _(link-adaptation codec shipped)_ - `DETWS_ENABLE_DEVICENET`
  (`services/devicenet`): the CAN link adaptation for CIP-over-CAN - the 4-group 11-bit CAN id, the
  explicit-message header octet, single-frame explicit messages, and the fragmentation reassembler
  (the CIP body itself is built with the shared `cip` codec); host-tested (`native_devicenet`).
  _Remaining:_ the predefined master/slave connection set, I/O (implicit) messaging, and the ESP32
  TWAI/CAN transport. Fixed BSS, no heap; pairs with the EtherNet/IP CIP work.
- [~] **LonWorks / LON** (L, ISO/IEC 14908) _(network-variable codec shipped)_ - `DETWS_ENABLE_LONWORKS`
  (`services/lonworks`): the LonTalk network-variable PDU - `detws_lon_build_nv` / `_parse_nv` for
  `[msg-code][14-bit selector][value]` (an NV update / poll), over **LON/IP** (IEC 14908-4, over UDP,
  so no Neuron/transceiver is needed - the host-reachable path) - plus the common SNVT scalar
  encodings (`SNVT_temp` 0.01 K fixed-point, `SNVT_switch` level + state); host-tested
  (`native_lonworks`). _Remaining:_ the fuller SNVT type table + the NV binding/address table. Fixed
  BSS NV table, no heap.
- [~] **Modbus Plus** (L, HDLC token bus) _(frame + token MAC shipped)_ - `DETWS_ENABLE_MBPLUS`
  (`services/mbplus`): the Modbus Plus HDLC frame codec - `detws_mbplus_build` / `_parse` for
  `[7E][addr][ctrl][payload][CRC-16/X-25][7E]` and `detws_mbplus_next_token` (the next station in the
  logical ring) - reusing the shipped Modbus PDU model for the payload; host-tested (`native_mbplus`,
  CRC check value 0x906E). _Remaining:_ the full path/routing model + the token-rotation timing, and
  the custom 1 Mbit/s bus (hardware-gated).
- [~] **INTERBUS** (L, Phoenix Contact) _(summation-frame codec shipped)_ - `DETWS_ENABLE_INTERBUS`
  (`services/interbus`): the summation frame - `detws_interbus_build` / `_parse` assemble the single
  rotating frame (loopback word + each device's 16-bit process-image slice + CRC-16/CCITT FCS) from a
  list of per-device word slices and disassemble a received frame back into them; host-tested
  (`native_interbus`, FCS check value 0x29B1). _Remaining:_ the PCP parameter channel + the physical
  ring shift-register clocking (hardware-gated). Fixed BSS, no heap.
- [~] **AMQP** (L, OASIS AMQP 1.0 / 0-9-1) - _the 0-9-1 frame codec is shipped._
  `DETWS_ENABLE_AMQP` (`services\amqp`): `amqp_protocol_header` (the `"AMQP" 0 0 9 1`
  preamble), `amqp_build_frame` / `amqp_parse_frame` (type + channel + size + payload +
  the 0xCE frame-end), `amqp_build_method` / `amqp_parse_method` (a METHOD frame's
  class-id / method-id / arguments), and `amqp_build_heartbeat`; host-tested. Remaining:
  the 0-9-1 method-argument field encoding (the connection/channel/exchange/queue/basic
  classes) for RabbitMQ interop, and the **AMQP 1.0** type system + framing (open / begin
  / attach / transfer / disposition) for broker links. Zero-heap: fixed link/session
  state, the payload streamed through the client transport, one build flag. Pairs with the
  MQTT / webhook outbound integrations.
- [~] **DF1 / DH+** (M-L, Allen-Bradley / Rockwell) - _DF1 full-duplex frame codec
  shipped._ `DETWS_ENABLE_DF1` (`services\df1`): a zero-heap framing + DLE byte-stuffing +
  BCC/CRC codec for the serial link layer (pub. 1770-6.5.16) - `df1_build_frame` wraps
  application data in `DLE STX ... DLE ETX` (doubled-DLE escape) with a BCC (2's
  complement of the data sum) or CRC-16/ARC (over the data + ETX, low byte first), and
  `df1_parse_frame` validates and un-stuffs; vectors verified against the manual (BCC
  `0x20`->`0xE0`, CRC `0xBB3D`). Remaining: the **PCCC** command set (PLC-5 / SLC-500
  data-table read/write) inside the application data, the half-duplex master/slave
  framing, and **DH+** (Data Highway Plus token LAN, physical-layer-gated). Fixed BSS,
  no heap.
- [~] **S7comm / S7comm-Plus** (L, Siemens S7) - _ISO-on-TCP framing + the S7comm read
  path shipped._ The TPKT + COTP transport (RFC 1006 / X.224, port 102) is a reusable codec
    - `DETWS_ENABLE_COTP` (`services\cotp`): `tpkt_build` / `tpkt_parse`, `cotp_build_dt`,
      `cotp_build_cr`, `cotp_parse`. **S7comm** itself is shipped - `DETWS_ENABLE_S7COMM`
      (`services\s7comm`): `s7_build_setup` (Setup Communication), `s7_build_read_request` (Read
      Var, S7-ANY items over DB/I/Q/M with the byte-to-bit address encoding), `s7_parse_header`,
      and `s7_read_next_item` (the response data items, honoring the length-in-bits transport
      sizes and the even-item padding); constants verified against the Wireshark dissector,
      host-tested. Remaining: S7comm **write** (function 0x05) + the userdata services, and
      **S7comm-Plus** (the S7-1200/1500 successor with its session/integrity wrapping). Fixed
      BSS data model, no heap.
- [~] **MELSECNET** (L, Mitsubishi) - _MC protocol binary 3E batch-read shipped._
  `DETWS_ENABLE_MELSEC` (`services\melsec`): `melsec_build_read` emits the binary 3E
  batch-read (word) frame (little-endian fields, subheader 0x5000, command 0x0401, the
  device code + 24-bit head device + point count) and `melsec_parse_response` validates
  the 0xD000 response and reports the end code + data; layout + device codes verified
  against a third-party MC impl, host-tested. Completes the major-vendor PLC read set
  (Omron FINS / Host Link, AB DF1, Siemens S7comm). Remaining: MC batch write + the
  1E/4E + ASCII frame variants, and **MELSECNET/H** / **/10** (the cyclic control
  network, PHY/timing-gated). Fixed BSS device model, no heap.
- [~] **FINS** (M-L, Omron) - _FINS/UDP frame codec shipped._ `DETWS_ENABLE_FINS`
  (`services\fins`): a zero-heap command/response builder + parser - `fins_build_command`
  / `fins_build_memory_area_read` emit the 10-octet routing header + MRC/SRC command code + parameters, and `fins_parse_command` / `fins_parse_response` read them back (the
  response MRES/SRES end code included), over the shipped UDP transport. Header layout
  verified against the FINS spec; pure, host-tested. Remaining: the full command set
  (memory-area write / run-stop / clock) and the FINS/TCP framing + the
  FINS/Hostlink-gateway addressing model. Fixed BSS device model, no heap.
- [~] **Host Link** (M, Omron) - _C-mode frame codec shipped._ `DETWS_ENABLE_HOSTLINK`
  (`services\hostlink`): a zero-heap ASCII frame builder + FCS-validating parser for
  Omron's serial C-mode protocol - `hostlink_build` emits `@UU` + header code + text +
  FCS + `*`CR (FCS = the 8-bit XOR from `@` through the text, verified against the
  `@00RD00000010` -> `57` vector), and `hostlink_parse` / `hostlink_end_code` validate and
  split a frame. Pairs with the FINS work (Host Link is the serial sibling); pure,
  host-tested. Remaining: the per-command text encoders (RD/WD/... of the DM/CIO areas)
  and the UART transport. Fixed BSS, no heap.
- [~] **SNP** (M, GE Fanuc Series Ninety Protocol) _(frame codec shipped)_ - `DETWS_ENABLE_SNP`
  (`services/snp`): the SNP master-slave serial frame - `detws_snp_build` / `_parse` for
  `[control][length][data][arithmetic-sum BCC]` with the control-byte constants, for register
  read/write on a GE Fanuc Series 90 (90-30/90-70) PLC over RS-485; host-tested (`native_snp`).
  _Remaining:_ the SNP-X session setup + the per-command register-access encoders, and the UART
  transport. Fixed BSS, no heap.
- [~] **DirectNET** (M, AutomationDirect) _(frame codec shipped)_ - `DETWS_ENABLE_DIRECTNET`
  (`services/directnet`): the DirectNET master-slave serial frames - `detws_dnet_header` (SOH +
  ASCII-hex slave/type/address/blocks + ETB + LRC) and `detws_dnet_data` / `_data_parse` (STX + data + ETX + LRC) - for V-memory read/write on an AutomationDirect DirectLOGIC PLC, with the
  longitudinal-XOR LRC and control-byte constants; host-tested (`native_directnet`). _Remaining:_ the
  ACK/NAK handshake sequencing + the V-memory address map, and the UART transport. Fixed BSS, no heap.
- [~] **IEC 60870-5-101 / -104** (L, power-grid SCADA) _(codec shipped)_ - `DETWS_ENABLE_IEC60870`
  (`services/iec60870`): the tele-control codec - the **-104** APCI (I/S/U frames), the ASDU header + 3-octet IOA, and the **-101** FT1.2 fixed/variable link frames (sum checksum); host-tested
  (`native_iec60870`). _Remaining:_ the fuller ASDU information-object type set (double points,
  measured values, commands) + the k/w sequence flow-control state machine over the shipped TCP
  transport. Fixed BSS point database, no heap.
- [~] **IEC 61850** (XL, substation automation) _(GOOSE publisher + MMS Read shipped)_ - **GOOSE**:
  `DETWS_ENABLE_GOOSE` (`services/goose`) - the raw-L2 multicast event publish for fast trips (the
  BER-encoded IECGoosePdu wrapped in the 8-octet GOOSE header + Ethernet frame, ethertype 0x88B8, on the
  shipped raw-L2 codec; host-tested `native_goose`). **MMS**: `DETWS_ENABLE_MMS` (`services/mms`) - the
  ACSI client/server core, the BER confirmed-request/response **Read** PDUs (invokeID + read service + a
  named Data-Object ObjectName) build + parse, over ISO-on-TCP (TPKT + COTP via the shipped
  `services/cotp`) on port 102; host-tested (`native_mms`). _Remaining:_ GOOSE subscribe/decode + the
  fast-retransmit timer, the fuller MMS service set (Write, GetNameList, reports) + the SCL-driven object
  model. Fixed BSS, no heap.
- [~] **IEEE C37.118** (M-L, synchrophasors) - _frame codec shipped._
  `DETWS_ENABLE_C37118` (`services\c37118`): a zero-heap builder + CRC-validating parser
  for the PMU synchrophasor wire frame (`SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK`,
  CHK = CRC-CCITT) - `c37118_build_frame` frames any payload, `c37118_build_command`
  handles the fixed Command frame, and `c37118_parse_frame` validates the CRC and reports
  the frame type / id / timestamp / payload, with `c37118_parse_command`. CRC verified
  against the canonical CRC-CCITT-FALSE check value; pure, host-tested. The fixed phasor
  configuration / data model (encoding the CFG-2 channel layout and the matching data
  frame, over TCP/UDP 4712/4713, streamed via the chunked / UDP cast path) remains to be
  layered on top. Pairs with the telemetry-math service for on-device PMU analytics.
- [~] **IEEE 2030.5 (SEP 2.0)** (M-L, DER / smart energy) _(resource codec shipped)_ -
  `DETWS_ENABLE_SEP2` (`services/sep2`): the core 2030.5 XML resource documents in the
  `urn:ieee:std:2030.5:ns` namespace - **DeviceCapability** (the root function-set links),
  **EndDevice** (sFDI/lFDI registration), and **DERControl** (a DER dispatch/curtailment event:
  interval + opModFixedW setpoint) - built into a caller buffer over the existing HTTP + TLS stack;
  host-tested (`native_sep2`). _Remaining:_ the fuller function-set resource schema (metering, pricing,
  demand response) + the resource discovery walk + optional EXI, on a fixed BSS model.
- [~] **OpenADR** (L, demand response) _(3.0 event/report codec shipped)_ - `DETWS_ENABLE_OPENADR`
  (`services/openadr`): the OpenADR 3.0 REST/JSON objects - `detws_openadr_event` (a demand-response
  signal: programID + eventName + interval payload points with start / duration / type / value) and
  `detws_openadr_report` (a VEN reading back to the VTN) - built into a caller buffer over the
  existing HTTP client/server + OAuth2, with JSON escaping and a no-stdlib 3-decimal value formatter;
  host-tested (`native_openadr`). _Remaining:_ the full VEN/VTN role state machine + subscriptions,
  and the OpenADR 2.0 XML/EXI profile. Fixed BSS, no heap.
- [x] **SunSpec Modbus** (M, DER device models) _(shipped)_ - `DETWS_ENABLE_SUNSPEC`
      (`services\sunspec`): a zero-heap codec for the SunSpec Alliance register maps layered
      on the holding-register model. A model-chain walker (`sunspec_check_marker` /
      `sunspec_begin` / `sunspec_next_model` - verify the `SunS` 0x53756E53 marker, then
      iterate each model's id / length / body to the 0xFFFF end model) + typed point readers
      (`sunspec_u16` / `_i16` / `_u32` / `_i32` / `_string`) and a map writer
      (`sunspec_write_marker` / `_model_header` / point writers / `_write_end_model`), so a
      solar inverter / meter / battery is interoperable. Marker + header format verified
      against the SunSpec spec; pure, host-tested. Pairs with the shipped Modbus
      ([modbus](../src/services/modbus/)).
- [~] **ICCP / TASE.2** (XL, IEC 60870-6) _(Data_Value codec shipped)_ - `DETWS_ENABLE_ICCP`
  (`services/iccp`): the TASE.2 indication-point Data_Value BER structures - `detws_iccp_state_q`
  (StateQ: a discrete state + quality flags) and `detws_iccp_real_q` (RealQ: a scaled INTEGER value +
  quality), each with an optional 4-octet TimeStamp - the telemetry a control center transfers as MMS
  Reads (on the shipped `services/mms` + `services/cotp`, ISO-on-TCP 102); host-tested (`native_iccp`).
  _Remaining:_ the data-set / transfer-set / bilateral-table object model on top of the MMS ACSI core.
  Fixed BSS object model, no heap.

### Test & measurement / lab instrumentation

Casting the interop net over bench and rack test equipment: a wireless ESP32 becomes a lab-instrument
bridge/controller. SCPI is near-universal across modern instruments and is the highest-value first step;
the LXI transports (VXI-11 / HiSLIP) and a GPIB gateway extend reach to older and higher-throughput gear.

- [ ] **SCPI / IEEE 488.2** (S, universal instrument control - **high value**) - Standard Commands for
      Programmable Instruments, the text command language nearly every modern bench instrument speaks
      (oscilloscopes, DMMs, power supplies, function/arbitrary generators, SMUs, spectrum/network
      analyzers, electronic loads) over a raw TCP socket on **port 5025** (also USBTMC / serial). A
      zero-heap codec: a SCPI command builder (headers, `:` hierarchy, parameter formatting), the IEEE
      488.2 common commands (`*IDN?` / `*RST` / `*OPC?` / `*CLS` / `*ESR?` / `*STB?`), and response
      parsers (numeric, boolean, string, arbitrary block `#<n><len><data>` for waveform captures) + the
      status byte / event-status register / error queue model. Makes the device a bench-instrument
      controller or a wireless bridge that fans instrument telemetry into HTTP/MQTT. Trivially
      host-verifiable against a Python `pyvisa` / raw-socket reference and any SCPI simulator.
- [ ] **HiSLIP** (M, modern LXI transport, IVI Foundation) - High-Speed LAN Instrument Protocol on
      **port 4880**, the current LXI transport that replaced VXI-11: a binary message protocol with
      separate synchronous/asynchronous channels, message-type framing (Initialize / AsyncLock /
      DataEnd / etc.), message-id sequencing, and much higher throughput than VXI-11. Carries SCPI as
      its payload; the codec is the HiSLIP message framing + the two-channel handshake. The right
      long-term LXI transport for high-rate instrument capture.
- [ ] **LXI / VXI-11** (M, ONC RPC instrument channel) - the older LAN eXtensions for Instrumentation
      transport: VXI-11 over ONC/Sun RPC (portmapper **port 111** -> the instrument channel), the
      `create_link` / `device_write` / `device_read` / `device_readstb` / `destroy_link` calls, plus
      the LXI mDNS/VXI-11 discovery beacon. Still the fallback transport for a large installed base of
      LAN instruments that predate HiSLIP; also introduces a reusable ONC RPC codec.
- [ ] **GPIB-over-LAN gateway** (S, Prologix-style) - the Prologix `++`-command GPIB-Ethernet gateway
      control set (`++addr`, `++read`, `++eoi`, `++mode`), so the device drives a bench of legacy
      IEEE-488 (GPIB) instruments through a common Prologix-compatible adapter - the bridge into
      pre-LAN test gear that will never speak SCPI-over-TCP directly.

### Intelligent Transportation Systems (ITS)

- [~] **NTCIP** (L, US ITS) _(object OIDs shipped)_ - `DETWS_ENABLE_NTCIP` (`services/ntcip`): the NTCIP
  object definitions on top of the shipped SNMP agent ([snmp](../src/services/snmp/)) - the **1202**
  (Actuated Signal Controller: maxPhases, phaseMinimumGreen, live phaseStatusGroupGreens) and **1203**
  (Dynamic Message Sign: dmsMaxMultiStringLength, dmsMessageMultiString) object roots under
  `1.3.6.1.4.1.1206.4.2`, plus `detws_ntcip_oid()` to build a full object OID (root + instance index)
  to register with `snmp_agent_add_*`; host-tested (`native_ntcip`). _Remaining:_ the fuller 1202/1203
  object set + **1211** (Signal Control and Prioritization). No heap.
- [~] **UTMC** (L, UK/EU ITS) _(common-database codec shipped)_ - `DETWS_ENABLE_UTMC` (`services/utmc`):
  the UTMC common-database HTTP+XML message set - `detws_utmc_request` (a UTMCRequest for an object
  id), `detws_utmc_response` (a UTMCResponse carrying the object value + a data-quality flag +
  timestamp), and `detws_utmc_parse_request` (extract the requested id) - over the existing HTTP
  server, XML-escaped; host-tested (`native_utmc`). _Remaining:_ the fuller object model + the
  DATEX-II profile. Fixed BSS, no heap.
- [~] **OCIT** (L, DE/AT/CH ITS) _(message codec shipped)_ - `DETWS_ENABLE_OCIT` (`services/ocit`): the
  OCIT-Outstations object message - `detws_ocit_build` / `_parse` for
  `[msg-type][object-type][instance][data-type][value]` (get / set / report of a field object) with
  the typed values (bool / byte / u16 / u32 / octets) and typed-value accessors - between central
  traffic computers and field controllers / detectors; host-tested (`native_ocit`). _Remaining:_ the
  fuller object dictionary + the OCIT-O BTPPL transport profile. Fixed BSS device/detector model, no
  heap.
- [~] **V2X / SAE J2735** (XL, connected vehicle) _(UPER codec + BSMcore + SPaT shipped)_ -
  `DETWS_ENABLE_J2735` (`services/j2735`): the ASN.1 **UPER** (Unaligned Packed Encoding Rules) bit-level
  primitive codec - constrained INTEGER (offset in `ceil(log2(range))` bits), BOOLEAN, raw bit fields, a
  bit writer/reader - and the three core V2X messages encode + decode on top: **BSMcore** (the safety
  kernel: msgCnt / id / secMark / lat / long / elev / speed / heading), **SPaT** (the MovementState list:
  per-signal-group phase + min/max end-time countdown), and **MAP** (the intersection geometry: id +
  reference point + per-lane id / ingress flag / node XY offsets). Host-tested against hand-computed bit
  patterns (`native_j2735`; the 162-bit BSMcore packs to 21 octets). _Remaining:_ the full BSM part II
  optionals + the 1609.2 security envelope. The DSRC / C-V2X radio is an external module; the message
  layer is the deliverable.
- [~] **IEEE 1609 (WAVE)** (XL, vehicular radio stack) _(WSMP + 1609.2 envelope codec shipped)_ -
  `DETWS_ENABLE_WAVE` (`services/wave`): the 1609.3 **WSMP** (WAVE Short Message Protocol) header -
  `detws_wsmp_build` / `_parse` (version + a P-encoded **PSID** + length + payload) with the PSID
  variable-length p-encoding (`detws_wave_encode_psid` / `_decode_psid`) - and the **1609.2**
  secured-message envelope header (`detws_wave_1609dot2_wrap`: protocolVersion + contentType); these
  carry the shipped J2735 messages. Host-tested (`native_wave`). _Remaining:_ the full 1609.2
  signature / certificate machinery (the crypto layer) + the WSA service advertisement. The DSRC /
  C-V2X radio is an external module; no heap.
- [~] **NEMA TS 2** (M, traffic cabinet bus) _(SDLC frame codec shipped)_ - `DETWS_ENABLE_NEMA_TS2`
  (`services/nema_ts2`): the traffic-cabinet SDLC bus frame - `detws_nema_ts2_build` / `_parse` for
  `[address][control][frame-type][data][CRC-16/X-25]` linking the controller to the MMU / BIUs /
  detector racks, with the common frame-type constants; host-tested (`native_nema_ts2`, CRC check
  value 0x906E). _Remaining:_ the per-frame-type cabinet object model + the synchronous serial PHY
  / BIU timing (hardware-gated).
- [~] **ATC** (S, platform note) _(interop shipped)_ - the Advanced Traffic Controller
  spec moves cabinets from closed microcontrollers to a standard Linux engine API (the
  ITS Cabinet / ATC API for field-device I/O) for local video analytics + sensor fusion.
  This is a host-platform specification more than a wire protocol; the relevant slice for
  this library is interop - exposing NTCIP / NEMA-TS2 data to an ATC engine over the
  existing HTTP/SNMP surface rather than implementing the Linux ATC stack itself.
  `DETWS_ENABLE_ATC` (`services/atc`): the field-I/O map serialized as
  `{"inputs":[...],"outputs":[...]}` JSON for an ATC engine over HTTP, plus the output
  setter/getter. _Remaining:_ the Linux ATC engine + video analytics stack (out of scope).

### IoT device management

- [~] **LwM2M** (L, OMA LwM2M) - _TLV content format shipped._ `DETWS_ENABLE_LWM2M`
  (`services\lwm2m`): a zero-heap writer + cursor reader for the OMA TLV
  (`application/vnd.oma.lwm2m+tlv`) resource encoding - `lwm2m_tlv_write` (+ typed
  `_write_int` shortest-form / `_write_bool` / `_write_string` / `_write_float` helpers),
  `lwm2m_tlv_read`, and `lwm2m_tlv_value_int`, handling 8-/16-bit ids and inline / 8- /
  16- / 24-bit lengths and the Object-Instance / Resource / Multiple-Resource /
  Resource-Instance kinds; type-byte layout verified against the spec, host-tested. Built
  on the shipped CoAP service ([coap](../src/services/coap/)). Remaining: the client
  interfaces (Bootstrap, Registration, Device Management, Information Reporting / Observe)
  and the standard object model (Security/0, Server/1, Device/3, Firmware Update/5, ...)
  on a fixed BSS model. The **SenML-CBOR / SenML-JSON** content formats are now shipped
  separately - `DETWS_ENABLE_SENML` (`services\senml`): `senml_json_build` /
  `senml_cbor_build` emit a SenML (RFC 8428) pack (base name/time, name, unit, one value,
  time per record) over the JSON / CBOR writers, integral numbers kept as integers; verified
  against the RFC example, host-tested. DTLS is gated on the TLS work; scope the NoSec +
  registration/observe core first. No heap, one flag.

### Messaging & RPC

- [x] **STOMP** (M, messaging) _(shipped)_ - `DETWS_ENABLE_STOMP` (`services\stomp`):
      a zero-heap STOMP 1.2 frame codec - `stomp_build_frame()` writes a frame (command +
      escaped `key:value` headers + blank line + NUL-terminated body) and
      `stomp_parse_frame()` is a non-mutating cursor that reports the command, header
      key/value slices, and body (honoring `content-length`, tolerating `\r\n` endings,
      skipping broker heart-beats), with `stomp_header()` lookup and `stomp_unescape()` for
      header escapes (`\r` `\n` `\c` `\\`). Drives `CONNECT` / `SEND` / `SUBSCRIBE` /
      `MESSAGE` / `ACK` ... against ActiveMQ / RabbitMQ / Artemis over the shipped outbound
      client transport (or STOMP-over-WebSocket via the WS client); pure, host-tested. The
      connection / subscription state is the application's.
- [x] **NATS** (M, messaging) _(shipped)_ - `DETWS_ENABLE_NATS` (`services\nats`): a
      zero-heap codec for the text-based NATS pub/sub protocol - builders for
      `CONNECT` / `PUB` (with optional reply-to) / `SUB` (with optional queue group) /
      `UNSUB` / `PING` / `PONG`, and `nats_parse()` which decodes an inbound
      `MSG` / `INFO` / `PING` / `PONG` / `+OK` / `-ERR` (MSG yields subject / sid / reply-to /
      payload). Line-oriented (CRLF), space-delimited, payload only on PUB/MSG; pure,
      host-tested. Rides the outbound client transport; the connection / subscription state is
      the application's.
- [x] **Sparkplug B** (M, industrial IoT) _(shipped)_ - `DETWS_ENABLE_SPARKPLUG`
      (`services\sparkplug`, implies Protobuf): a zero-heap builder for the Eclipse Sparkplug B
      MQTT payload + topic - `spb_build_topic()` (`spBv1.0/group/type/node[/device]`),
      `spb_build_metric()` (a Tahu Metric: name / alias / timestamp / datatype + an int / long
      / float / double / boolean / string value), and `spb_build_payload()` (timestamp +
      metrics + seq) over the Protobuf codec. Field numbers + datatype codes verified against
      the Eclipse Tahu sparkplug_b.proto; pure, host-tested. Publish it with the MQTT client.
- [~] **gRPC / Protocol Buffers** (L) - _Protobuf wire codec shipped._
  `DETWS_ENABLE_PROTOBUF` (`services\protobuf`): a zero-heap streaming writer
  (`pb_uint64` / `pb_sint64` / `pb_fixed32` / `pb_fixed64` / `pb_float` / `pb_double` /
  `pb_bytes` / `pb_string`, embedded messages via a sub-buffer + `pb_bytes`) and a
  cursor reader (`pb_read_field` over varint / ZigZag / I32 / I64 / length-delimited),
  host-tested against the spec vectors (`08 96 01`, the `"testing"` string, ZigZag
  mapping). **gRPC-Web framing also shipped** - `DETWS_ENABLE_GRPC_WEB`
  (`services\grpcweb`): the 5-octet `[flags][len BE32]` message frame
  (`grpcweb_frame_message`), the 0x80 trailers frame (`grpcweb_frame_trailer`,
  `grpc-status` / `grpc-message`), and `grpcweb_parse`, wrapping the Protobuf codec over the
  shipped HTTP/1.1 server/client (host-tested). Full **gRPC** (the same framing but over
  **HTTP/2** with `application/grpc`) remains gated on the HTTP/2 roadmap item above. Fixed
  BSS, no heap.
- [~] **DDS** (XL, OMG DDS) _(RTPS framing shipped)_ - `DETWS_ENABLE_DDS` (`services/dds`): the RTPS
  (DDSI-RTPS) message + submessage framing codec - the 20-octet header (magic / version / vendor /
  guidPrefix) and the typed submessages (INFO_TS, DATA, HEARTBEAT, ACKNACK, ...) with the endianness
  flag, built by `detws_rtps_header` / `_submessage` and walked by `detws_rtps_parse`; host-tested
  (`native_dds`). _Remaining:_ the CDR serialized-payload encoding, SPDP/SEDP discovery, and the
  reliability/heartbeat reader-writer protocol + QoS subset (all zero-heap BSS); **DDS-XRCE** (the
  resource-constrained agent/client profile) is the more MCU-appropriate full target.
- [x] **WAMP** (M, web messaging) _(shipped)_ - `DETWS_ENABLE_WAMP` (`services\wamp`): a
      zero-heap codec for the Web Application Messaging Protocol (unified RPC + PubSub over
      WebSocket, subprotocol `wamp.2.json`). Builders for HELLO / SUBSCRIBE / UNSUBSCRIBE /
      PUBLISH / CALL / REGISTER / YIELD / GOODBYE (JSON arrays emitted via the shared
      `JsonWriter`; Options/Details default to `{}`, Arguments / ArgumentsKw passed as JSON
      literals) and a nesting-aware positional parser (`wamp_get_type` / `wamp_get_uint` /
      `wamp_get_uri` / `wamp_element`) that pulls the message type, ids, and URIs out of an
      inbound WELCOME / SUBSCRIBED / EVENT / RESULT / INVOCATION / ERROR. Message codes
      verified against the WAMP spec; pure, host-tested. It rides the shipped WebSocket
      layer; the session / subscription / registration tables are the application's. The
      caller can still serialize payloads with the MessagePack / CBOR codecs.
- [x] **CloudEvents** (S-M, CNCF spec) _(shipped)_ - `DETWS_ENABLE_CLOUDEVENTS`,
      `services/cloudevents`: `cloudevents_build_json()` emits a structured
      `application/cloudevents+json` envelope (required `id` / `source` / `type` +
      `specversion` 1.0, optional `subject` / `datacontenttype` / `data` - `data` either a
      verbatim JSON value or an escaped string) over the existing JSON writer, and
      `cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers.
      Host-tested (`test_cloudevents`, 7 cases). The HTTP body / `ce-*` header bindings
      are emitted with the normal send / `add_response_header` paths; an app reuses the
      same envelope over MQTT / WebHook. Fixed BSS, no heap, one build flag.
- [x] **MQTT-SN** (M, sensor networks) _(shipped)_ - `DETWS_ENABLE_MQTT_SN`
      (`services\mqtt\mqtt_sn`): a zero-heap MQTT-SN v1.2 wire codec for the UDP / non-TCP
      MQTT variant on constrained, lossy links (numeric topic IDs instead of strings,
      gateway discovery, sleeping-client keep-alive). Builders for CONNECT / REGISTER /
      PUBLISH / SUBSCRIBE (by name or pre-defined id) / PINGREQ / DISCONNECT / SEARCHGW and
      a `mqttsn_parse_header()` (both the 1- and 3-octet Length forms, big-endian fields) +
      typed parsers for CONNACK / REGACK / PUBACK / SUBACK / PUBLISH / REGISTER, with a
      `mqttsn_make_flags()` helper (DUP / QoS / retain / will / clean / TopicIdType). Wire
      bytes verified against the spec + the Eclipse Paho reference; pure, host-tested. The
      datagram send (`det_udp_sendto`), topic-ID registry, and sleep/retransmit state are
      the application's. Pairs with the existing MQTT client.

### Network telemetry

- [x] **NetFlow / IPFIX** (M, flow export) _(shipped)_ - `DETWS_ENABLE_FLOW_EXPORT`
      (`services\flow_export`): a zero-heap exporter-side codec for on-device flow
      accounting. **NetFlow v5** is fixed (`flow_v5_write_header` + `flow_v5_write_record`,
      24- and 48-octet layouts); **NetFlow v9** (RFC 3954) and **IPFIX** (RFC 7011) share a
      small cursor (`FlowWriter`) - `flow_ipfix_begin` / `flow_v9_begin`, then
      `flow_export_template()` (a Template Set/FlowSet), `flow_export_data_begin/record/end`
      (matching Data Set), and `flow_export_finish()` which patches the IPFIX message length
      or the v9 record count (and pads each v9 FlowSet to a 4-octet boundary). Field offsets
      verified against RFC 7011 / RFC 3954 / the published v5 layout; pure, host-tested. The
      flow cache (5-tuple + counters) and the UDP send (`det_udp_sendto`) are the app's.

### Building automation

- [~] **BACnet/IP & BACnet/SC** (L, ASHRAE 135) - _the BVLC + NPDU framing is shipped._
  `DETWS_ENABLE_BACNET` (`services\bacnet`): `bvlc_build` / `bvlc_parse` (the BACnet/IP
  virtual-link envelope - type 0x81, function, length) and `npdu_build` / `npdu_parse`
  (the network layer - version + NPCI control + optional DNET/DADR destination addressing + hop count, slicing the APDU); layout per ASHRAE 135 Annex J / Clause 6, host-tested.
  Remaining: the **APDU** application layer (the object model - Device / Analog-Input /
  Binary-Output / ... objects, properties, ReadProperty / WriteProperty / COV) and the
  BBMD foreign-device registration; then **BACnet/SC** (Secure Connect) reuses the shipped
  WebSocket + static-pool TLS for its BVLC-SC framing + the same APDU/object model. Fixed
  BSS object database, no heap.
- [~] **XMPP (IoT profile)** (L, XSF) _(stanza codec shipped)_ - `DETWS_ENABLE_XMPP` (`services/xmpp`):
  the RFC 6120 stanza codec - correctly XML-escaped `<stream:stream>` / `<message>` / `<presence>` /
  `<iq>` builders and the stanza-name + attribute readers; host-tested against exact-output vectors
  (`native_xmpp`). _Remaining:_ the streaming XML parser + the SASL/TLS handshake (TLS reuses the
  shipped client TLS) and the IoT XEPs (0030 disco, 0060 pub/sub, 0323 sensor data, 0325 control) on
  a fixed BSS roster/node model.

### Databases & time-series

- [x] **InfluxDB Line Protocol** (S, time-series ingest) _(shipped)_ - the
      `DetwsLine` builder (`services/udp_telemetry`, `DETWS_ENABLE_UDP_TELEMETRY`) emits
      the full `measurement,tag=v field=v timestamp` form: `detws_line_add_tag()` (escaped
      tag set, must precede fields) + `detws_line_set_timestamp()` on top of the existing
      int/uint/float fields. Host-tested (`test_udp_telemetry`: tags+timestamp ordering,
      special-char escaping, tag-after-field fail-closed). Cast over UDP
      (`detws_udp_telemetry_cast`) or POST the same line to InfluxDB `/write` with the
      shipped HTTP client. Pairs with the telemetry-math service. One build flag, no heap.
- [~] **NoSQL / database clients** (M-L, candidate) - _Redis RESP codec shipped._
  `DETWS_ENABLE_REDIS` (`services/redis_resp`): a zero-heap `resp_encode_command()`
  (array of bulk strings, binary-safe via explicit arg lengths - drives any command
  incl. SET/GET/HSET/XADD) + a cursor `resp_parse()` reply decoder (simple / error /
  integer / bulk / array / nil; incomplete + malformed fail closed). Host-tested
  (`test_redis_resp`, 8 cases). Drive it over the shipped outbound client transport.
  Heavier candidates (MongoDB wire protocol, Postgres frontend/backend protocol) are
  larger and lower-priority. Fixed BSS, no heap, one flag per backend.

### Motor / actuator control (ESC protocols)

For drone / robotics use - drive ESCs and ingest their telemetry straight from the
web server, using the ESP32's hardware timing peripherals (RMT / MCPWM) so the
microsecond-precise pulse trains run without locking the CPU. Each behind a build
flag; fixed BSS, no heap.

- [~] **DShot** (M, RMT) _(codec shipped)_ - `DETWS_ENABLE_DSHOT` (`services/dshot`): the DShot frame
  codec - `detws_dshot_encode` / `_decode` build + validate the 16-bit packet (11-bit throttle /
  command + telemetry-request + 4-bit nibble-xor CRC), with the special-command constants and the
  per-rate (150/300/600/1200) bit high-times for an RMT driver. Host-tested against hand-computed
  vectors (`native_dshot`). _Remaining:_ the RMT pulse-train transport (the HW timing backend) and
  the HTTP/WS/dashboard throttle+arming control surface.
- [~] **Bidirectional / Extended DShot (EDShot)** (M, RMT) _(outbound frame shipped)_ - the
  bidirectional inverted-CRC frame is built by `detws_dshot_encode(..., bidirectional=true)` (and
  validated by `_decode`). _Remaining:_ decoding the ESC's GCR-encoded return frame (eRPM /
  temperature / voltage / current) off the same wire, so the server can stream live ESC telemetry
  (pairs with the telemetry-math + dashboard services).
- [~] **ProShot** (S, RMT) _(shares the DShot packet)_ - ProShot carries the same 16-bit DShot value
  (built by `detws_dshot_encode`, shipped) as four pulse-position symbols. _Remaining:_ the
  nibble->pulse-position encoding + the RMT emission (the packet + CRC are done).
- [~] **OneShot / Multishot** (S, MCPWM) _(pulse-width mapping shipped)_ - `detws_esc_pwm_ns()`
  (`services/dshot`) maps a 0..1000 throttle to the pulse high-time for standard PWM (1-2 ms),
  **OneShot125** (125-250 us), **OneShot42** (42-84 us), and **Multishot** (5-25 us); host-tested
  (`native_dshot`). _Remaining:_ driving it from the MCPWM peripheral synced to the control loop.

### Network Time Security (NTS, RFC 8915)

Authenticated time - the secure successor to plain NTP (the current
`DETWS_ENABLE_NTP` client trusts whatever the network hands it, so a MITM can shift
the device's clock and break TLS validity windows, JWT/OIDC `exp`, TOTP, and log
timestamps). NTS adds cryptographic authentication on top of NTPv4 with no shared
secret. Builds on the existing NTP client + the TLS stack + AEAD primitives; fixed
BSS, no heap, behind a build flag.

> The plain (unauthenticated) NTP **server** already ships (`DETWS_ENABLE_NTP_SERVER`,
> RFC 5905 server mode on UDP/123 - see the feature reference; example 58.NtpServer pairs
> it with a GPS stratum-1 source + upstream-NTP fallback). NTS below hardens the time the
> device _consumes_; a future NTS server mode could extend the server the same way.

- [~] **NTS-KE (Key Establishment)** (L) _(record codec shipped)_ - `DETWS_ENABLE_NTS`
  (`services/nts`): the NTS-KE TLV record codec - `detws_nts_ke_request` builds the standard client
  request (Next Protocol NTPv4 + AEAD AES-SIV-CMAC-256 + End-of-Message, all critical) and
  `detws_nts_ke_parse` walks a response, surfacing each record (cookies, negotiated AEAD, server,
  port, error) via a callback; host-tested (`native_nts`). _Remaining:_ running it over the static-pool
  mbedTLS client (ALPN `ntske/1`) + the RFC 5705 exporter key derivation (the label is exposed).
- [~] **NTS-protected NTP** (L) _(extension-field framing shipped)_ - `services/nts` also builds the NTS
  NTP extension fields (Unique Identifier, NTS Cookie, and the RFC 7822 4-byte-padded framing);
  host-tested. _Remaining:_ the `AEAD_AES_SIV_CMAC_256` (RFC 5297) authenticator protect/verify + the
  anti-replay unique-id echo + cookie rotation, then feed the validated offset to `det_clock`.

### Documentation: Sphinx over Doxygen (do last)

> The final roadmap item - tackle only after everything else on the TODO and the
> roadmap above is checked off.

The docs today are Doxygen + a custom CSS theme. The goal is to add **Sphinx** on top and
then apply **"squirty"** styling over it for a polished, modern docs site.

- [x] **Sphinx + Breathe bridge** (L) _(shipped)_ - the Sphinx project lives in `docs/sphinx`: `docs/Doxyfile`
      now emits XML (`GENERATE_XML=YES` -> `docs/sphinx/xml`), **Breathe** renders it into the API reference
      (the `DetWebServer` public surface), and **MyST** pulls the hand-written Markdown guides
      (`ARCHITECTURE.md`, the `docs/learn` primers) into the same site via `include` stubs (one source of
      truth). Built + verified on the RPi (doxygen + a `~/.sphinxdocs` venv from `docs/sphinx/requirements.txt`,
      then `sphinx-build`), and kept buildable by the **Docs (Sphinx)** CI workflow
      (`.github/workflows/docs-sphinx.yml`, build + artifact). The full per-symbol `doxygenindex` is not
      inlined (the Sphinx C++ domain does not parse every macro-heavy declaration); that complete reference
      stays in the Doxygen HTML.
- [x] **Apply "squirty" over the Sphinx theme** (M) _(shipped)_ - "squirty" is the project brand: the
      **Squirty the Injection Squid** mascot (`docs/squirty.svg`) plus the **Retro TTY Green Screen** palette
      (phosphor `#2bb35a` on a near-black `#080c08` CRT, from `docs/custom.css`). Layered over **Furo** via
      `docs/sphinx/conf.py` (light/dark brand css-variables) + `docs/sphinx/_static/squirty.css` (the mascot
      logo, phosphor glow, a faint scanline wash, monospace headings, terminal-styled Breathe signatures),
      light/dark aware.

### Bare-bones default build + per-service flash cost

> Today a fresh build enables far more than a minimal HTTP server needs (`src/ServerConfig.h`
> defaults `DETWS_ENABLE_WEBSOCKET`, `DETWS_ENABLE_SSE`, `DETWS_ENABLE_FILE_SERVING`,
> `DETWS_ENABLE_AUTH`, ... to `1`). The default should be a single-purpose HTTP server and
> everything else strictly opt-in, so the baseline footprint is as small as possible and the
> cost of each feature is explicit.

- [ ] **Slim the default config to a single-purpose HTTP server** (M) - flip the `src/ServerConfig.h`
      `DETWS_ENABLE_*` defaults so only the core request/response path is on out of the box; every other
      service (WebSocket, SSE, auth, file serving, WebDAV, the L6/L7 protocols, the industrial/IoT buses,
      crypto, storage, ...) defaults **off** and is turned on per build via its own flag. Bare-bones by
      default shrinks the baseline flash/RAM and makes each feature a conscious opt-in. Update the examples
      that rely on a default-on feature to set its flag explicitly, and re-check the configurator + FEATURES
      grid.
- [ ] **Per-base-service flash-cost table** (M) - measure and document the incremental flash (and static
      RAM) each base service adds on top of the bare-bones baseline, so a user can budget a build. Extend the
      footprint tooling (`tools/ci/example_footprints.py`) to emit a "cost of enabling service X alone" delta
      against the minimal build and land it as a table in `docs/FOOTPRINTS.md`.

### Split dwserver.cpp into single-purpose files

> `src/dwserver.cpp` is a ~4,000-line monolith holding the whole DetWebServer implementation
> (listen/begin lifecycle, routing + dispatch, request handling, the response builders, chunked
> streaming, static-file + WebDAV serving, auth, and the HTTP/2 + HTTP/3 seams). One giant TU is
> harder to navigate, debug, and extend than a set of focused units.

- [ ] **Break dwserver.cpp into a `src/server/` of single-purpose files** (L) - carve the monolith into
      cohesive translation units (e.g. `server/lifecycle` for listen/begin/restart, `server/routing` for
      route matching + dispatch, `server/response` for the send/send_template/send_chunked builders,
      `server/file_serving`, `server/webdav`, `server/auth`, and the `server/http2` / `server/http3` seams),
      each with a clear internal header. Pure move-and-split - no behavior change - so the native + dual-core
      builds and the full test suite stay green throughout; keep the owned-context / no-stdlib / single-owner
      rules. Makes the core easier to debug, extend, and maintain.

### One folder per implemented service

> Convention: a service that has both a `.h` and a `.cpp` lives in its own `src/services/<name>/`
> directory; only header-only services (a `.h` with no `.cpp`) are allowed to sit at the
> `src/services/` root. Today nine root-level services break this.

- [x] **Move each `.h`+`.cpp` service at the `src/services/` root into its own folder** (M) _(shipped)_ - cloudevents,
      mdns_service, ntp_service, ota_service, provisioning_service, redis_resp, stomp, upload_service, and
      web_terminal currently live as loose `src/services/<name>.{h,cpp}` pairs. Move each into
      `src/services/<name>/<name>.{h,cpp}`, update the `#include` paths, the `build_src_filter` globs in
      `platformio.ini`, the compile-DB / dep-graph tooling, and any docs references. Header-only services
      (e.g. `clock.h`, `i2c.h`) stay at the root. Pure relocation - all builds + tests stay green.

### Perf: unroll small pool scans to bitflag + bitmask compares

> Hot-path validity/membership checks that loop over a small fixed pool (conn_pool, ws_pool, sse_pool,
> the SSH per-conn tables) should be O(1) bitflag+mask ops or fully-unrolled branchless compares, not
> `for` scans. The stale-pcb guard in `det_tcp_do` (`pcb_still_bound`) is the motivating case: SEND/OUTPUT
> already reduce to the O(1) `k->pcb == conn_pool[k->slot].pcb` slot compare; only RAWSEND (TLS BIO, slot
> unknown) still scans.

- [ ] **Replace small fixed-pool scans with a live-slot bitmask** (M) - maintain a `uint32_t` active-slot
      mask (bit i = slot i has a live pcb/conn), updated at the single set/clear-`pcb` sites, so membership
      and "any active?" become one `& (1u << slot)` / `!= 0` test. For pointer->slot lookups that cannot
      carry the slot (the RAWSEND / TLS BIO path), unroll the fixed `CONN_POOL_SLOTS` scan into a
      branchless OR of compares (the compiler already does this at -O2; make it explicit + measured).
      Benchmark before/after via the JTAG CCOUNT harness on the S3 rig so the win is real, not assumed.

- [ ] **SWAR / bit-sliced constant-time base64 decode** (M) - the ESP32 base64 _decode_ is mbedTLS's
      constant-time codec (branchless per-character range masks) so it does not leak the credential via
      timing; the CCOUNT bench measured ~5040 cyc / 21 us to decode a Basic-auth credential (~11x the hex
      decode). A plain byte-indexed lookup table would be faster but is NOT constant-time (the touched cache
      line is data-dependent -> a timing side-channel). The constant-time way to go faster is SWAR: pack 4
      chars into a `uint32` (8 into a `uint64`) and apply the range comparisons to all lanes in parallel with
      word-wide branchless masks, running the classification once per word instead of once per char (~4-8x
      fewer ops), still data-independent. Decode is once-per-request (not a hot byte loop) so the win is
      modest - do it only with (a) RFC 4648 test vectors both directions, and (b) a **timing-invariance
      check** on the S3 rig (CCOUNT must not vary with the input bytes), measured through `/bench`.
- [ ] **Hand-rolled SSE record framer** (S) - `sse_format()` builds each `event:`/`id:`/`data:` record with
      three `snprintf("%s")` calls; the CCOUNT bench measured ~3393 cyc / 14 us for a fully-addressed record
      on the S3, dominated by the Xtensa `vsnprintf` path (~7x the `mime_type` lookup). A branchless
      `memcpy`-based framer (fixed field prefixes + `strlen`/`memcpy` of each value + the terminators) would
      cut it by roughly an order of magnitude. Not worth it at SSE's per-event cadence, but a real win for a
      high-rate broadcast fan-out (many subscribers, high push rate). Gate on the existing `test_sse_format`
      byte-exact tests + the `/bench` figure.

### Radar presence sensor drivers (mmWave + Doppler)

> Two more presence radars for the EM / radar presence + motion section above (LD2410 is the shipped
> reference codec): a framed-UART mmWave module and a bare-GPIO Doppler module - opposite ends of the
> presence-sensing cost/capability range. Both bridge northbound to the web stack like the other sensor
> drivers; a device reads them over UART / GPIO and publishes presence.

- [ ] **Waveshare HMMD 24 GHz mmWave presence sensor** (M) - human micro-motion detection (moving /
      standing / stationary body plus a target distance gate) built on the **S3KM1110** 24 GHz FMCW radar
      SoC behind a **PY32F003** MCU, exposed over a 3.3V UART (default **115200** baud) plus a GPIO presence
      pin ([waveshare.com/wiki/HMMD_mmWave_Sensor](https://www.waveshare.com/wiki/HMMD_mmWave_Sensor)). Add
      `DETWS_ENABLE_HMMD` / `services/hmmd`: a pure, host-tested **little-endian** frame codec - the default
      ON/OFF + distance-gate report parse and a byte-by-byte resyncing stream reassembler (mirror the
      `Ld2410Stream` reassembler), plus the command encoders (read firmware version `0x0000`, serial number
      `0x0011`, register `0x0002`, parameter config `0x0008`). Bridge presence / distance northbound exactly
      like LD2410. Test = captured-frame vectors both directions (`native_hmmd`); the sensor's GPIO OUT pin
      shares the debounced presence bridge below.
- [ ] **RCWL-0516 microwave Doppler presence sensor** (S) - the low-cost ~3.18 GHz Doppler motion module
      (RCWL-9196 controller + MMBR941M RF amp): no data protocol at all - a single 3.3V **OUT** pin that
      latches HIGH for ~2 s on any moving reflector and returns LOW when idle. This is the analog-Doppler end
      of the presence path already noted in the EM / radar section. Add `DETWS_ENABLE_RCWL0516` /
      `services/rcwl0516`: a tiny debounced edge-detect + hold-time state machine over one GPIO (pure,
      host-testable by injecting pin levels against a fake clock via `detws_millis`), surfaced to the web
      stack as a boolean presence event - the same one-GPIO presence facade the HMMD OUT pin and other
      bare-output detectors (PIR, HB100) can reuse.
- [ ] **HLK-LD2410B (Bluetooth variant of the shipped LD2410)** (S) - the LD2410**B** is HiLink's
      BLE-equipped build of the same 24 GHz FMCW presence radar, speaking the **same `FD FC FB FA` framed
      UART protocol** the shipped `services/ld2410` codec already builds and parses
      ([datasheet](https://en.ai-thinker.com/Uploads/file/20231016/20231016032622_13559.pdf)). So this is a
      **verification + thin extension** of the existing driver, not a new codec: confirm the LD2410B report /
      ACK frames decode byte-for-byte with `ld2410_parse_report` + the `Ld2410Stream` reassembler (add
      captured-frame vectors to `native_ld2410`), document it as a supported part, and add the few
      LD2410B-only config commands (Bluetooth enable / permission, MAC query) to the `FD FC FB FA` command
      encoders. The BLE control channel itself is out of scope for the wired driver - the UART report / config
      path is the shared surface.

### Functional-safety & industrial-standards profiles (IEC 61784-3, ODVA)

> The "black channel" functional-safety communication profiles (IEC 61784-3) layer a SIL2/SIL3-capable
> safety protocol - its own CRC signature, a monitoring / consecutive counter (detects lost, repeated,
> inserted, or reordered frames), a watchdog timeout, and a per-connection authentication - _on top_ of a
> standard, non-safe fieldbus, treating the transport as an untrusted pipe (so the underlying network needs
> no safety certification). The base fieldbuses these ride on are already shipped here as pure codecs
> (`services/profinet`, `profibus`, `enip`, `cip`, `iolink`), so each safety profile is a black-channel
> Safety-PDU codec on top and stays a **pure, host-testable codec** - the device carries and validates the
> safety frames; the safety PLC / logic solver owns the SIL claim. All follow
> [[implement-protocols-faithfully]] against the published specs + a real reference peer.

- [ ] **IEC 61784-3 black-channel Safety Communication Layer (SCL)** (M) - the shared mechanism the four
      profiles below all specialize: the residual-error model (CRC signature over safety data + a virtual /
      actual consecutive number + a receive watchdog), sized so the probability of an undetected corrupt
      frame meets SIL 3. Land the common primitives once (`services/safety_scl`): a CRC-signature helper, a
      monitoring-counter state machine, and a watchdog/timeout guard, then have each profile's codec compose
      them. Pure + host-tested against per-profile vectors.
- [ ] **PROFIsafe (IEC 61784-3-3, PI)** (M) - the SIL3 safety layer over the shipped PROFINET / PROFIBUS
      codecs. `DETWS_ENABLE_PROFISAFE` / `services/profisafe`: the Safety-PDU (F-I/O data + status/control
      byte + consecutive number + CRC2 signature), the F-Parameters (F_Dest_Add / F_WD_Time ...) with their
      CRC1, and the host/device watchdog timing. Pure codec; host-tested and interop-checked against a
      PROFIsafe reference (e.g. a Siemens F-host / codesys-safety).
- [ ] **CIP Safety (IEC 61784-3-2, ODVA)** (M) - the SIL3 safety extension of CIP over the shipped
      EtherNet/IP + CIP codecs. `DETWS_ENABLE_CIP_SAFETY` / `services/cip_safety`: the safety I/O message
      (Base + Extended format: Mode Byte, Actual + Complemented data, Time Stamp, CRC-S1/S3/S5), the Time
      Coordination exchange, and the safety connection (`SafetyOpen` / `SafetyClose`) producer/consumer time
      expectation. Pure codec; interop vs an ODVA CIP Safety originator.
- [ ] **FSoE / Fail Safe over EtherCAT (IEC 61784-3-12, ETG)** (L) - the SIL3 safety layer over EtherCAT.
      `DETWS_ENABLE_FSOE` / `services/fsoe`: the FSoE frame (Command + SafeData + a CRC per data slot +
      Connection-ID / sequence), the FSoE connection state machine (Reset -> Session -> Connection ->
      Parameter -> Data), and the watchdog. **Gated on an EtherCAT (mailbox) base transport first** - EtherCAT
      is noted but not yet shipped (see the field-device roadmap above); FSoE itself is black-channel so it
      host-tests standalone against ETG.5100 vectors.
- [ ] **IO-Link Safety (IO-Link Community)** (M) - functional safety over the shipped IO-Link (SDCI,
      IEC 61131-9) codec. `DETWS_ENABLE_IOLINK_SAFETY` / `services/iolink_safety`: the OSSDe signal handling
      plus the safety-PDU over IO-Link process data / ISDU (CRC + counter + watchdog, FS-Master / FS-Device
      roles). Pure codec; host-tested against the IO-Link Safety test spec vectors.
- [ ] **ODVA CIP Security (CIP Networks Library Vol 8)** (L) - the ODVA _security_ (not safety) profile for
      EtherNet/IP: device authentication + integrity + confidentiality via **TLS (EtherNet/IP TCP) and DTLS
      (UDP I/O)** with X.509 identities, plus the CIP Security object model (the EtherNet/IP Security,
      Certificate Management, and QoS objects). Reuses the crypto already in this library - the mbedTLS TLS
      transport and the hand-rolled **DTLS 1.3** stack ([[dtls13-initiative]]) + Ed25519 / X.509 - so this is
      mostly the CIP object plumbing binding those to `services/enip`. `DETWS_ENABLE_CIP_SECURITY` /
      `services/cip_security`; interop vs an ODVA CIP Security stack.
- [ ] **PI (PROFIBUS & PROFINET International) - PROFINET Security** (L) - PI is the steward of the shipped
      PROFINET / PROFIBUS codecs and the PROFIsafe profile above; its security counterpart to ODVA's CIP
      Security is **PROFINET Security** (the Security Class 1/2/3 model: integrity + authenticity via a
      per-frame SecurityControl header + MAC, then confidentiality, with certificate / 802.1X device
      identity). `DETWS_ENABLE_PROFINET_SECURITY` / extend `services/profinet`: the SecurityControl +
      integrity-MAC frame wrapping and the certificate-based identity, reusing the library's crypto
      (HMAC / AES-GCM, X.509). Interop vs a PROFINET Security controller.

### Industrial wireless & QoS (WirelessHART, DiffServ, IEC 62657-2)

> Wireless industrial traffic - especially the functional-safety frames above - needs prioritized,
> coexistence-managed delivery. These items give the device the transport-side QoS and the wireless
> profiles a plant floor expects.

- [ ] **WirelessHART (IEC 62591)** (L) - the 2.4 GHz IEEE 802.15.4 TDMA channel-hopping mesh that carries the
      HART application layer, on top of the shipped `services/hart` command codec (FieldComm Group). Scope the
      realistic slice: the WirelessHART command set (network-manager / device commands) + the network
      management data (superframe / link / route reports) as a pure codec over HART; the full TSMP-style TDMA
      MAC + channel-hopping schedule is radio-firmware territory (like the other 802.15.4 radios) and is
      called out as HW-gated. `DETWS_ENABLE_WIRELESSHART` / `services/wirelesshart`.
- [ ] **DiffServ QoS marking (RFC 2474)** (S) - set the 6-bit DSCP in the IP header (the RFC 2474 DS field)
      on outbound traffic so safety / real-time packets carry the Expedited-Forwarding class (EF, DSCP 46) and
      the Wi-Fi driver / AP maps them into the top 802.11e WMM access categories (AC_VO voice, AC_VI video)
      instead of best-effort. A small transport-layer addition: a per-listener / per-connection DSCP setting
      plumbed through `det_udp` / `det_conn` to lwIP's `ip_tos` / the `IP_TOS` socket option, with a policy
      that tags the functional-safety and CIP / PROFIsafe flows EF by default. Host-testable by asserting the
      TOS byte on the emitted datagram; `DETWS_ENABLE_QOS_DSCP`.
- [ ] **Wireless coexistence management (IEC 62657-2)** (M) - adhere to IEC 62657-2 so the device behaves in
      a crowded industrial 2.4 / 5 GHz band shared by WLAN, WirelessHART, ISA100, Bluetooth, and Zigbee:
      coexistence-aware channel selection plus a spectrum-use / interference report a central coexistence
      manager can consume, and a backoff / channel-blacklist policy. `DETWS_ENABLE_COEXISTENCE` /
      `services/coexistence`: the reporting data model + the channel-plan policy engine (pure, host-tested);
      the actual RF scan feeds it from the radio driver.

### Seamless Wi-Fi roaming (IEEE 802.11r / k / v)

> Real-time and functional-safety traffic cannot tolerate a multi-second reassociation when the device
> roams between plant-floor access points. The ESP32 Wi-Fi firmware / supplicant already provides the
> 802.11r/k/v primitives; the library's job is to enable + configure them and add the roaming _decision_
> layer that uses them, holding the DSCP-EF safety flows (above) alive across a handover.

- [ ] **802.11r Fast BSS Transition (FT)** (M) - pre-authenticate / pre-derive the PTK with the target AP
      before roaming so reassociation completes in **< 50 ms** (no full 4-way handshake on the new AP). On
      ESP32 the FT key hierarchy (PMK-R0 / PMK-R1) + FT handshake live in the Wi-Fi supplicant, so this is
      enable + configure (`DETWS_ENABLE_FAST_ROAM`; PSK or WPA2/3-Enterprise FT), expose the handover timing,
      and keep the safety flows pinned across the transition. Measure the handover on the S3 rig against the
      50 ms budget.
- [ ] **802.11k Radio Resource Measurement (neighbor reports)** (S) - request the AP's neighbor report (the
      candidate BSSIDs + channels) so the device targets a roam without a full off-channel scan, and surface
      that list to the decision layer below. `DETWS_ENABLE_RRM`.
- [ ] **802.11v BSS Transition Management (BTM)** (S) - accept and act on network-suggested roams (BTM
      Request -> Response) so the infrastructure can steer the device to a better AP before its signal drops,
      feeding the hint into the decision layer. `DETWS_ENABLE_BTM`.
- [ ] **Predictive roaming decision layer** (M) - the policy that fuses 802.11k neighbor reports + 802.11v
      BTM hints + the RSSI trend to pick and execute an 802.11r fast transition _before_ the current link
      degrades, not after it drops - the piece that turns the three primitives into seamless roaming. Pure,
      host-testable logic (feed it synthetic RSSI / neighbor / BTM inputs, assert the roam trigger + target);
      the actual association is the supplicant's. `services/roaming`.
