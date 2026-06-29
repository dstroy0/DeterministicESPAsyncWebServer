# Changelog

All notable changes to DeterministicESPAsyncWebServer are documented here.

## [Unreleased]

### CI / Build

- update CHANGELOG.md [skip ci] ([`ed56bb6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed56bb650f0f969d3d3343934b8698098512a9c6))
- update test report [skip ci] ([`c267b27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c267b276e20af44e66963e7d7789257565f38ca9))
- update CHANGELOG.md [skip ci] ([`d0d9288`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0d9288f31eecee76d9494bed76faa9433b07a78))
- update test report [skip ci] ([`90077e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90077e63321b7f4f8cc1a8c1b2a9036bcb563bfc))
- update CHANGELOG.md [skip ci] ([`80463fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80463fb724f937b247859b7e50abb8614215c4a9))

### Documentation

- link the learn series + STANDARDS.md from the docs landing page ([`c4f0222`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4f02223894ef0a719ea416041cb5c4c96faee0d))
- beginner from-scratch primers (OSI model, TCP/IP, languages) ([`0ceadac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ceadacd98d99b5b6a20abf917c1e188cad05e76))
- add STANDARDS.md (links every standard the lib uses) + roadmap 'audit against standards' item ([`06db79f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06db79f0e8e7426866ad0ad03398d548ccd2278d))

## [4.11.2] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.2 - 2026-06-29</b></summary>

### Bug Fixes

- Observe used millis() (host-unbuildable + pluggable-clock violation) + CI coverage for gated features ([`4333bc8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4333bc8036e3fd2a57bb8bc29256086840960e2d))

### CI / Build

- update test report [skip ci] ([`3c3e0ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c3e0cea03c3233cfac6ba6ca4aa6a2c52406dd4))
- update CHANGELOG.md [skip ci] ([`65b9a26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65b9a269a638462d7ad4b1a81404ac053aed2b3c))

### Changes

- Bump version: 4.11.1 → 4.11.2 ([`7e4b710`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e4b710fca4175f842daf66c08240f9b65cdec83))

</details>

## [4.11.1] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.1 - 2026-06-29</b></summary>

### Bug Fixes

- reject Transfer-Encoding on inbound requests (request-smuggling) + test-gap hardening ([`f45eafb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f45eafb40bbe716963329266ec9b689060b4298f))

### CI / Build

- update test report [skip ci] ([`7a77905`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a779052e3dee71f37b4d2151894e51534946c63))
- update CHANGELOG.md [skip ci] ([`d14e248`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d14e248e63c98f730412780fc8b44804c9bf6368))
- update test report [skip ci] ([`6b953f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6b953f778d1e4044c631d0175137b79ed3783855))
- update CHANGELOG.md [skip ci] ([`2614b1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2614b1b411036d06215921f961f94e49c56095c2))
- update test report [skip ci] ([`80e7f9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80e7f9c4166474ad7dd9f65fdecb18d3333d232c))
- update CHANGELOG.md [skip ci] ([`10a406a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/10a406a732f451222b31ab5c7f8ee1aaf9ca3c0c))
- update test report [skip ci] ([`9a43b18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a43b1889c98bdb990c842c1d743fd53998477eb))
- update CHANGELOG.md [skip ci] ([`4dd142e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4dd142eb79d88c3ea43fbc043ec5b316567cc3bd))
- update CHANGELOG.md [skip ci] ([`50fcafc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50fcafcc986e9bb45584912a2b7aa6560a484729))
- update test report [skip ci] ([`1c5e46e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c5e46e8a2f50e14b24c047bfef411d400d1e96e))
- update CHANGELOG.md [skip ci] ([`04c4d39`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04c4d3993aba64181fabb53be273a722fd98f888))

### Changes

- Bump version: 4.11.0 → 4.11.1 ([`2f284ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2f284ab4700753893f6d74c844620abdd5d53d8b))

### Documentation

- standing front-end assumption (SPI/UART/I2C adapters + ADC for 4-20mA) so no protocol is hardware-blocked ([`235f669`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/235f66930171d9de5f94b3b1a90b7b4b407849cb))
- raw-L2 enabler + real-time timing model; ESC/motor protocols; real-protocol interop harness ([`91fd148`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91fd14801030032a334ce00c371f9e53f8650592))
- add IoT/messaging/DB protocols (LwM2M, STOMP, gRPC/Protobuf, DDS, WAMP, CloudEvents, MQTT-SN, NetFlow/IPFIX, BACnet/IP+SC, XMPP-IoT, InfluxDB line, Redis/NoSQL) ([`8430840`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8430840755a2ed86fa99813dc780322c4ed424b1))
- add ITS / V2X / traffic-cabinet protocols (NTCIP 1202/1203/1211, UTMC, OCIT, SAE J2735, IEEE 1609 WAVE, NEMA TS 2, ATC) ([`1a12665`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a12665d240199fead0606d0f1b23c83c29d2883))
- add DER / smart-grid protocols (IEEE 2030.5/SEP2, OpenADR, SunSpec Modbus, ICCP/TASE.2) ([`89973e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89973e5d72f289238e4a2db3475c5075b133b29e))
- add power-grid SCADA protocols (IEC 60870-5-101/104, IEC 61850 MMS/GOOSE, IEEE C37.118) ([`ccb5f5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ccb5f5a987ebc033b150e22ab1520390ad49d3b2))

</details>

## [4.11.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`608abb4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/608abb4498cea660a8939eca32512fafaefe1547))
- update CHANGELOG.md [skip ci] ([`43476ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43476ad288c12f461203f27a1973b3a5c98f002e))
- update test report [skip ci] ([`7eab22b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7eab22b89c3a482880aa927ac9201ed6b67e1889))
- update CHANGELOG.md [skip ci] ([`0d572c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d572c94d6af6580f3a2baa63802425b29953fca))

### Changes

- Bump version: 4.10.0 → 4.11.0 ([`b01b560`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b01b56036e5893a86ce881cd1fc3813c8bf88051))

### Documentation

- fix BUGS.md nested-list formatting (prettier) ([`489137f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/489137fc69fd08c9f664eefbb995de0f3debadfb))

### Refactor

- drop redundant pcb threading from the egress API (ingress/egress symmetry) ([`cfea4c4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cfea4c4a566b32bb88234ef0d902b9ff758f4f41))

</details>

## [4.10.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.10.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`4701a90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4701a909ee8c209829a748cbb11c58e3905eb9a4))
- update CHANGELOG.md [skip ci] ([`a2998b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2998b6efaeff15e1596f773e1a28bce8003713a))

### Changes

- Bump version: 4.9.2 → 4.10.0 ([`3181bb3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3181bb36352a205e6200488345e9a16ec282fd63))

### Refactor

- shared response helpers, MIME/byte-cursor primitives, unified base64url + DNS ([`817ca53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/817ca53b097c8c90308e9824744fcdab572835d5))

</details>

## [4.9.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.2 - 2026-06-28</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`d4d314c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4d314c8eecb1cd3caef4d6b0cab964fa1641a36))
- remove editor-tool ignore entry + stale changelog line ([`8e4d25e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e4d25eb808786077979da8942324eb3305b133a))

### Changes

- Bump version: 4.9.1 → 4.9.2 ([`ab2e6ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab2e6ed0780ce277a0dfed28df8c3f961358cc10))

### Refactor

- shared det_hex + relocate primitives to src/shared_primitives ([`c1a8669`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c1a8669774024124beecc7a3ffb4592fdce93eae))

</details>

## [4.9.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.1 - 2026-06-28</b></summary>

### Bug Fixes

- range overflow, If-Modified-Since month mis-parse, dns_resolver clock ([`57675c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/57675c05d675f06bb5dcffb75dbb147f1eaaa580))

### CI / Build

- update test report [skip ci] ([`2bc855e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2bc855eb3646c006fc88ffc9953a7a3c60ec696f))
- update CHANGELOG.md [skip ci] ([`369ad94`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/369ad94b29f2450ae7e844d6a0ca02133b43e03a))

### Changes

- Bump version: 4.9.0 → 4.9.1 ([`5697ef1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5697ef1ffb7292187b9dfd821d68315936d20cef))

</details>

## [4.9.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`0402eb8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0402eb8dbff6b1bde79c513d67792e3dd6a8695a))
- update CHANGELOG.md [skip ci] ([`9b42ce7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b42ce7113f6e194ea427e40890c269f53f7ad4e))

### Changes

- Bump version: 4.8.2 → 4.9.0 ([`2b95cbb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b95cbb9ef0f64648a00959aa344e30db83d4aa8))

### Features

- Last-Modified + If-Modified-Since conditional GET on served files ([`715c5da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/715c5da3535494b45a3ff5372ddf9921ad65f04c))

</details>

## [4.8.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`5d56f51`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d56f5149e151abeb54685ec0ae82a6ef9ced510))
- update CHANGELOG.md [skip ci] ([`9508f7a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9508f7ad3e9a6459eaef0a717bfb941e8d421069))

### Changes

- Bump version: 4.8.1 → 4.8.2 ([`4b202bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b202bc21d4fa9c04a442f35cb50cf4d591c23ec))

### Performance

- share the ring producer too (bulk memcpy both sides) ([`75f77bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75f77bfdea0d70c03656cf9202ebce6f5e5a94d6))

</details>

## [4.8.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`946d971`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/946d97136db2c9b7244c9f536cb4bd3cda5d8d88))
- update CHANGELOG.md [skip ci] ([`9f5df31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f5df31b7e1032cb43467a4c4b5f39c3c25c18bf))
- update test report [skip ci] ([`bb04311`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb04311a5ca62d952f1b45719c04aa81a8b7b225))
- update CHANGELOG.md [skip ci] ([`7ea66b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ea66b16f47c7da483cefe491931ef41f8ca53a3))
- update test report [skip ci] ([`a79e38b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a79e38b4dfbfcccdacf529dc14b1b2bd6d096f84))
- update CHANGELOG.md [skip ci] ([`ccf92bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ccf92bccd1b3e7a2bba2bf4e3a7446c50c10589f))

### Changes

- Bump version: 4.8.0 → 4.8.1 ([`bebffe0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bebffe003327443aedb12f2497dfc3ff5e9f5b5b))
- clang-format hook decl + cspell add doc/test words (fix CI) ([`5177f78`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5177f780748c391bce9f2ebb9a83e8b12004369e))

### Documentation

- record HW validation of the client-transport ack-on-consume fix ([`14fbadc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14fbadc1a8b3b5392f0b1ce0a4908879ef83313e))

### Refactor

- one shared SPSC ring primitive for both transports ([`7edc28c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7edc28c5287c30724f7d53e8078bd301550abfe7))

</details>

## [4.8.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.0 - 2026-06-28</b></summary>

### Bug Fixes

- ack-on-consume on the outbound transport (det_client) ([`deb71d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/deb71d9f8eb740f262858c019625633eeabd6961))

### CI / Build

- update test report [skip ci] ([`f36066c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f36066c5251a0d8f570defffa88b4a6ddd564193))
- update CHANGELOG.md [skip ci] ([`9d648a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d648a92521d85e82b14e071bcdcc10b93a2bb5f))
- update CHANGELOG.md [skip ci] ([`5896ed3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5896ed31bcfda9fa18d26b4ac12253c341fd4c56))

### Changes

- Bump version: 4.7.0 → 4.8.0 ([`ee2a89f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee2a89fb4393a017732b8cea8b3fc0e4f629e8e7))

### Documentation

- close internal-piping Phase 4 (client buffers documented) ([`a036811`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a03681158d3477a335961ae88678c36f7afe85e6))

</details>

## [4.7.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.7.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`d0df4d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0df4d924bb69a2ec95da131e45b997ddf9a3dc5))
- update CHANGELOG.md [skip ci] ([`b5f3892`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5f389281d99f819ba3905e7612db383287ca865))

### Changes

- Bump version: 4.6.1 → 4.7.0 ([`bdb56cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdb56cdd87c06573274b775e32cb1dbfc2d12db0))

### Features

- concurrent streamed PUTs via slot-aware streaming hooks ([`17cb428`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17cb42863fd482c8bc0ba99d1b938c22ee302ff2))

</details>

## [4.6.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.6.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`430801e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/430801eab0061e62d095113210104058adffb389))
- update CHANGELOG.md [skip ci] ([`51402ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51402abb1272288cceb573e7a844bbdcc66d4dcd))
- ignore .pio deps (Unity) so third-party findings stop recurring ([`ba52844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba5284411cc78dbe00b54360a5154d1b3d6ce696))
- update test report [skip ci] ([`513a523`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/513a52320f3d933d6a51193bf80b21ae5222706d))
- update CHANGELOG.md [skip ci] ([`5ee2d05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ee2d05c7e418f0710084007ceb1f698e62b4b5d))
- scrub from library.json; gitignore / ([`d04cb80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d04cb800651ac0fd0da3889a5c9a11f52be0cd5b))
- update test report [skip ci] ([`c69233e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c69233e4491540d472260ecca250578ff7a2fc7b))
- update CHANGELOG.md [skip ci] ([`b547143`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b54714344d70c69db3590841e9334a766a8aa477))

### Changes

- Bump version: 4.6.0 → 4.6.1 ([`6fa4ed6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6fa4ed617f11fa731010e972c39897154b1eb57c))

### Refactor

- single RX read API; consumers stop poking the ring ([`2c5d0ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c5d0bad560f63aad096155ded39e45562a39e20))

</details>

## [4.6.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.6.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`394eee9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/394eee92cfc5a53e532d8af2b077c2f992713111))
- update CHANGELOG.md [skip ci] ([`400794e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/400794eac8809b3eebccc33f0bdec2dd77c5a68b))

### Changes

- Bump version: 4.5.0 → 4.6.0 ([`f5cff73`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f5cff73e5b2542151077dc04e21b8ec8cf2e5293))

### Documentation

- roadmap TLS 1.2/1.3 + HTTP/2/3 (RFCs); README not-audited notice ([`b9b6362`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9b6362ce03d235e0972e539001c54d54e034ec2))

### Features

- stream PUT bodies to disk; make RX flow control deadlock-proof ([`5e1bcd1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e1bcd1af59c979c857a801c3c7bfc87efcfe0f1))

</details>

## [4.5.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.5.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`08af379`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08af379410f9dc2bae71788a65a0bb27094639d1))
- update CHANGELOG.md [skip ci] ([`11d151b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/11d151b2070bb92e92bb486baa46b944d0dc0742))

### Changes

- Bump version: 4.4.2 → 4.5.0 ([`e348e1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e348e1dad434cff7539ed9eda2a08fc57e78ee08))

### Documentation

- expand the educators note; sync README -> docs/README ([`4ffed17`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ffed17c6424072e20adad36c71e1b4575c753e1))

### Refactor

- pull-generator send_chunked, paged across loops (no truncation) ([`8ef4c6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8ef4c6c6e0d1bcaf325f234f937ffd971ce81a81))

</details>

## [4.4.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`37ba052`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37ba05243dcfdd52af0667f3d34c3962330ef10d))
- update CHANGELOG.md [skip ci] ([`562418c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/562418c67a0ee4f664142e347c8ef8a3ce49f3fa))

### Changes

- Bump version: 4.4.1 → 4.4.2 ([`3c01e80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c01e807cc06c6e9f9812e69f2b40cf00397896c))

### Documentation

- dual-license section (AGPLv3 always-open + commercial + educators) ([`ba5767b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba5767bbe70c4f6aaf4c06023e8d219699d0549c))

</details>

## [4.4.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.1 - 2026-06-28</b></summary>

### Bug Fixes

- page large file responses across loops, never truncate ([`48ce846`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48ce84606ca8630a3bb2cf117d5a8e1e8ff3ed35))

### CI / Build

- update test report [skip ci] ([`e378da3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e378da30cc7f6f9805a3278a6141baee2328b3e8))
- update CHANGELOG.md [skip ci] ([`30d97b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30d97b46cf4df01e30567233b72cfac7079cb18f))
- bump actions/checkout from 4 to 7 ([`3a71dc5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a71dc53d92ba8e609117d9f41ac5384db933a8b))
- bump github/codeql-action from 3 to 4 ([`3f0e462`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f0e462cafe2f0df76d0b60f5ccbf93c8f34699c))
- update test report [skip ci] ([`ef3a48b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef3a48bbfb7e7f0c47fc711b2193fc32bb19ed15))
- update CHANGELOG.md [skip ci] ([`5987236`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/598723690af035597488a4a705e8fea8b9cde80d))

### Changes

- Bump version: 4.4.0 → 4.4.1 ([`9f67ab0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f67ab06013bedf29d24eca62e4c0f49f114a50b))
- Merge pull request #4 from dstroy0/dependabot/github_actions/actions/checkout-7 ([`62593c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62593c9df205fc138aecbff6fcc5bbb468bfcb98))
- Merge pull request #5 from dstroy0/dependabot/github_actions/github/codeql-action-4 ([`d41f6e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d41f6e07f618a7615072b601ace86cb010a04059))

### Documentation

- add a prominent active-development / breaking-changes notice ([`af68d81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af68d8159faa5935e023677e1559709a9577d118))

</details>

## [4.4.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`28346fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28346fa2a54e05b70ee89535ae90b3796148f070))
- update CHANGELOG.md [skip ci] ([`4612b53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4612b53b089e49dbc937863d423074e01b46be67))

### Changes

- Bump version: 4.3.0 → 4.4.0 ([`0920424`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0920424b22f709d8777e5b2aa5a83a13c82e47bf))

### Features

- answer PROPPATCH with 207 Multi-Status instead of 405 ([`2235e72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2235e722ca21d1ec5b6ec08b46e667f6676a1bce))

</details>

## [4.3.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.3.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`6c65228`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c6522876a87a5f59a2d277a2b068f9d7a9747c8))
- update CHANGELOG.md [skip ci] ([`82355c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82355c11933975f93bf6649396ae127955c6197f))

### Changes

- Bump version: 4.2.0 → 4.3.0 ([`53b3141`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/53b3141f5a811e74c029ddf2345d8d167ec38db1))

### Features

- add a zero-heap MessagePack decoder ([`f43e281`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f43e2812fb32bb5b096c0952129a35fe167ed52d))

</details>

## [4.2.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.2.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`6e9d3c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e9d3c80b57e4008b9ac9cbb377dee8b563ecfa5))
- update CHANGELOG.md [skip ci] ([`778cff5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/778cff59bfc8e6b5a3632881a965418df543f5d7))

### Changes

- Bump version: 4.1.1 → 4.2.0 ([`74d0f5d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/74d0f5d4a42b8dbf6324244e51289c558d1c264e))

### Documentation

- add TUNING.md worker/perf guide; mark concurrency tuning shipped ([`c041c00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c041c0008ee4b9e7f5dd779ab09987ddb9bcdfa1))

### Performance

- bulk-copy received segments into the ring with a single publish ([`1bbd8e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1bbd8e5aed6de00c7f7576a35d7c7a59c8e6c368))

</details>

## [4.1.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.1.1 - 2026-06-28</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`5bf07e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5bf07e7f3e1c6426599de29c1147318b49eb6d2f))

### Changes

- Bump version: 4.1.0 → 4.1.1 ([`e88906e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e88906ea3f04edded5acd307fabfebd1300354e5))

### Documentation

- mark notification-driven worker drain shipped ([`9aa113f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9aa113f3d9c943938e9e97196a538307870ab402))

</details>

## [4.1.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.1.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`f7d6f12`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7d6f121b2e479932d0c1df5800a4a05c42293f9))
- update CHANGELOG.md [skip ci] ([`1ca1ded`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ca1dedd989fe4cf58d20989566c7198e266350d))

### Changes

- Bump version: 4.0.0 → 4.1.0 ([`2ad2bd9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ad2bd99e3fdaed9f131e1b67cd746245b765a04))

### Performance

- notification-driven blocking drain instead of the fixed poll ([`2ace854`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ace85490dcc6ba3e77982f801ef5e28392e1450))

</details>

## [4.0.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.0.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`9cfd7e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9cfd7e8fd33002cd9760bcfbf5e303f5cac00b51))
- update CHANGELOG.md [skip ci] ([`9781b50`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9781b509f3b953d1bba102d37ccdc41ae553cbba))

### Changes

- Bump version: 3.14.0 → 4.0.0 ([`4e79070`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e79070bd30893e07a9ea85e9f6be7fbe2ac183f))

### Refactor

- remove heap_needed/heap_available shims and the PROTO_NONE HTTP fallback ([`455ab53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/455ab53a03347290005d1cd024f7f36f93014fa9))
- fold the 12 codecs into per-component subfolders ([`866f421`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/866f4212672854a47b03628316b80b339196aed8))

</details>

## [3.14.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.14.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`e9f6c7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e9f6c7ffd84cd8ef80620cd06f9c991191b179fc))
- update CHANGELOG.md [skip ci] ([`00a66da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00a66da3176b2ba00444136b3016b18a8ac5379f))

### Changes

- Bump version: 3.13.0 → 3.14.0 ([`d54d72d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d54d72de4966b975e10d79978b5ebed563e33050))

### Features

- migrate ws_client onto det_client (Bucket 3b, 3/3) ([`9cba440`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9cba440d3a63d923ac13d36e157a82f74c067982))

</details>

## [3.13.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.13.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`7703a29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7703a29ce4b5c50be0894eba1ee33a7ec887d254))
- update CHANGELOG.md [skip ci] ([`5e6f3f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e6f3f6183ead2d23cd8eb407cdf741d25797f98))

### Changes

- Bump version: 3.12.0 → 3.13.0 ([`53099e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/53099e7a75eb4706905205c950423ffb46f60194))

### Features

- migrate mqtt onto det_client (Bucket 3b, 2/3) ([`2181a17`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2181a17de55c1fb2f79043a891863b1e26a9a6c9))

</details>

## [3.12.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.12.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a97d108`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a97d10807514f2b34a107f0e337a4b2619660895))
- update CHANGELOG.md [skip ci] ([`d43dd00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d43dd004e4b15bcd3310fc12cb3d61b6657e0ada))

### Changes

- Bump version: 3.11.0 → 3.12.0 ([`2063f6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2063f6ea7e31dd52d895f3a67f49999fd061a58d))

### Features

- shared outbound client API; migrate http_client (Bucket 3b, 1/3) ([`2411506`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24115062bf6721195c52b2a2a04f7b37d44bb0b6))

</details>

## [3.11.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.11.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`7efdbc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7efdbc751646859cef0c382766672d291e721835))
- update CHANGELOG.md [skip ci] ([`5ada2a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ada2a3dc4b47839e27379bec35257337d7e883d))

### Changes

- Bump version: 3.10.0 → 3.11.0 ([`5bd7afa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5bd7afa93b736cb38666475f2b4dd66fd7387bcf))

### Features

- make CONN_CLOSING a real dwell (Bucket 3a part 2) ([`87b490f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/87b490f454b74b05f8a89705e63509e11861d936))

</details>

## [3.10.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.10.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`1625732`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16257325c97a41588bf6dfb12f8a54751b1c5ee7))
- update CHANGELOG.md [skip ci] ([`134a8ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/134a8ec4212e92ed53b3b15b0253963fb76b4043))

### Changes

- Bump version: 3.9.7 → 3.10.0 ([`3c72803`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c72803856169ae2c89bfc2418284fa4b47e5b82))

### Features

- observability hook + counters (Bucket 3a, part 1) ([`12d1961`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12d1961deea2481ef988ee6d86fe9de345955bbf))

</details>

## [3.9.7] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.7 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`c450f1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c450f1fd7572f14720f19a1983723a66e07ea136))
- update CHANGELOG.md [skip ci] ([`aa6c130`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aa6c13083ae1bb187e0077c72d2f36663d196e34))

### Changes

- Bump version: 3.9.6 → 3.9.7 ([`0b89991`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b899918111efa12c67ae47244c0c7850bda14da))

### Testing

- live progress (counter + percent + spinner) in run_tests.sh ([`4e59944`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e59944bd236adc635710e467b3c8067e55ec6f8))

</details>

## [3.9.6] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.6 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`30aa45b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30aa45b298815714200637babb4d32230dc3ce76))
- update CHANGELOG.md [skip ci] ([`b906b7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b906b7ff62d5c9d1a957a853ecb8511b7e2020a1))

### Changes

- Bump version: 3.9.5 → 3.9.6 ([`c065794`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c06579497f25b1a5c49eda6ec78828afff2abff4))

### Testing

- table-driven env generator; runner auto-discovers all 60 envs ([`1314259`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1314259d7b5b0efbc6ac2a04b0e8ead43d9e25a4))

</details>

## [3.9.5] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.5 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`fb896b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb896b197081f3a0a44ef3f35174fa284cbcd81d))
- update CHANGELOG.md [skip ci] ([`5ecfede`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ecfedeeb1936686fb9a8130409ce15615121df9))

### Changes

- Bump version: 3.9.4 → 3.9.5 ([`5e4227e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e4227ebf2a655796abe40a549c78a5aa6a9811b))

### Testing

- expand host fuzz to 23 targets + add live adversarial driver ([`875c7e2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/875c7e2e23b84ca7a817a14d71687c265705f5da))

</details>

## [3.9.4] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.4 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`d06ba81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d06ba81423e65a582e3e355f4ccc8c91cbb301cb))
- update CHANGELOG.md [skip ci] ([`a86c692`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a86c692b9500700322b35a41030cae0c6aac76a6))
- update test report [skip ci] ([`3883a9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3883a9cf70bd20bba8caf6e2d11385c7b7b1e99b))
- update CHANGELOG.md [skip ci] ([`d0c45eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0c45eb64a41f65f56c2d91f6b348ef7ffa1623f))
- pin Prettier to 3.9.1 and reformat all markdown to match ([`0350f11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0350f1170d25aeff90d5172bca1177dd8c5ad670))
- update test report [skip ci] ([`780c57a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/780c57a4f8645e8ddcc27181ea489f3670fb121c))
- update CHANGELOG.md [skip ci] ([`7789767`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7789767071f2c2be51c7f8349a2dd48f555790f5))
- update test report [skip ci] ([`99dcd65`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99dcd65158f610ea7ca989c1205df29744775076))
- update CHANGELOG.md [skip ci] ([`f2079cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2079cd5db4cfc9d06f0f0ba9d0d1b4a821fcb8d))
- update test report [skip ci] ([`d6aa831`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6aa831e486c7da695f404b2490ed69e5c08451a))
- update CHANGELOG.md [skip ci] ([`e2d3202`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e2d3202118629dade70d228c1a65b853506b3633))
- update test report [skip ci] ([`0ca6b32`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ca6b32dae50652cae1b1758376f3468a41370e4))
- update CHANGELOG.md [skip ci] ([`42522da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42522da8b18d477eb124a81c719e7dee49910269))
- update test report [skip ci] ([`5adc0e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5adc0e05c00e937b793a0b503a6765b6b895979e))
- update CHANGELOG.md [skip ci] ([`47e7588`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/47e7588d63c78c1bf7e08b6c13492b79133a3276))

### Changes

- Bump version: 3.9.3 → 3.9.4 ([`0d929c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d929c00b10e8162bfe1369cc4af6f21a67b35f3))

### Documentation

- add annotated READMEs for L7 50-56 (Oidc, Vfs, GraphQL, EspNow, OAuth2, OpcUa, OpcUaClient) ([`3473442`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/347344233e1de0464291c0f8470ee38eb898daa8))
- add annotated READMEs for L7 45-49 (Totp, Webhook, RadioPower, DnsResolver, AuditLog) ([`b23133e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b23133ea891569179a34f09c61c5382572a7d02c))
- add annotated READMEs for L7 40-44 (Guardrails, LogBuffer, ConfigExport, ModbusScan, OtaRollback) ([`924003e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/924003e0d33a40f1db8f0f90b56c2af3bb341ffd))
- add annotated READMEs for L7 35-39 (Dashboard, NetEgress, PartitionMonitor, GpioMap, UdpTelemetry) ([`4bf18fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4bf18fdc55276c572c2752e716778cb416a3748e))
- add annotated READMEs for L7 30-34 (Modbus, TimeFallback, DeviceUuid, Csrf, Telemetry) ([`99dfedb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99dfedb5e3d11e7966f30812807cfd260152ee68))
- add annotated READMEs for L7 25-29 (WsClient, SnmpTrap, CoapObserve, CoapBlock, WebDav) ([`0ee2184`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ee2184d3ec10f815de38faa9ce9fffacb6b7aa5))
- add annotated READMEs for L7 20-24 (Diagnostics, Prometheus, Stats, HttpClient, MqttClient) ([`3610c1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3610c1fe3ab9a6f6a8632b4225aae6ae25e35ddd))
- add annotated READMEs for L7 15-19 (mDNS, OTA, Provisioning, SNTP, Syslog) ([`264d40c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/264d40cec4ee3209b4aa85df1cd1cc87837e5614))
- add annotated READMEs for L7 11-14 (Upload, Range, CoAP, SNMP) ([`c49a0d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c49a0d7a4af6cf4ba5e97de4e64e9a8d71fd5121))
- per-example READMEs for L7-Application 01-10 ([`d56103a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d56103a4a26e5308ad20d48d171a13e4e052f55d))
- add per-example READMEs for the L6-Presentation layer ([`0eea5e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0eea5e9da0c4c5bc2d85119b952c4d9e9f48724a))
- per-example READMEs for L4-Transport; make EXAMPLES.md links-only ([`f0c8ea8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0c8ea8b971c8478a9970059fa09b36fef19fb5b))
- add per-example READMEs for the L5-Session layer ([`0443496`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0443496611c81a577c0b3e43472473e493bab834))
- add per-example READMEs (Foundation layer) and the examples index ([`58b3977`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/58b3977f2a405b3ebdcf8dc116736a6eb989e8ce))

</details>

## [3.9.3] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.3 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`56a540c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/56a540ccfd5c194031e17ccefacf10667930bb2b))
- update CHANGELOG.md [skip ci] ([`e30fd3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e30fd3ffe82544b64d3f0af0ab2bbcf7185849e2))

### Changes

- Bump version: 3.9.2 → 3.9.3 ([`f8f7f71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8f7f710bff021902dba28808008f6e156dfbc52))

### Documentation

- redraw Squirty the mascot (smooth 64-grid squid) with live expressions ([`4091d3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4091d3bc837c81677e94304a16ef61e7c2b7ad38))

</details>

## [3.9.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a2216c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2216c9460750cb72397364338d1cb0bf2b984ce))
- update CHANGELOG.md [skip ci] ([`c7b7502`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7b7502207b210fb5e7957e788f6aed0934f6780))

### Changes

- Bump version: 3.9.1 → 3.9.2 ([`2eec7ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2eec7ef6d18b2f9b17e48a3ce41f77e8ae396e35))

### Documentation

- fix incorrect and missing source doc-comments from the coverage sweep ([`1041679`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1041679e79981f40d526ae62545c98435ae16c9e))

</details>

## [3.9.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`45b9ab5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45b9ab5adbb3710caabc42fdbd4e75a4317ea9a6))
- update CHANGELOG.md [skip ci] ([`37c11c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37c11c031e4b79986c4e3ef37a03837d4547fd0f))

### Changes

- Bump version: 3.9.0 → 3.9.1 ([`1906e37`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1906e37f52cf818cb7db247c9df5e1d4a8e0da37))

### Refactor

- move the connection source-IP read into L4 and drop dead config ([`8a814f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a814f5745690780e8043af8532bf2914b3a3255))

</details>

## [3.9.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`ad394b5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad394b59ad52cc1b15d313ecb07de184d59381bd))
- update CHANGELOG.md [skip ci] ([`a062109`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a062109860d48dd515c43b3a4e2b59e65b72c06b))
- add contributor/maintainer health files and npm tooling manifest ([`90e41cf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90e41cfd56b5a95334aa04eae50eb77c33026578))
- update test report [skip ci] ([`610ea2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/610ea2b8b60376da6e2833a22528d443703f0727))
- update CHANGELOG.md [skip ci] ([`4ad530a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ad530af60606f1589b14333bb32faae38f2695a))

### Changes

- Bump version: 3.8.4 → 3.9.0 ([`b1f0b41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b1f0b41c47b8610c9d4b8e60976e07a725ec644d))

### Features

- validate build-flag dependencies and document the dependency tree ([`679590c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/679590cd3a7a776cfe542f909c300a377db80a1d))

</details>

## [3.8.4] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.4 - 2026-06-28</b></summary>

### Bug Fixes

- add missing esp_wifi.h include in EspNow; correct stale @file tags ([`8ad72b8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8ad72b86e966f9eaec280564963fb69e89bb3adb))

### CI / Build

- update test report [skip ci] ([`0374a77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0374a7723d524f52e93a667f82bfb0706a09ddbf))
- update CHANGELOG.md [skip ci] ([`8830a04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8830a044a35bf8f1d1afc3207edd5a0514077f58))

### Changes

- Bump version: 3.8.3 → 3.8.4 ([`cbc90ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cbc90ba360ff2b33107e814ff590d7ce9eae18d0))

</details>

## [3.8.3] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.3 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`f8e6952`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8e6952a20c1aa1ebbfe2263e2a6c9fb118f59ec))
- update CHANGELOG.md [skip ci] ([`96ae1ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96ae1eeaf3a799ef0dd50307814d246e23c3adfd))

### Changes

- Bump version: 3.8.2 → 3.8.3 ([`500b9d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/500b9d93c5b04ef283a8eec84b3b1868b201af48))

### Documentation

- fix accuracy gaps found by the documentation audit ([`2c0f2c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c0f2c2137770420abb96a01c0737b53fd2d9654))

</details>

## [3.8.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`1dce842`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1dce84273b19a646f223f87bfee1d1359dbc17d4))
- update CHANGELOG.md [skip ci] ([`2570153`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25701532a1a934bfb7748395d946feb761e801eb))

### Changes

- Bump version: 3.8.1 → 3.8.2 ([`0a018d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a018d01e448915359b184fe7f12735b691a99df))

### Refactor

- regroup all 85 examples into OSI-layer folders ([`3345ad0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3345ad00f8d92dd816dc2537388991fee86ef45b))

</details>

## [3.8.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`da20458`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/da204580c19c4c26127f09d5a86462d7f8d9b4f9))
- update CHANGELOG.md [skip ci] ([`c73c4a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c73c4a33d2cd125a746a80ce7793ba9cf69b1977))

### Changes

- Bump version: 3.8.0 → 3.8.1 ([`c69d0d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c69d0d44c20df99c13ffd881d810fdc33331be16))

### Documentation

- surface OPC UA + full protocol set across the apex docs (audit) ([`daf1232`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/daf1232dcbba58200598135a64cddc4bac9e6667))

</details>

## [3.8.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a3cb391`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3cb3912deeba2acd6b6fc03f7760944f15d2149))
- update CHANGELOG.md [skip ci] ([`1661f57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1661f57dc7d39448dfe9f73699ab6b04f9c20c2b))

### Changes

- Bump version: 3.7.1 → 3.8.0 ([`36b887b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36b887b6d02ecefd9d4d3ee345ba931d1810e751))

### Features

- OPC UA Write service (DataValue/Variant decode + write resolver) ([`72257a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72257a114071ecd1959d5c8bde9e7ac046697b34))

</details>

## [3.7.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.7.1 - 2026-06-28</b></summary>

### Bug Fixes

- spec-compliance bugs found by a library-wide protocol audit ([`f692b5c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f692b5cab83949ee84441ce4007972760f49045b))

### CI / Build

- update test report [skip ci] ([`51efec5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51efec507479064d785c121b8559b9841eaec8e9))
- update CHANGELOG.md [skip ci] ([`511ea89`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/511ea894919797b15e48994fabca5dcc1ba13589))

### Changes

- Bump version: 3.7.0 → 3.7.1 ([`dc13764`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc137648b0d1690464ce9ef98196f676f02ec67a))

</details>

## [3.7.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.7.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`acef7bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/acef7bbc0e5d7b1b9b07431d1c2b5da4d622564c))
- update CHANGELOG.md [skip ci] ([`ed2f70a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed2f70a567f4d0f40bf8b95cbc7d8f05ddb13358))

### Changes

- Bump version: 3.6.0 → 3.7.0 ([`f57d2bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f57d2bc11e58108585379d68bf04a0f5f2c29a45))

### Features

- OPC UA GetEndpoints + ServiceFault + spec-compliant MSG framing (real-client interop) ([`ae0e493`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae0e493854faf8cc1671f15f94c60f607c27742a))

</details>

## [3.6.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.6.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`376bcbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/376bcbee03945f3de9cce98046252ad7967cbc09))
- update CHANGELOG.md [skip ci] ([`d88d7d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d88d7d62154db614317a8b9b9a7ead0cab412ac3))

### Changes

- Bump version: 3.5.0 → 3.6.0 ([`f77ad11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f77ad11ec7de6d02ebf7d4f4e7edc942948d1588))

### Features

- OPC UA Binary client module (services/opcua_client) ([`6a70eae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a70eae9dab0f59918136b2c40b8f125a3b648fc))

</details>

## [3.5.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.5.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`5b7cca2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b7cca2b0b658daa7ca425aeab6723e297a37b05))
- update CHANGELOG.md [skip ci] ([`e697fd7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e697fd7251669f18e4e51b87996e4eef25e4116b))

### Changes

- Bump version: 3.4.0 → 3.5.0 ([`b3bf3b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b3bf3b9472675c6b5277a955b256d800f79b653c))

### Features

- OPC UA Browse service + CloseSession ([`e4e2a5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4e2a5a74be6277ba0f0b989d2b66b5a56f0ae10))

</details>

## [3.4.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.4.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`06e4c6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06e4c6c1c5dde383bb498bbcc876b597f65b9494))
- update CHANGELOG.md [skip ci] ([`be198b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be198b4e39a611a3e7e7f4d3c2a8df4cbc7055c7))

### Changes

- Bump version: 3.3.0 → 3.4.0 ([`f9c6b6f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f9c6b6f5c85491e21749c549f869f258a69b853d))

### Features

- OPC UA Read service (Variant/DataValue + registered resolver) ([`09f46db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09f46db7af655328ebea2c3685a83e622478fc44))

</details>

## [3.3.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.3.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3c61087`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c61087d09ef8c53800b2744c34c2603367817c2))
- update CHANGELOG.md [skip ci] ([`7f12c86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f12c86a552a851df2f29ee03414c88c7e3e23f5))

### Changes

- Bump version: 3.2.0 → 3.3.0 ([`4e26583`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e26583c4b6f13d496f79aaa8f27032da0abf28b))

### Features

- OPC UA Session (CreateSession + ActivateSession over MSG) ([`6b74f1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6b74f1c468adbd6cc4e0e5bd0bf6d4c2f949c519))

</details>

## [3.2.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.2.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`0b02e1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b02e1a9501b8e1efa962ca038eea3ded1bffb15))
- update CHANGELOG.md [skip ci] ([`25895ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25895cefe92686af52642f07f5d1bac4b70999c3))

### Changes

- Bump version: 3.1.0 → 3.2.0 ([`30d0e0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30d0e0beae37148d2203c860c3e9f2a8be441490))

### Features

- OPC UA SecureChannel (OpenSecureChannel/OPN, SecurityPolicy None) ([`6297475`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6297475eabda802ed885dda5c5b6989d14c9e4bc))

</details>

## [3.1.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.1.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3fbe970`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fbe97024296a995c0b4c12b7f472980a86f2b54))
- update CHANGELOG.md [skip ci] ([`c7ec6a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7ec6a1dc05dc32104ff49d0bb26d1793a5db80e))

### Changes

- Bump version: 3.0.0 → 3.1.0 ([`9bc8b0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9bc8b0ff0ffbe73b08b76095775ca532b7295091))

### Features

- OPC UA Binary server, increment 1 (UA-TCP + Hello/Acknowledge) ([`ead3edc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ead3edc29b3a54c0ff0a5e3960abd30d4a3ae9bb))

</details>

## [3.0.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.0.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`45680d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45680d4d25e44de4e05a183791b4ef27f73fa5fe))
- update CHANGELOG.md [skip ci] ([`262d220`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/262d2203b36eddd5e0b3953c9bdbca821c0b0dff))

### Changes

- Bump version: 2.36.0 → 3.0.0 ([`f3aafd3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3aafd3e2807793a4f1d3d5ae1821a7102d40286))

</details>

## [2.36.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.36.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`873e4c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/873e4c72c397105e9891f4829c0a7df46e717415))
- update CHANGELOG.md [skip ci] ([`f7fad94`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7fad94f0389bb9e30731f4b90ded1187aa281ab))

### Changes

- Bump version: 2.35.0 → 2.36.0 ([`23d5dd5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23d5dd551f08d3cea96d39f76e46725f24f60994))

### Features

- OAuth2 token-endpoint client (authorization_code + refresh) ([`3a02051`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a020519525c3a6ea4ed7d33c467a715b00bcd1f))

</details>

## [2.35.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.35.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`4265730`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42657303e7428f64b0d998d615a0c694eb964d62))
- update CHANGELOG.md [skip ci] ([`8d9a716`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8d9a7164ba1765438c904774da6580ac12851142))

### Changes

- Bump version: 2.34.0 → 2.35.0 ([`7769cb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7769cb97b9118a1b3561842d0f5bac55e4d9e800))

### Features

- ESP-NOW peer messaging (typed envelope + peer registry) ([`0bf7b42`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0bf7b42707460845c7e26d586b9db8a1352636d6))

</details>

## [2.34.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.34.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`6096d83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6096d832af3d59945adcb17a5808464888a62dfd))
- update CHANGELOG.md [skip ci] ([`8081b2c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8081b2cc520963501c777d6ef7deaf0943af65f6))

### Changes

- Bump version: 2.33.2 → 2.34.0 ([`4a6a379`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a6a379e988bf4f69e165b92ce7cd11097e9f860))

### Features

- GraphQL query subset (zero-heap parser + executor) ([`18ba5ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18ba5ef6f8f1fe4d06a00344fdaca076be824afc))

### Refactor

- drop <stdlib.h> from the library (no-stdlib number parsing) ([`201f992`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/201f99286f2814c8ad970febb960682fe4e015dd))

</details>

## [2.33.2] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.2 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`fd2dde2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd2dde275cb89c7d58c4e1751e33b638d5ed4baa))
- update CHANGELOG.md [skip ci] ([`6dc1b34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6dc1b34fc7733d6d3678e54e2dc46e374b9fa5ad))

### Changes

- Bump version: 2.33.1 → 2.33.2 ([`4112a9b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4112a9be0245f12e2db6f0e07ed2169269943aaa))

### Documentation

- ESP32 Secure Boot + Flash Encryption hardening guide ([`ebf288c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebf288c16b994dccc8d701cb75f3cc51cd3b239d))

</details>

## [2.33.1] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.1 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`0eaf4d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0eaf4d22e0e2d04744688f0404acdbabf270d20d))
- update CHANGELOG.md [skip ci] ([`25854e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25854e04e8afff16d6728b0a28ec046595280349))

### Changes

- Bump version: 2.33.0 → 2.33.1 ([`862d620`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/862d620fb16990b84f31872f94b30a19f9ef41d5))

### Documentation

- feature table with hover tooltips + docs/FEATURES.md ([`854ccb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/854ccb256307cf39ea4e7130b09b3d1a758a35dc))

</details>

## [2.33.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3b540d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b540d46b3674c64f15de342015a0e5650eda2e1))
- update CHANGELOG.md [skip ci] ([`f8c5fc9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8c5fc992dabecdb57e7cc695d93300d144acfff))

### Changes

- Bump version: 2.32.0 → 2.33.0 ([`4b7fb09`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b7fb096804904d5a9d1c379bcea48c2f934c5a3))

### Features

- unified VFS wrapper (RAM + Arduino FS backends) ([`7446668`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7446668c001ff9a734d5fd164c69c44569f03999))

</details>

## [2.32.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.32.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`9ee1ca3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ee1ca327626d7189650a148cb59c589947537a0))
- update CHANGELOG.md [skip ci] ([`554307d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/554307d182c7cb4e2fb4e1f54413628c30ab3994))

### Changes

- Bump version: 2.31.0 → 2.32.0 ([`36dbb84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36dbb841450e08e1c7c644088de31885ec72e584))

### Features

- OpenID Connect ID-token verification (RS256) ([`5c21aaf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c21aafb1c715e6fa497b1cecef5115e198ec759))

</details>

## [2.31.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.31.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`bf5d9f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf5d9f5d65af32416577ecbc6ece2bbe5645bfa3))
- update CHANGELOG.md [skip ci] ([`84c16d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c16d1e19e2d979a57aa97e6b087e09b6d30cf8))

### Changes

- Bump version: 2.30.0 → 2.31.0 ([`3a33f18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a33f187205ecc363115dc945dfc626637311840))

### Features

- tamper-evident hash-chained audit log ([`012b6e3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/012b6e3ac2d44fdae160e6ff53298e1644749d10))

</details>

## [2.30.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.30.0 - 2026-06-27</b></summary>

### Changes

- Bump version: 2.29.1 → 2.30.0 ([`711bf43`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/711bf43d5aa8f520659216c66600908a178e8a4e))

### Features

- WebSocket permessage-deflate outbound compression ([`ef9babd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef9babdc1081609090acfe685970fddab28f0d0a))

</details>

## [2.29.1] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.29.1 - 2026-06-27</b></summary>

### Bug Fixes

- never HTTP-parse a WebSocket-upgraded slot (first-connection drop) ([`dc83dd5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc83dd5b406dbc30ae01b517fac6cbca988c5a8e))

### CI / Build

- update test report [skip ci] ([`4ac1ce6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ac1ce6ba09f8746793d5c1f2698b5b592869ff8))
- update CHANGELOG.md [skip ci] ([`7db045d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7db045d30dbbc5e3df2f96a447b5dbcd1a5acd6e))

### Changes

- Bump version: 2.29.0 → 2.29.1 ([`976d66a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/976d66abca34e8d1b5db13e629a080c849f42c06))

</details>

## [2.29.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.29.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`f39cb16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f39cb169d71806247924517360e3260ffaa98075))
- update CHANGELOG.md [skip ci] ([`c136f6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c136f6cf6acdcd3200e0451d176cd4f855f53218))

### Changes

- Bump version: 2.28.0 → 2.29.0 ([`7092cb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7092cb29eafcd6178cb223b2e15106bb1d91d482))

### Features

- DNS resolver with answer verification ([`f904e4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f904e4ab220175c59d2fa8fb8ecf8746fc2d41d7))

</details>

## [2.28.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.28.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`786ab30`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/786ab3077a4922ace8995aea6a8d79744a038d86))
- update CHANGELOG.md [skip ci] ([`0ca3b74`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ca3b74d411b98784e9509c037f3e1aa94e6e630))
- update test report [skip ci] ([`15f3582`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/15f3582bf5fe65394171db6310d5f6fac51c6b85))
- update CHANGELOG.md [skip ci] ([`94c7b88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/94c7b887cf074e43caab76e663d63a4862c027e5))

### Changes

- Bump version: 2.27.0 → 2.28.0 ([`d311adf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d311adfafe53d95c3d2114ae1cc1b1a20becc82d))

### Features

- WiFi radio power controls (modem-sleep + TX cap) ([`60333ff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60333ff3779d2fcc03a9b22ad56650cdfe45d84d))
- pentesting / adversarial suite + guide ([`68e1c6a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68e1c6ab3b832e687785ab75c0e1c7f32b7c4a97))

</details>

## [2.27.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.27.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`1f6ea1e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f6ea1efd27b1f957f535e98b5f20783ed5cd9e8))
- update CHANGELOG.md [skip ci] ([`731bf19`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/731bf198007a2115a1cd6b6b1639a1c5e8f5f06e))

### Changes

- Bump version: 2.26.0 → 2.27.0 ([`0649646`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0649646b4ac5570bb8a5f01354150cff2c239c32))

### Features

- outbound webhooks / IFTTT ([`655951b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/655951b241a90f3b8c56c9cabe0f52c96144a025))

</details>

## [2.26.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.26.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`519d982`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/519d982c6a889d97d0eec1cb06a7c00be550011f))
- update CHANGELOG.md [skip ci] ([`59ab16a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/59ab16aabb2322b08bd7995add31089e3aacac4c))

### Changes

- Bump version: 2.25.0 → 2.26.0 ([`6f49e2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f49e2bedb4068f2d1c5ff42fbef7556c9212dba))

### Features

- TOTP two-factor auth (RFC 6238) ([`a718d5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a718d5b6c1a1483011b53f8882b5eb805ba9e7e9))

</details>

## [2.25.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.25.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`25b1402`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25b14028f9d5fc5d29100674b6846f779bea435b))
- update CHANGELOG.md [skip ci] ([`6793b1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6793b1afbb3b1b5252c1b3c9f4fb63a7292b5e2d))

### Changes

- Bump version: 2.24.0 → 2.25.0 ([`6ecbff6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ecbff631cd607465ce8f78d117992e7573da61c))

### Features

- pluggable monotonic clock + compile-time worker poll-rate knob ([`c634a5e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c634a5e14ec2ca6a37faafae519743930e8704c3))

</details>

## [2.24.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.24.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`ba1e38f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba1e38fe4a874d300447083febb6be7d5e45314a))
- update CHANGELOG.md [skip ci] ([`92d8b02`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92d8b02eef5e0c059c93d61e6b2dafa79f1ff37f))

### Changes

- Bump version: 2.23.0 → 2.24.0 ([`9d38b0d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d38b0df5e3b50e53194c9cdb65b70227c64f0a3))

### Features

- OTA rollback protection + soft-brick safeguard ([`5a022a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a022a784661d44d06a9100990d0c2d79d6ffd6c))

</details>

## [2.23.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.23.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`b53b085`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b53b085477dbb33f5b32d3b168ad7678fd5aa61c))
- update CHANGELOG.md [skip ci] ([`328ae49`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/328ae49d04623412a7f1c0ee39f80bd8671a53bb))

### Changes

- Bump version: 2.22.0 → 2.23.0 ([`eab8e04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eab8e04dec3630c7708cb15c1fc5ed55cbca8382))

### Features

- Modbus master codec + register scanner ([`efa789d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/efa789db5f7a9df90ca865477c35e917f7c08154))

</details>

## [2.22.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.22.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3b428a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b428a066f9017107448e733a03f6d1953e31df5))
- update CHANGELOG.md [skip ci] ([`6c7fffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c7fffed89d5368b433d0fccea79c14cf2f07195))

### Changes

- Bump version: 2.21.0 → 2.22.0 ([`26b7f48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26b7f48b68aa61b13cf878952ddf21791211e686))

### Features

- schema-driven config export/restore ([`08ee802`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08ee8028aade673ac4e536078fc5321ee7561f32))

</details>

## [2.21.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.21.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`c98657a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c98657a5fbf671acb8e06e2b1e478a9b4b421e14))
- update CHANGELOG.md [skip ci] ([`90a34f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90a34f769f68dedc7430f084510ad6d0a7ff88e3))

### Changes

- Bump version: 2.20.0 → 2.21.0 ([`ce1bcca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ce1bccade8d3241b2375e7ea738e1098cd06ffa9))

### Features

- fixed-RAM rotating log buffer with severity traps ([`23af94a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23af94a14f5b4c4e557b39e55e2dca3a2fb9dcc5))

</details>

## [2.20.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.20.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`1b5cd23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b5cd23ca975e2d73c0b53fe21106f5fd4e95eb1))
- update CHANGELOG.md [skip ci] ([`e7b971d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7b971dcd1cdf89401fcd259c06e1769a177b1f4))

### Changes

- Bump version: 2.19.0 → 2.20.0 ([`794a655`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/794a65523dbcee80bb8f0ec8f8319aa2b137ce6d))

### Features

- runtime heap/stack guardrails with breach callback ([`fae0b29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fae0b297b8d5d2cc462fcacce39d491f4249e5dc))

</details>

## [2.19.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.19.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`32f2772`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/32f27721ebea318705b98bd2494b9dc005980b4d))
- update CHANGELOG.md [skip ci] ([`f31a1c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f31a1c6537d400484c35be98438a2b0e0b633c55))
- update test report [skip ci] ([`a350a0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a350a0f647bbfd97e0bc9d590e932d07dd92d6cc))
- update CHANGELOG.md [skip ci] ([`c4a6da3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4a6da3d7777e9be2395e56f93240f3db9f2134d))
- update test report [skip ci] ([`a244f15`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a244f1533fa70523a98a508fd7d1aed3ccc90da3))
- update CHANGELOG.md [skip ci] ([`338b51f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/338b51f79fe9b47746b4efcaf4b99ba39e4bd703))

### Changes

- Bump version: 2.18.0 → 2.19.0 ([`a5541c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a5541c8039ac0da840d6df0c1835a958317f9b48))

### Documentation

- record worker throughput benchmark (N=2 ~1.5x under concurrency) ([`e7fb89b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7fb89b28f8e491b48fb8dd16e7c74b602baa289))
- document the worker model (concurrency roadmap + README) ([`8cf7838`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8cf78389d505a6ace7462ffad5b7341cb3ee7759))

### Features

- zero-heap UDP telemetry cast (InfluxDB line protocol) ([`2c5e156`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c5e15691c04d8d943142aac3b73b36ee42d16e4))

</details>

## [2.18.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.18.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`563f71e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/563f71ec4ff2a1d78592009a681286ce4f9ac6a5))
- update CHANGELOG.md [skip ci] ([`6a18be1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a18be19c3953295c276908b8e304af5c83262f2))

### Changes

- Bump version: 2.17.0 → 2.18.0 ([`a3b72f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3b72f25dc188c5eff27b36981c3a154538c5be9))

### Features

- thread-safe deferred-callback path for app pushes to workers ([`ac0eccd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac0eccd0942c2c67d1b1757e7fedf82abbb883d5))

</details>

## [2.17.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.17.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`6d4bf5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d4bf5b787a7349da1b9ae6ed0d9bab59fbcc8fc))
- update CHANGELOG.md [skip ci] ([`e6254de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e6254deab6a13ff1a2468da802adb9a723aa4db1))

### Changes

- Bump version: 2.16.0 → 2.17.0 ([`024739f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/024739fb0241fcf25ffc0ae49751618dea8c63c4))

### Features

- core-partitioned parallel workers (DETWS_WORKER_COUNT > 1) ([`60baa1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60baa1b3e9858bc3f71775aac687a0f63b132576))

</details>

## [2.16.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.16.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`5196ba6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5196ba68f9220071c04efb4438e86b6b2df38b6e))
- update CHANGELOG.md [skip ci] ([`3f9c68c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f9c68cfa795448220d637b29da3a96bd31731d7))

### Changes

- Bump version: 2.15.0 → 2.16.0 ([`9ab38f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ab38f5d4dd2f18d4fb822cef3c118be5febd145))

### Features

- run the server in a dedicated worker task, freeing loop() ([`0768a4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0768a4f741e375472515533835296e4a9a0c5ed6))

</details>

## [2.15.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.15.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`9eac7a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9eac7a25badf0185301545cf009f1b086740d88e))
- update CHANGELOG.md [skip ci] ([`23ace18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23ace18a7cbb48407c3c3da7eca0210b91d939c3))
- add msgpack encoding terms and changelog hash to cspell dictionary ([`6f7b56b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f7b56b7617bd8340aa781e703bd629b42c94ee8))
- update test report [skip ci] ([`7b62407`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b624075e9903865eb469e711620de9fbf1ff99a))
- update CHANGELOG.md [skip ci] ([`a8c7b19`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8c7b19104d3492447b44a8bdc0f2f7f9d494012))

### Changes

- Bump version: 2.14.0 → 2.15.0 ([`fa3b83b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa3b83b4448620665c917c19f4287a87c180541e))

### Features

- thread-safe transport groundwork for the worker model ([`71dbd72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71dbd729d978b94280865f6101c452540af395ac))

</details>

## [2.14.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.14.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`554907b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/554907baf6e2b4e6bb7b523fb7e50b6b86221b37))
- update CHANGELOG.md [skip ci] ([`de76522`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de765223f70d451beb832491e0e0acc81f16c5ed))

### Changes

- Bump version: 2.13.0 → 2.14.0 ([`48e679b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48e679b1b4a9dbf82c2f797a5d2b627497c81c76))

### Features

- browser GPIO pin-mapper diagnostics endpoint ([`c0857bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c0857bc1e96ed89d447e4c62e68424ed44a3ea25))

</details>

## [2.13.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.13.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`f0bc212`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0bc212a07ba92406158ef2238c371e7ba0b6dbc))
- update CHANGELOG.md [skip ci] ([`8b0cffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b0cffec4fd1b6a143c428215f8ffc76876dbd83))

### Changes

- Bump version: 2.12.0 → 2.13.0 ([`ed77db5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed77db54b56786b74db5bec76a99d28f814d7894))

### Features

- zero-heap MessagePack encoder ([`0354ffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0354ffe1d8d3d79c59baef7ccf8c6b8662f00980))

</details>

## [2.12.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.12.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`12ed60f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12ed60f58d13198203edc13de39fa798c4e5117a))
- update CHANGELOG.md [skip ci] ([`110e56c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/110e56c106143067e19db4a90964e7ea2d08b681))

### Changes

- Bump version: 2.11.0 → 2.12.0 ([`fc9cacb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc9cacbdd20de5191165dcf6f32e66c0d253eae8))

### Features

- granular JWT scope/role authorization helpers ([`5dfedb1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5dfedb1573bb98eaf875e48fceb6b976a53eb041))

</details>

## [2.11.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.11.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`a20f887`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a20f8878dc8149612e0534f4475f7a9d5f2dc729))
- update CHANGELOG.md [skip ci] ([`b9da393`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9da3937bac349927eb4d3f9f7562bcd33a85d1d))

### Changes

- Bump version: 2.10.0 → 2.11.0 ([`1ae8c46`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ae8c4674fc958cc5caaf63bc9ec726b481bf109))

### Features

- CBOR decoder (cursor reader) ([`c053c07`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c053c07c3b8416af29806f4119e0c70f804c14f4))

</details>

## [2.10.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.10.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`847014c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/847014c74a08e87dfe6c107276190abf4ff7423b))
- update CHANGELOG.md [skip ci] ([`305ded5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/305ded54d8b08b5921e97e9e55c743a70cf79ac2))

### Changes

- Bump version: 2.9.0 → 2.10.0 ([`d1756eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1756eb940d611b5a621f30377b2320f6fa7025a))

### Features

- zero-heap CBOR (RFC 8949) encoder ([`00a4bae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00a4bae29976a28368de6ed1ee4f005fca8c4264))

</details>

## [2.9.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.9.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`2b1c080`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b1c08009e7a38a77b38b7d2cd7ea6fa6d3df968))
- update CHANGELOG.md [skip ci] ([`dd4ba62`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd4ba627d2264418c6e97f2ec7b7ba8963af6173))

### Changes

- Bump version: 2.8.0 → 2.9.0 ([`67243c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67243c0bba3ad6dfa15bcb14ec09a37b41bb9364))

### Features

- flash partition-map monitor endpoint ([`5fbc48a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5fbc48af0a1f93a8827694db4ba2ca13a01cceee))

</details>

## [2.8.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.8.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`c264beb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c264bebdb5d1d1b410e1febd710681feff77ac06))
- update CHANGELOG.md [skip ci] ([`aedff6a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aedff6a05fc9af1d6e44f75377bace8d33bbe0c0))

### Changes

- Bump version: 2.7.0 → 2.8.0 ([`2e20222`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2e202224dd1e2ae0a62f743dc278a4645c5d29b1))

### Features

- egress-interface reporting (det_net_egress) ([`56682af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/56682affa12a8cd16e91b3c7c7902ff3f01a65ab))

</details>

## [2.7.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.7.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`a9ced97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a9ced9750067560c88cdaf32b60f0b4170b1e215))
- update CHANGELOG.md [skip ci] ([`7050872`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/70508720daa67712f70a5daa7eb1a60abff3952c))

### Changes

- Bump version: 2.6.0 → 2.7.0 ([`f32cf87`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f32cf876ea97726950cabccb6c311788350a5f15))

### Features

- dashboard WebSocket controls and Canvas chart (phase 2) ([`dc7e703`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc7e7038c3b0bed260168271c8138e5bf18628d7))

</details>

## [2.6.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.6.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`4929167`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4929167fa8f37158a860b681b05b8e5c9da3a338))
- update CHANGELOG.md [skip ci] ([`04f3061`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04f3061272c6f4bfb9b337032d249087cbad1f63))

### Changes

- Bump version: 2.5.0 → 2.6.0 ([`bfa2483`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bfa2483ed078cffb543e262928143d2df891b706))
- clang-format the 61.Telemetry example ([`a59a546`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a59a54674e6674e034c8681069c0528b9ae95cb7))

### Features

- real-time SVG dashboard over SSE (phase 1) ([`048c457`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/048c457a0bb218c78e1d10afd260260414329d0d))

</details>

## [2.5.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.5.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`91d304b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91d304b0c01c809501119a52dc29b0558f1daa40))
- update CHANGELOG.md [skip ci] ([`e8de582`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e8de58237028642cb88b915460c350019200c9cc))
- update test report [skip ci] ([`ace19b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ace19b392c81ad1454e37005f16de85aca89c275))
- update CHANGELOG.md [skip ci] ([`4558d97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4558d97375e89e617a64a01abd8fb191cced7976))

### Changes

- Bump version: 2.4.1 → 2.5.0 ([`9922d9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9922d9de9e177fc24e491faef268c9848dedc99e))

### Documentation

- link CodeQL / Roadmap / Known-Limitations pages from the docs index ([`334028a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/334028ac02ae427e92acf7b350192bbb538bc80f))

### Features

- telemetry math helpers (moving-window stats, rate-of-change, totalizer) ([`84c6170`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c6170b1aed6f91e8c2038acbee2b57f456e516))

</details>

## [2.4.1] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.4.1 - 2026-06-26</b></summary>

### CI / Build

- registry-ready library.json (homepage, headers, export rules) ([`2a61268`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2a61268293161a17735cbe64f578e982bfd03966))
- update test report [skip ci] ([`fcca10b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fcca10b61212b0652070fdd03e895586ca8add06))
- update CHANGELOG.md [skip ci] ([`fd1a2b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd1a2b269ea2acf52c4abb8fef7c208de57f7dc4))
- expand CodeQL coverage to the new modules and integration paths ([`3703e86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3703e86676f41d74d2d243e46889b28a5a383700))
- update test report [skip ci] ([`f9d355a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f9d355a264bdd75eb69a6cc65a53aaafeb0a2919))
- update CHANGELOG.md [skip ci] ([`19d3a76`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/19d3a76d28ba6b5d544ede79f1a3a8f8c6989e1f))
- update test report [skip ci] ([`f935fad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f935fad0552c2a9bddb20d3725947565450e0b11))
- update CHANGELOG.md [skip ci] ([`fd4e3a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd4e3a031a4a13ee2d7836d8083eddfd3c18db4e))
- only SSH-sign bot commits when the signing key secret is present ([`a7543fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a7543fac993055958561e5e2321b4350cc00d89e))

### Changes

- Bump version: 2.4.0 → 2.4.1 ([`7db5bf5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7db5bf591bb4ffb3892bdee86fa4b6d1cce1d45a))
- less homicidal squirty ([`edbb9d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/edbb9d66ba831bca3bdb49a62220ec9a684d1f6f))

### Documentation

- add pentesting / adversarial suite to roadmap ([`b386a50`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b386a50473c44d12f08dfa3951f40fd94bb42cbb))
- group README features by OSI layer in collapsible blocks ([`01d95f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01d95f7335d861871075dc633ce94ead1a896028))

</details>

## [2.4.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.4.0 - 2026-06-26</b></summary>

### CI / Build

- SSH-sign the changelog and test-report bot commits ([`dfa89e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfa89e4e23479b9f05cf7a2d89074b15d3ab773c))
- push changelog and test-report via the automatic-actions deploy key ([`940dece`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/940dece0740510c78d5f6ca136c4dcb31354aa13))

### Changes

- Bump version: 2.3.0 → 2.4.0 ([`4185c7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4185c7feed2959dc271db6c26907a673d7c45e24))

### Features

- CSRF protection for state-changing requests ([`6d0e17c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d0e17c19d6fd68f7a6efe3428b0df16ff05c9fd))

</details>

## [2.3.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.3.0 - 2026-06-26</b></summary>

### Changes

- Bump version: 2.2.0 → 2.3.0 ([`a6cdac1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6cdac12a6c1a35a8ad0f0c4fa0ed8eed2d9486d))

### Features

- per-IP brute-force lockout for HTTP auth ([`1bcdd03`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1bcdd03d0ee700482ee3bb54bc2cbb535b773d70))

</details>

## [2.2.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.2.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`0c199a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0c199a33eb912ada1dcad26dc671745911d69376))
- update CHANGELOG.md [skip ci] ([`3fd5992`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fd59927ac647abeda8b1caa3dd19cd1ebf30d6a))
- update test report [skip ci] ([`aae0666`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aae066607f952e0a54f87749a3365832090be093))
- update CHANGELOG.md [skip ci] ([`d21d17d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d21d17d01b311e95894c407229c36b4dca6e8e74))
- add CodeQL analysis workflow + badge ([`e3ef7df`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e3ef7dfa2bfe6f3e804997d086432ca5fb69b00d))
- update test report [skip ci] ([`04b7ae8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04b7ae8c46a818a22549ffb35dfb2c66aa35cc54))
- update CHANGELOG.md [skip ci] ([`3c82c25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c82c25a5e02a1fcdb192223fdcd9c5c438350e0))
- bump actions/checkout from 4 to 7 ([`1154ff1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1154ff1422fd14f1a05d96248a9d4331ffd4e629))
- update test report [skip ci] ([`64845f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64845f977118e7e16040070efebc0f96b327add5))
- update CHANGELOG.md [skip ci] ([`f96406e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f96406e0fae8e405dd346e296cabfe9e4b7e727f))
- update test report [skip ci] ([`9f7c6f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f7c6f2ded07403a92539892697c34039933860e))
- update CHANGELOG.md [skip ci] ([`f2b4a11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2b4a1192999eedaf240bd59e462ed711e080f33))
- update test report [skip ci] ([`07b3802`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/07b380240a8a6b25b94a010045ba4ed096251e4e))
- update CHANGELOG.md [skip ci] ([`571ffc5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/571ffc51893dca1e41aa200231d994f9b58c755b))
- update CHANGELOG.md [skip ci] ([`b633663`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b63366344e35f19159119a1263dafadbd2422c83))

### Changes

- Bump version: 2.1.2 → 2.2.0 ([`e525ad2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e525ad2bd2bcbc3f7bb64adc221ef19daaa1ee4d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`0ce576e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ce576edba78205116954d5bd764fe497d685296))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e399d4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e399d4a4eafc1e8c59abf10f5ffa7860fb5481f1))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`522ae47`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/522ae4741923a34120a88ba0784e35d83cf1d818))
- Merge pull request #3 from dstroy0/dependabot/github_actions/actions/checkout-7 ([`e12efa8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e12efa8c9a45e5278910140fdd6c106aca27f675))
- codeql workflow ([`fc43f63`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc43f63f38541d22ea2a5081221609614226a80c))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ff7a9e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ff7a9e9def10229bab270420d81fd744fdcecd29))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`059af85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/059af85e7289728f892d8eafdf5b19d26bfc3de9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`debec1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/debec1dbbd49b4fce7d0344bae167a9490d52731))

### Documentation

- fill in CodeQL findings (no security issues) ([`5ab3515`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ab35156a888782c07ff5c880d3b83bef9cceebd))

### Features

- source-IP allowlist (accept-time firewall) ([`077e3d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/077e3d061e78103dcf8f315c39fb1f8d52e280e2))
- mDNS/Bonjour TXT records and extra service advertisement ([`4c1a23d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c1a23d37f12faa60c8494dcc8480111d7e5b623))
- MAC-derived device UUID (RFC 4122 v5) ([`c9020f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9020f4d89c804f9e5645cce13fbeaf4ab373eaf))
- Cache-Control header for static files ([`d2247dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2247dd8d050bf1afc920579b0a34f80efec31ee))

### Refactor

- SSH scratch tenants + ROADMAP/KNOWN_LIMITATIONS split ([`6d7eedb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d7eedb21214040c7a6d4f6b080af8f28a669f0c))

</details>

## [2.1.2] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.2 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`3b594ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b594edd17eae90ed9fdcfd773708925669d18e5))
- update CHANGELOG.md [skip ci] ([`e96c884`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e96c8845eda7e6b3e893ed98853c5a2014f17fdf))
- update CHANGELOG.md [skip ci] ([`d35218a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d35218a704b28185f92bcd27ed129826a814986c))

### Changes

- Bump version: 2.1.1 → 2.1.2 ([`8e52f4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e52f4b9c40c0f00291b233003248dc9675ee09e))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f831485`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f831485243c8c2567015ef2244ece510f7504f59))

### Features

- scratch pool, permessage-deflate, time-source + config-store services ([`e01c07c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e01c07c31da72cd5d5b8d7bcda22ad4e42b630bb))

</details>

## [2.1.1] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.1 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`017478b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/017478b88121e1cb80427220254abb8c856c7af5))
- update CHANGELOG.md [skip ci] ([`c558f86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c558f86ab7f71957a1eeafe245b543a2efb661d6))
- update CHANGELOG.md [skip ci] ([`c4768db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4768db7145a17e156491d89c067a206abfd2c39))

### Changes

- Bump version: 2.1.0 → 2.1.1 ([`0a82616`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a82616c46834fde553f52008d6882ca8eaa9a26))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`cf77381`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf77381344ac6c034ce518599cc113160542889b))

### Refactor

- consolidate generated assets into one unit; template stats() ([`1727f80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1727f8045ec7a998c3b5f9efe190f101e6485acf))

</details>

## [2.1.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`a06e0ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a06e0acbad7160dcf99ba20950899cc39e04c128))
- update CHANGELOG.md [skip ci] ([`64a8328`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64a8328aa86ae5e2ab4ca45c21f2a7f79cedac07))
- update test report [skip ci] ([`0a81789`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a81789452ac4234eba8e3ca84670e692964e544))
- update CHANGELOG.md [skip ci] ([`7ab520b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ab520b74fa3188aed9b9d9380acabdf85c35038))
- update test report [skip ci] ([`caab45b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/caab45b421a377b6ce0084320367f7de625a14dc))
- update CHANGELOG.md [skip ci] ([`5e1c641`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e1c641641678864cabd632f0224b86aff57e1a1))
- update test report [skip ci] ([`60b3a0a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60b3a0ad29ac2b322b4a9497aae7315f57edabe6))
- update CHANGELOG.md [skip ci] ([`fbadd3d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fbadd3dd46aacda1862b3246893269e0913caa81))
- update test report [skip ci] ([`d81d0e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d81d0e19bfa305de9b1c567a937e46d579ea55c0))
- update CHANGELOG.md [skip ci] ([`da969e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/da969e0d26f73bd1c403b14bcd30dedb79fdc169))
- update test report [skip ci] ([`8dabf46`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8dabf46c081ab4bbe846f59620ed3f2fbb99de43))
- update CHANGELOG.md [skip ci] ([`d99d2b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d99d2b97c5ae33e69268eca58dbdc35ac60c277e))
- update test report [skip ci] ([`64b7d3a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64b7d3afc0147a3fb4248a600f26cd2846909582))
- update CHANGELOG.md [skip ci] ([`96631a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96631a1c3a9fc22de79d22b6eea0b6054d2ff085))
- update test report [skip ci] ([`988e980`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/988e980173dbc204a3fa34af14c425b5426af315))
- update CHANGELOG.md [skip ci] ([`266d4dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/266d4ddc5bdcb9d25954c2f7cc27cde6a1d03f59))
- update test report [skip ci] ([`96b22e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96b22e1d07ae1a201e1add7faf668eed4782f0bf))
- update CHANGELOG.md [skip ci] ([`f2478b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2478b2d0413564fe57b0e3c3840614e6a668a4b))
- update test report [skip ci] ([`01f8a40`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01f8a40a121f1be22dbc2cf32011b8ada69f3234))
- update CHANGELOG.md [skip ci] ([`45b7a55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45b7a55801ff3afa1798f0bdaf742bf322840087))
- update test report [skip ci] ([`2342b47`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2342b475bbf0e35fdcbd35eda6ab5d275583f11e))
- update CHANGELOG.md [skip ci] ([`401716a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/401716a55530d86c4738aebcd404526ce6180401))
- update test report [skip ci] ([`33610dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33610dd20f2adecf4c4c93bdd1bd368e4adf92aa))
- update CHANGELOG.md [skip ci] ([`46480dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46480dcd01f64dcdea79e7d56e2edeeffe6f7ebc))
- update test report [skip ci] ([`3eff235`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3eff235f193ca20559adf87d12feff2196c3830e))
- update CHANGELOG.md [skip ci] ([`de2a3f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de2a3f1f9e37d72bcb03b31e0f77cdafea186b5e))
- update test report [skip ci] ([`50583ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50583ed86388381a88c0e6af562c56806a0967ab))
- update CHANGELOG.md [skip ci] ([`7d7ea83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d7ea83adfa2d0d7cf437530cd9a842b1d5dcb06))

### Changes

- Bump version: 2.0.0 → 2.1.0 ([`0f8289c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f8289c0d0a8d9c00c2911308b1bb4b62a86eec9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`716f59c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/716f59cdfe86932ab6bc9406096493d90241205d))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`cacd0a8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cacd0a8dec596e3282b989309e38ed6f4099e0ed))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`bf82439`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf8243923350fced29314724fdbf211346ea18d4))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`4e7e870`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e7e87091d4fb52d7d64c79f879bbfc6c0d61e15))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ca360f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ca360f8354fee579f1b26cf88286b58768d07056))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`d7bcc90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7bcc900c43237a2c4b3eadb12cbeb6c416d0455))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`1b691fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b691fdf0248ccbfa23a3f153e61a770bb5682ce))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`770c011`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/770c011376e31bbb222ac3c076a6b18cea062428))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7290bff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7290bff7052ecd6b39a16eda6eae1b4a552aa997))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`9fb00f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fb00f6ca195e3d02c7feb97b37ee966755398a1))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ba88aad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba88aad8758d3e6519d7230b5f427f82e13bd9a7))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c60f4b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c60f4b9d43ead45ee5c9b73e304bf1322e4ef36f))

### Documentation

- format ([`a8c2c55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8c2c55a1a669024f5f0d9589f40ea21f68bd1fd))
- smile, 9-way eyes, swim rotation, live-rock house, sticky timeout ([`4f7c5e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f7c5e991190edd7ccb9aa415eba58b4b0a3b858))

### Features

- TLS session resumption - RFC 5077 session tickets ([`fd0c350`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd0c35008ad71e5f583a265ea4a8c7e7576f2ec5))
- Modbus TCP slave - Modbus Application Protocol on TCP/502 ([`6d732c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d732c006aec173b64a531837da291cacfb46982))
- WebDAV server - RFC 4918 (class 1 + advisory locks) ([`5b09b48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b09b485f7061aaa13f3862907a04afe61105dd5))
- per-IP accept-rate throttle (connection-flood defense) ([`d5364d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5364d0afd2731efeadd306688d16193e9b732d1))
- CoAP block-wise transfer - RFC 7959 (Block1/Block2) ([`191090f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/191090f56b3e7aa62b0cbbb9c8c27a4fb5ae733b))
- CoAP resource observation - RFC 7641 (Observe) ([`d8ed477`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d8ed47796f94352e596d8c2336e9316b7814c58c))
- outbound SNMP notifications - traps and informs (v2c + v3) ([`98ce997`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/98ce9979f42a012258e2ee696cf558fd98dac731))
- outbound WebSocket client + wss (RFC 6455) ([`a0d0b57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d0b57b4ac8b4da55140fc85c22e43faec23426))
- MQTT 3.1.1 client + MQTTS, full QoS 0/1/2 ([`4c8191d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c8191dd92ddb52e59883c86b89e8c28c3f5901f))
- pluggable protocol dispatch, flow control, and outbound TLS auth ([`0417d04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0417d04af1ab7d134f3086805576e1b24cd5d4a1))
- add 10 optional subsystems, harden transport, regroup examples ([`9028bfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9028bfa1481df7f69cd865660091e1807fab67dc))
- add 10 optional subsystems, harden transport, regroup examples ([`cfa9ac4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cfa9ac4dfcc1f2590c2ec7153bd456781b99ed48))

### Refactor

- externalize served strings to src/web with a deterministic asset generator ([`7be3e55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7be3e550fadc54a642d8dad2bc8ef96898e92c03))

</details>

## [2.0.0] - 2026-06-24

<details>
<summary><b>Show Changelog for version 2.0.0 - 2026-06-24</b></summary>

### CI / Build

- update test report [skip ci] ([`b11f1f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b11f1f3a1ba0a45acdb0843427ca23e46dcfffa1))
- update CHANGELOG.md [skip ci] ([`e60eb61`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e60eb612a7aa3dbed15785a99d29cca2609a47b2))
- update test report [skip ci] ([`836608c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/836608c5565f4f429ac84f3b859879b032da264b))
- update CHANGELOG.md [skip ci] ([`04d2147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04d21470f09e199708fd94ea8c15822d04530721))
- update CHANGELOG.md [skip ci] ([`1a0f678`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a0f6782232ff2807383e36f2f25ef23e0566812))

### Changes

- Bump version: 1.2.7 → 2.0.0 ([`ab9f558`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab9f55868206d3fa90e2acfe21f90d1dc68fe16b))
- formatting ([`9a67e81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a67e81df024b4d365deaf6ae5d7079f2c0a7400))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`be35eb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be35eb2bd4579752bb7bd346894482ae153e1883))

### Refactor

- transport-layer I/O API + Telnet server; reorganize examples; docs ([`d34d51a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d34d51adb754b206039e68e3d6b286d88edc6e10))

</details>

## [1.2.7] - 2026-06-24

<details>
<summary><b>Show Changelog for version 1.2.7 - 2026-06-24</b></summary>

### CI / Build

- update test report [skip ci] ([`42b0fd7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42b0fd7331cff5a81abc589b191e895549e10e92))
- update CHANGELOG.md [skip ci] ([`c718cf3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c718cf32b1d18b41f42abf7741d676e92a41e83c))
- update test report [skip ci] ([`2c56ddc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c56ddcd0eb59a0d5af3078a153d36dd9e05ad65))
- update CHANGELOG.md [skip ci] ([`e40c020`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e40c0203edfdb7be4c4ec6f7a88f188aa0453d90))
- bump streetsidesoftware/cspell-action from 6 to 8 ([`9b3e289`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b3e2898c3e76359af1444dc2e2fb001ce26f7d5))
- update test report [skip ci] ([`835e317`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/835e3175a9f64b760581a74a1fe63e462732c84f))
- update CHANGELOG.md [skip ci] ([`46368f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46368f37030ba94efc90e308cd4a9062b77ee6c4))
- update CHANGELOG.md [skip ci] ([`1d7c16c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d7c16c4407dc75c1f00fc45cfcd12590bd3df59))

### Changes

- Bump version: 1.2.6 → 1.2.7 ([`782a0bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/782a0bb82da9951bf641db1b290246945322ca36))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`0e4abe5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e4abe551515956485d6dd828019e67d56d1753d))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`fa1af6d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa1af6dd48cb7fb5b0907f2830ba2227fb326ba5))
- Merge pull request #2 from dstroy0/dependabot/github_actions/streetsidesoftware/cspell-action-8 ([`41f78d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/41f78d18cd2f10caa4bbd5a89230bb85dc674b06))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`5c9e89c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c9e89c149e1c302997b35934cd5345ba8c815f7))

### Features

- SNMP v1/v2c/v3 agent (USM authPriv) on the BER codec ([`1477bbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1477bbdf6291ea37b0bac49be77b4b361945086e))
- HTTPS, web terminal, JSON helper, regex/interface routes, SNMP codec ([`69a864d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/69a864d4747872415a7b6b8ee3a99cf9481e87b2))

</details>

## [1.2.6] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.6 - 2026-06-23</b></summary>

### CI / Build

- update test report [skip ci] ([`666146e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/666146e250c43c883a2c00a9f706250f1ea610b1))
- update CHANGELOG.md [skip ci] ([`f873e5e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f873e5e8ac9d9f78cec57f73984c2e1d192df371))
- update CHANGELOG.md [skip ci] ([`d39f711`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d39f7110c93369c8315eb39b64b8b19553956a3d))

### Changes

- Bump version: 1.2.5 → 1.2.6 ([`2ae9c27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ae9c2749d7d8589ab19df02ee775b7ae8ca228d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`a0d83bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d83bd8386649367102d10e02c7d92bd5f0ceda))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7b630b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b630b4278b0ade11bf92a4fb613b19e886d9956))

### Features

- path params, Digest auth, templating, middleware, and chunked responses ([`24adb9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24adb9df73d30c95f9b2d163014eece1c9131d9b))

</details>

## [1.2.5] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.5 - 2026-06-23</b></summary>

### CI / Build

- update test report [skip ci] ([`ee7322b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee7322b0b21d6924c7e30d9e33dd8cd9bd8b9271))
- update CHANGELOG.md [skip ci] ([`b68d7e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b68d7e1ac901fe4f9596913a5749075b32594d6f))
- update test report [skip ci] ([`075d4eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/075d4eb03e1a5ec58df3941181cbe706a4175d0b))
- update CHANGELOG.md [skip ci] ([`2ee4a51`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ee4a518f3c91adede0d3e7e77fd0afccd0787ac))
- update test report [skip ci] ([`89c466b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89c466b7f2c2678fbd6b303afe1f29e4f7f06a0b))
- update CHANGELOG.md [skip ci] ([`cc64a23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cc64a234509ff4223a39b58425a4c6045a4cef3d))
- update test report [skip ci] ([`5a683e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a683e4314a21f818d6067b6a445212de4e8f572))
- update CHANGELOG.md [skip ci] ([`84c9c1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c9c1dc8855b721bd4a3cd3602e87d213562492))
- update CHANGELOG.md [skip ci] ([`572b3e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/572b3e0c461df3426dff20feb41d2e1ec6de6cb9))

### Changes

- Bump version: 1.2.4 → 1.2.5 ([`dfbcf71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfbcf7138c24edd7a5f96625b7994759d9c5e868))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c1ec861`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c1ec861b4aa06bd56ff5e15d3c0475b31974cce4))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f6557de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6557de195179e5f421ff62efa7820bcb5259385))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`d657132`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d65713242f3e2c8d339f27bdab765ce9cace0bc6))
- create squirty the injection squid; update docs ([`14d4ef3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14d4ef31d6cdf724f17f6f2e537a9d6a4292f85b))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`65fa7f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65fa7f48d8e0dee67dccb4a6c6ff9b6f5c0485c8))
- update docs ([`7a5b461`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a5b4618c003aeeba5353f25e482fb6b54cbd865))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ea49289`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ea492898c6607d3d957b8ff30706bba37b9c31e4))

### Documentation

- docs formatting ([`9f10d21`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f10d211b21506d943790484d0c7ab3247bd6394))

### Features

- custom response headers/cookies + urlencoded form params ([`012829b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/012829b53fcfe261e836b3a9aa9d9496b583dc57))

</details>

## [1.2.4] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.4 - 2026-06-23</b></summary>

### Bug Fixes

- fix ci errors ([`8269a66`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8269a668527694c0801c938bc660498202ec390b))

### CI / Build

- update test report [skip ci] ([`4c39fa5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c39fa58bb5f8766c14de5c63c2463165c82662f))
- update CHANGELOG.md [skip ci] ([`7cc3427`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cc34279b71de4f5cb8dff8f61d480b032341a49))
- update test report [skip ci] ([`6ccd273`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ccd2731089745ec3d533241f9dad8870bd7da80))
- update CHANGELOG.md [skip ci] ([`535d844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/535d844998b6b895212a13ceff058410c2bcff81))
- update test report [skip ci] ([`a031108`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a031108933cfc6c4ff9c041a876c86c5660d9d09))
- update CHANGELOG.md [skip ci] ([`252fee7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/252fee7c65779202acbfff5d019670f22ab9a6e9))
- update CHANGELOG.md [skip ci] ([`152a08d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/152a08df007aaf97722f0cfca676414e98cb3dc8))

### Changes

- Bump version: 1.2.3 → 1.2.4 ([`0344053`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0344053b3894e0e59643671f831e396969a82410))
- update readme ([`cbd487d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cbd487dfa3554cfef7312e4f73221d24eb860ca9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`08462d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08462d02acaeb647f61d8678943a5f79e12a39f1))
- update todo ([`854d297`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/854d2972c63efa2979b0f5c9f144e0a18e903040))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7c64a3e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c64a3e50660aad1b3865d24018dfefd9292a88f))

</details>

## [1.2.3] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.3 - 2026-06-23</b></summary>

### Bug Fixes

- fix concurrency issue in test-report ([`7e1b38c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e1b38c9e95a4594a1bc156efed63a09ee3732fe))

### CI / Build

- update test report [skip ci] ([`c5733da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c5733da34beba9c03dd6ba92e148e48b6b98d175))
- update CHANGELOG.md [skip ci] ([`ef0de34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef0de3447f92650254f63312ac9fd386799b5795))
- update test report [skip ci] ([`ac6c36f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac6c36f39afc4c1be11de612e48e6b0700807da4))
- update CHANGELOG.md [skip ci] ([`996488a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/996488a133851861e9d61d4fa983b27ca169c25e))
- update test report [skip ci] ([`7f67f58`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f67f58da64c63393d0ddc86bc35153ee03d46a5))
- update CHANGELOG.md [skip ci] ([`d10a5e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d10a5e8794b08e860237c987a893fcc1edf53da5))
- update test report [skip ci] ([`0e872ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e872ef5719e3a06adf7c57f79eed84ab125c80a))
- update CHANGELOG.md [skip ci] ([`b8ecc21`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8ecc214ecb3d6da003c17f9b396dcd32bfc7cd7))
- update test report [skip ci] ([`d3bf700`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3bf7008af6e23784eb305e79d048af7bd5bfdfa))
- update CHANGELOG.md [skip ci] ([`e30b48d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e30b48d050fe6c44504fb30d639f0d7629a242a4))
- update test report [skip ci] ([`150f8bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/150f8bcf8a053230f6a0c4705bc07ef1e6d18c91))
- update CHANGELOG.md [skip ci] ([`c20569a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c20569aec8ce215ba2e098b9337246126b79908b))
- update test report [skip ci] ([`1f2b41c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f2b41c8954d206cdabaf1515d8de3aa9be0c863))
- update CHANGELOG.md [skip ci] ([`ad53276`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad532769d8e980c83327d752194905a283bd81b8))
- update test report [skip ci] ([`ef9facc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef9facc435b7caf534bb81e52b2bb938c8044c85))
- update CHANGELOG.md [skip ci] ([`206abe1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/206abe1a8f6a823af00b0aa93b7fb045dbe37b4f))
- update test report [skip ci] ([`b260304`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b2603045df12eeb2b14866398138f91a4f752fc3))
- update CHANGELOG.md [skip ci] ([`6863c57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6863c579e07218ebe2e431ee9bbfe814ac59efd4))
- update test report [skip ci] ([`604cef3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/604cef3d8cd273916e04ed44ef95b94b1d9c96ee))
- update CHANGELOG.md [skip ci] ([`46d618d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46d618d06f4e571b542a965e78a84d984940b0bf))
- bump patch version to 1.2.1 and update README ([`68d7375`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68d73753852f61a8242bd1fcb795ce6d83ba19fc))
- update test report [skip ci] ([`8598435`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8598435a68f4ca6765cfc75f86a3528f766bef13))
- update CHANGELOG.md [skip ci] ([`4a2cd14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a2cd14db09d426f7d71588fc6f790bd0f14df91))
- update test report [skip ci] ([`d356856`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3568563a7de39f27cd92ab9597a03e96116cf78))
- update CHANGELOG.md [skip ci] ([`b01f7b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b01f7b20b3cad3f3eab4dab2909b6e3fd2d3f498))
- update test report [skip ci] ([`eb23a2d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb23a2dceba5a93964791ecfc23b81166996f650))
- update CHANGELOG.md [skip ci] ([`63784f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/63784f641ea0e54c4c6db7d4d0702307e3648225))
- update test report [skip ci] ([`4a536dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a536dcbc54c8928f0daed6efdae7a12a376e620))
- update CHANGELOG.md [skip ci] ([`ce8e5fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ce8e5facad2a2cbaa679799b99fd2f422211acbc))
- update test report [skip ci] ([`82b13cb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82b13cbdb044cd1a732d14b24acde31bb9f9c69a))
- update CHANGELOG.md [skip ci] ([`92583c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92583c6ec6071110b5dd110ceac214ab26759f2e))
- update test report [skip ci] ([`0e20e9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e20e9c56db25b77d76ea34505d7f14a589f3dd8))
- update CHANGELOG.md [skip ci] ([`dfde0b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfde0b9f5369e1903071ee2806123fa646bfa948))
- update test report [skip ci] ([`ded48b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ded48b99ed5d44b46bd27c93b4e23b4cc6e95a86))
- update CHANGELOG.md [skip ci] ([`a74c71d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a74c71d3c086e15abaa57ca6d29f6cec5da63717))
- update test report [skip ci] ([`0084249`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00842490bae66c1eefb095445449af3a5e64eee6))
- update CHANGELOG.md [skip ci] ([`b5d7e33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5d7e330c022a097728917d9bada68548a276a62))
- update CHANGELOG.md [skip ci] ([`f306e1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f306e1ca909e9abd55db08632739509824d7a393))
- update CHANGELOG.md [skip ci] ([`66e3d57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/66e3d57ce55bf678e5665bfc0e674a9af9bb0ccf))
- update CHANGELOG.md [skip ci] ([`484a46c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/484a46cfdda3148c6ade4ad02769b1132f9afb24))
- update CHANGELOG.md [skip ci] ([`655130c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/655130cc105c49bc2c724d1ced92c84483d453a9))
- update CHANGELOG.md [skip ci] ([`6e8beaa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e8beaa140305d190bb46b884bf533c5155d6b52))
- update CHANGELOG.md [skip ci] ([`3a55a15`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a55a1556faeaf96f31a400f5fff01012efdcf4e))
- update CHANGELOG.md [skip ci] ([`01d1e1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01d1e1acbfc53b79de30074d6bc2bae6b4cb33ec))
- update CHANGELOG.md [skip ci] ([`a4d0a84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4d0a849a95bc369ee567199f833fb50e34bd7e9))

### Changes

- Bump version: 1.2.2 → 1.2.3 ([`5225d85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5225d85724d3dba79dde8bb2b2b091366bba33c5))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`3f3391b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f3391b8329c04c3bc9a3174965b09a89bb75282))
- update initial search token ([`fba70f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fba70f924c0673c11bd52072276b6a9ab49260d2))
- Bump version: 1.2.1 → 1.2.2 ([`1cf6787`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1cf6787927782e495ce27ff4e0c8d0962a18efbc))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e4d2614`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4d26140b11b01c8e13ce8df1bb88d904be35c51))
- update bumpversion to use GPG ([`72686e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72686e86c1ee7f23318575eb7906ace657702679))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`b8335ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8335ec55cf53f07afd24b6cd85577d3d8d095cf))
- add version badge to docs ([`7ecebd1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ecebd1d404c2bbe975d71d6f2760642b77b7eb8))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`aaa6b68`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aaa6b68f47a5595214dd529ae3b7d83c15be8bcd))
- update initial search token ([`668f56e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/668f56e629e39a763991a29d86d461550c08d7eb))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f3a4a1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3a4a1c75084dc97a8f8c79d6e8ba1235eb5ed19))
- update initial search token ([`8a02843`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a0284336640b3d194b2058530fd86b514b1668e))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`a65c596`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a65c5963d8bbf02dea7289c54e57a25647292ba0))
- update readme; add qol bumpversion ([`6062d0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6062d0ba13d182fcbc1376a1636cd0382089ba5f))
- fix markdown formatting style check issues ([`d0399c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0399c23ca11bb2576ec1fdb6e484118be16a06a))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`566095c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/566095cc8065a74a9afe6b4e258628d1d15ea717))
- formatting ([`216902c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/216902c416b2bed37e5465073cdb05129ce35c2e))
-   - Add optional system services: ([`5b3c5f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b3c5f2bb500d4d4eafbafb1650c4292514a558d))
- add toc ([`9f165c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f165c53fedb4e40fddd532b4aaf2797010fe847))
- update md formatting; add collapsible sections ([`0b77665`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b776659877417ad63f6eaf141918194480b8201))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`89dcb6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89dcb6ee2fdab0adff07f5c954f2322059d4b612))
- update TEST_REPORT.md formatting ([`8f6891c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f6891cd5a47d54023b39cc1d7983fde7d8f1b49))
- add md format linter workflow ([`06e965b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06e965b166e5ceccbf5677aa66c9e8eef8732a8f))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c653ca9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c653ca91f0deab888bef98f8f3291b20cfa8b34e))
- update clang-format workflow to use v22 ([`6654bfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6654bfa5d0805d2a480c28f33c4dd33d28ab620b))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f833120`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f833120b824fda7e9c1ea622b786b7f7f7ef0549))
- stop tracking .vscode/ ([`b618929`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b61892970a93f631aac292731c55d4e589b61dd6))
- format ([`290f745`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/290f74509db99120b5f6febbb4cbb175aab72f8f))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c4de884`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4de884402685b2aea7a5711926d832a8ab90868))
- format codebase; check todo list for the current state of the library before the version bump finalizations ([`2cc3601`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2cc3601a965cacb7036f1eafd80be5bfc7ff10dc))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e04a03b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e04a03b8d0a1feb524cd7ab8d9119066df54a628))
- update examples; verify crypto logic by hand; second pass optimization complete; begin TODO feature implementation, then add new bugs to TODO, then squash bugs before third pass optimization ([`4e82975`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e829757713b3d893804d5ef917db5e269cbff75))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c214285`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c21428503d9f8f0e4e3acd035b2dc9fe598587ff))
- patch ([`e689c81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e689c81d911c0be76c1e85c989d178aff3f2ecd1))
- add support for Telnet and partially implement SSH; add port listener abstraction layer; add more hw crypto; update test suite to account for new functionality; reorganize network_drivers/, it has subfolders for all OSI layers, functionally grouped by layer; lint codebase; spellcheck codebase; move test results to test/TEST_REPORTS.md; create test/TEST_DOCUMENTATION.md and copy all test documentation to it;, link to test/TEST_DOCUMENTATION.md and test/TEST_REPORTS.md in the README.md; create RFC.md; move RFC info from README.md; remove RFC info from the README.md; link to RFC.md in README.md; ([`75ab65e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75ab65ef84321dbc824b25e1a4f240f5ad56f1f2))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c8c043c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8c043c319e8c2967e3939d607aff6056e51eee9))

### Documentation

- move detailed README body to docs/README.md ([`8eb8de2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8eb8de2105b1590bff3736a09dbd13ff54b81330))
- add GitHub Actions workflow status badges to README.md ([`346c0cf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/346c0cf9362244e711d945a01abc0ba9ddee5203))
- update feature flags info & add provisioning, MDNS & NTP services ([`b4135ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4135ba256608624ac5037f5f9e694775f4dca20))
- docs ([`93c7ef8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93c7ef88637ee6576bef7a6ba34a358881ff42a3))

### Refactor

- centralize HTML templates into DETWS_HTML.h ([`72f5d1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72f5d1f8b83fbfd2b2d6593ba76a9fa391964f8b))

</details>

## [1.2.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 1.2.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`388646e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/388646e7d98a3d23f6e0f959ea03b87e2234bd4d))
- update CHANGELOG.md [skip ci] ([`399b835`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/399b835a8378ae3d5c5079a2078ae9f2c152570a))

### Changes

- add features; bump version ([`a0d5a67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d5a67dbeaed2cfa832d8b7d7f9b80f0494dcc1))
- implement websockets, sse, auth per-route, multipart form parsing, hw SHA-1 (mbedtls/sha1), user selectable features/configuration (e.g. ws, sse, etc.) via flag and compile-time constants. ([`d3c8ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3c8ec97acf3ec66bcbe103128e944fcde1ab2ed))
- update version to 1.1.0, add author and maintainer info, and update description in library.properties ([`4da247c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4da247cea4209809b1d939dd85318f7539192f77))

</details>

## [1.1.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 1.1.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`bb32fdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb32fdfba90eb4aeb4da4fd6130bae0ebc414813))

### Changes

- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`86988ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86988ca73c51b903d15034deccf533e4c504e8a0))
- format codebase using .clang-format, lint and add more examples, lint docs ([`12baf8d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12baf8d5ce6976ceef0a65b2309702dfd1babf89))

</details>

## [0.1.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 0.1.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`3db5a62`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3db5a6212dafab7c44dd6a68915aaa33e957f0f3))
- update CHANGELOG.md [skip ci] ([`3ddf27c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ddf27c9362bf869f0216afc19874d141fbb0e24))
- update CHANGELOG.md [skip ci] ([`eeea908`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eeea9082c24ae05b8898ed371cef5dbffb0386db))
- update CHANGELOG.md [skip ci] ([`e81b25f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e81b25ff412562043e086ffc315852f16ce3f180))
- update CHANGELOG.md [skip ci] ([`1a4c39b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a4c39be56b8ba74da376b60f4b45bbb6ac57f07))
- update CHANGELOG.md [skip ci] ([`f04ac84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f04ac84bdb7e675a6d2b255a87372bfa91ffd048))
- ci initial commit ([`939e034`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/939e0349f40368dc9a7b580f00d8424b24a10258))

### Changes

- update test suite, adjust logic for RFC compliance ([`689b240`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/689b24052bbbf8ba1905697b845e55af1de07c1d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`73d5f25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73d5f2525e6756cdb9762b8477c43d36fcdb0215))
- add dependabot github-actions dependency auto-updater ([`787158f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/787158f7214963b54bf77cb7f93f78315764b9d2))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`99a92ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99a92ee1827c72ce691801a1d52612ffdb3b5c71))
- update ci action versions ([`14748bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14748bfcb83c501975b5e6b8f50559e0e8f66513))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`16039a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16039a9384c580cf02392e25862f10cd45ac5664))
- add examples; update README ([`ecfeb0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ecfeb0faa559fbc674671a41a57e2ed03e10ecf8))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`846720c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/846720cc8111f113b5af11931eb17d9a81ef1c11))
- update cliff.toml for this repo ([`1fb43ae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1fb43ae1229ec4391ec7c89a1b9d9747faf327ce))
- update CHANGELOG output dir ([`2838da2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2838da22b3673aef7f6b365a1e0ec2c3ebedd504))
- update CHANGELOG output dir ([`d799b4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d799b4f7c0031f5e539c9f3544127f283416f87e))
- initial commit ([`9b48742`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b48742e971a4e41924df8d67aab91f10c3346e8))

</details>
