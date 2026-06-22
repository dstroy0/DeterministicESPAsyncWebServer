# SSH Support

DeterministicESPAsyncWebServer includes a **complete SSH-2.0 server protocol**:
identification-string exchange, algorithm negotiation, Diffie-Hellman key
exchange, user authentication (password **and** publickey), and the
connection/channel protocol - all built on constant-memory, side-channel-aware
primitives.

The message state machine is driven by a single transport-agnostic dispatcher
([`ssh_server.cpp`](../src/network_drivers/presentation/ssh/ssh_server.cpp)) that
consumes decrypted message payloads and emits responses through a callback, so
it is fully unit-testable off-target. The TCP glue
([`ssh_conn.cpp`](../src/network_drivers/presentation/ssh/ssh_conn.cpp)) binds a
`PROTO_SSH` connection to a session slot, pumps ring-buffer bytes through the
banner exchange and binary-packet layer, and writes responses back to the
socket.

> **Handshake:** banner exchange → `KEXINIT` negotiation → `KEXDH` → `NEWKEYS` →
> `ssh-userauth` (publickey or password) → `ssh-connection` session channel,
> with transparent in-session re-keys.

## Feature summary

- **DH-group14-SHA256 key exchange** (RFC 3526 + RFC 8268) - 2048-bit MODP, g=2, full exchange-hash + host-key signature
- **AES-256-CTR** encryption (RFC 4344) - hardware-accelerated on ESP32 via mbedTLS, software fallback for native tests
- **HMAC-SHA2-256** integrity (RFC 6668) - MAC-verify-before-use invariant
- **RSA-SHA2-256** host key (RFC 8332) - PKCS#1 v1.5 signing (real on ESP32; native build is a test stub, see below)
- **Publickey authentication** (RFC 4252 §7) - client RSA-SHA2-256 signatures are verified for real on both platforms; an application callback authorizes keys per user
- **Password authentication** (RFC 4252 §8) - credentials checked via an application callback; the password is wiped from the stack after every attempt. Compile out with `DETWS_SSH_ALLOW_PASSWORD=0` for publickey-only hardening
- **Session channel** (RFC 4254) - `shell`/`exec`/`pty-req` accepted; inbound channel data surfaced to the app as a raw byte stream, with RFC 4254 §5.2 window flow control
- **In-session re-keying** (RFC 4253 §9) - client- or server-initiated; the session id is fixed at the first exchange hash across re-keys
- **Static-only allocation** - all SSH state pre-allocated in BSS; no heap after `begin()` (except the one-per-connection mbedTLS RSA operation during KEX, freed immediately)
- **RSA private key never in static memory** - loaded from NVS → stack → sign → volatile-wipe; zero window for overflow-based key exfiltration
- **Sequence number overflow guard** - connection closed before 32-bit wrap to prevent CTR keystream reuse

## RFC / FIPS compliance

| Component                     | Standard                   | Status                                                                  |
| ----------------------------- | -------------------------- | ----------------------------------------------------------------------- |
| SHA-256                       | FIPS 180-4                 | Implemented (software both platforms; HW one-shot via mbedTLS on ESP32) |
| HMAC-SHA2-256                 | RFC 2104, RFC 6668         | Implemented; verify-before-use, constant-time MAC compare               |
| AES-256-CTR                   | FIPS 197, RFC 4344         | Implemented (HW via mbedTLS on ESP32; software AES on native)           |
| DH group14 (modexp)           | RFC 3526, RFC 8268         | Implemented (HW `mbedtls_mpi` on ESP32; software Montgomery on native)  |
| DH public-value validation    | RFC 4253 §8                | Implemented (rejects `e`/`f` outside `1 < v < p-1`)                     |
| Session key derivation        | RFC 4253 §7.2              | Implemented (six A–F keys from `K`, `H`, session_id)                    |
| Binary packet protocol        | RFC 4253 §6                | Implemented - framing, ≥4-byte padding to 16-byte multiple, encrypt-then-MAC over `seq‖plaintext`, CTR length-peek with snapshot/restore |
| RSA public-key blob           | RFC 4253 §6.6, RFC 8332 §3 | Implemented - blob type string is `ssh-rsa`                             |
| RSA-SHA2-256 signing          | RFC 8332, RFC 8017 §8.2    | **ESP32: real** (`mbedtls_pk_sign`). **Native: test stub** (`d=1`), never compiled into firmware |
| RSA-SHA2-256 verification     | RFC 8332, RFC 8017 §8.2    | **Real on both platforms** (small public exponent); validated against an openssl-generated KAT |
| Version / KEXINIT negotiation | RFC 4253 §4.2, §7.1        | Implemented - banner exchange + algorithm negotiation (one choice per category) |
| Exchange hash + NEWKEYS       | RFC 4253 §7.2, §8          | Implemented - H over `V_C,V_S,I_C,I_S,K_S,e,f,K`; keys activate on NEWKEYS |
| User authentication           | RFC 4252 §5, §7, §8        | Implemented - `publickey` and `password` methods                        |
| Connection / channel          | RFC 4254 §5, §6            | Implemented - single `session` channel, data stream, window flow control |
| Re-keying                     | RFC 4253 §9                | Implemented - packet-count threshold; session id preserved across re-keys |

## Authentication and hardening

Two methods are offered. The application installs callbacks:

```cpp
ssh_auth_set_pubkey_cb([](const char *user, const uint8_t *blob, size_t len) {
    return is_authorized_key(user, blob, len); // compare against your authorized_keys
});
ssh_auth_set_password_cb([](const char *user, const char *pass) {
    return check_password(user, pass);
});
```

For publickey, the server first answers the no-signature probe with
`USERAUTH_PK_OK`, then verifies the client's RSA-SHA2-256 signature over the
session id and request before granting access.

**Hardening:** define `DETWS_SSH_ALLOW_PASSWORD=0` to compile out password auth
entirely. The `password` method is then refused and is not advertised in the
`USERAUTH_FAILURE` method list; only `publickey` remains.

## Limitations

- One `session` channel per connection (no port-forwarding / X11).
- Native-build RSA *signing* is the `d=1` test stub above (ESP32 mbedTLS path is
  real). RSA *verification* is real on both platforms.

## Memory footprint

All numbers use default configuration values.

| Symbol                    | Type | Notes                                            |
| ------------------------- | ---- | ------------------------------------------------ |
| `ssh_pkt[MAX_SSH_CONNS]`  | BSS  | Packet state + RX reassembly buffers             |
| `ssh_keys[MAX_SSH_CONNS]` | BSS  | AES contexts + 4 × 32-byte MAC/IV keys           |
| `ssh_dh[MAX_SSH_CONNS]`   | BSS  | DH scalars y, f, K (wiped post-KEX) + H          |
| `ssh_sess[MAX_SSH_CONNS]` | BSS  | Handshake phase, V_C/I_C/I_S, session id         |
| `crypto_work[1536]`       | BSS  | Montgomery bignum scratch; zeroed after each use |
| `group14_p`, `group14_g`  | BSS  | RFC 3526 prime + generator (constants)           |
| `ssh_host_pubkey`         | BSS  | RSA-2048 public key (n + e); no secret material  |

**Key material is excluded by design.** AES-256 keys, HMAC keys, and the DH
shared secret K live in `ssh_keys[]` / `ssh_dh[]`. The RSA private key is never
in static memory - it exists only on the stack during `ssh_rsa_sign()` and is
volatile-wiped before the function returns.

The BSS symbols (`ssh_pkt`, `ssh_keys`, `ssh_dh`) are separate linker symbols, so
a linear buffer overflow from `ssh_pkt[i].rx_buf` cannot reach `ssh_keys` in a
single stride.

See [SECURITY.md](SECURITY.md) for the full security treatment.
