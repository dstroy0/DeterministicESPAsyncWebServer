# Changelog

All notable changes to DeterministicESPAsyncWebServer are documented here.

## [Unreleased]

### Highlights (hand-curated)

Security / correctness:
- Native RSA signing is now a real full-width `m^d mod n` (was a `d=1` test stub);
  validated by a sign→verify round-trip with a real 2048-bit key.
- SSH brute-force defense: failed `USERAUTH_REQUEST`s are bounded per connection
  (`SSH_MAX_AUTH_ATTEMPTS`) then `SSH_MSG_DISCONNECT`.
- Non-constant-time software crypto is now compile-excluded from firmware
  (`#ifndef ARDUINO` around the native Montgomery cluster).
- `base64_decode()` takes a `dst_cap` bound (no overflow) and rejects misplaced
  `=` padding / non-multiple-of-4 input.
- Opt-in global accept-rate throttle (`DETWS_ENABLE_ACCEPT_THROTTLE`).
- `SSH_MSG_UNIMPLEMENTED` reply for unknown messages (RFC 4253 §11.4).

ESP32 build / performance:
- `ssh_rsa.cpp` compiles on mbedtls v2 **and** v3 (`MBEDTLS_VERSION_MAJOR`
  guards). Fixed a latent bug where the host signature covered `H` instead of
  `SHA256(H)`.
- SSH streaming SHA-256 now uses the ESP32 hardware engine (mbedtls-backed
  context), accelerating per-packet HMAC + KEX; AES-256-CTR encrypts the whole
  buffer in one `mbedtls_aes_crypt_ctr()` call (was per-16-byte-block ECB).
  Software paths remain native-only. `examples/07.SSHCryptoSelfTest` validates
  the HW path on a device.

Features:
- `serve_static(prefix, fs, root)` - one-call static subtree mount with
  `index.html` fallback, MIME auto-detection, gzip-static (`<path>.gz`), and
  path-traversal rejection.
- `DetWebServer::mime_type()`, `redirect()`, and named `DetWebServerResult` codes
  for `begin()`/`listen()`/`restart()`.
- SSH: `ssh_conn_send()` outbound channel API + `examples/06.SSH`, plus host-key
  provisioning docs (`docs/SSH.md`).

### CI / Build
- update CHANGELOG.md [skip ci] ([`01d1e1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01d1e1acbfc53b79de30074d6bc2bae6b4cb33ec))
- update CHANGELOG.md [skip ci] ([`a4d0a84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4d0a849a95bc369ee567199f833fb50e34bd7e9))


### Changes
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c214285`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c21428503d9f8f0e4e3acd035b2dc9fe598587ff))
- patch ([`e689c81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e689c81d911c0be76c1e85c989d178aff3f2ecd1))
- add support for Telnet and partially implement SSH; add port listener abstraction layer; add more hw crypto; update test suite to account for new functionality; reorganize network_drivers/, it has subfolders for all OSI layers, functionally grouped by layer; lint codebase; spellcheck codebase; move test results to test/TEST_REPORTS.md; create test/TEST_DOCUMENTATION.md and copy all test documentation to it;, link to test/TEST_DOCUMENTATION.md and test/TEST_REPORTS.md in the README.md; create RFC.md; move RFC info from README.md; remove RFC info from the README.md; link to RFC.md in README.md; ([`75ab65e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75ab65ef84321dbc824b25e1a4f240f5ad56f1f2))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c8c043c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8c043c319e8c2967e3939d607aff6056e51eee9))


### Documentation
- docs ([`93c7ef8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93c7ef88637ee6576bef7a6ba34a358881ff42a3))


## [1.2.0] - 2026-06-20

### CI / Build
- update CHANGELOG.md [skip ci] ([`388646e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/388646e7d98a3d23f6e0f959ea03b87e2234bd4d))
- update CHANGELOG.md [skip ci] ([`399b835`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/399b835a8378ae3d5c5079a2078ae9f2c152570a))


### Changes
- add features; bump version ([`a0d5a67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d5a67dbeaed2cfa832d8b7d7f9b80f0494dcc1))
- implement websockets, sse, auth per-route, multipart form parsing, hw SHA-1 (mbedtls/sha1), user selectable features/configuration (e.g. ws, sse, etc.) via flag and compile-time constants. ([`d3c8ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3c8ec97acf3ec66bcbe103128e944fcde1ab2ed))
- update version to 1.1.0, add author and maintainer info, and update description in library.properties ([`4da247c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4da247cea4209809b1d939dd85318f7539192f77))


## [1.1.0] - 2026-06-20

### CI / Build
- update CHANGELOG.md [skip ci] ([`bb32fdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb32fdfba90eb4aeb4da4fd6130bae0ebc414813))


### Changes
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`86988ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86988ca73c51b903d15034deccf533e4c504e8a0))
- format codebase using .clang-format, lint and add more examples, lint docs ([`12baf8d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12baf8d5ce6976ceef0a65b2309702dfd1babf89))


## [0.1.0] - 2026-06-20

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



