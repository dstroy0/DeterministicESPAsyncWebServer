#!/usr/bin/env bash
# DTLS 1.3 real-peer interop: drive the library's dtls_conn server with wolfSSL's DTLS 1.3 client.
# Builds wolfSSL (with DTLS 1.3) on first run, generates a throwaway Ed25519 certificate, compiles
# the harness (test/servers/dtls_wolfssl/dtls_interop_server.cpp) against the library sources, then
# runs a full handshake + application-data round trip. Prints "INTEROP OK" on success.
#
# Linux only (needs a POSIX UDP socket + a native g++). Tested on Debian/aarch64 (Raspberry Pi).
#
#   usage: test/servers/dtls_wolfssl/run_interop.sh
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)" # repo root (contains src/)
WORK="${DTLS_INTEROP_WORK:-$HERE/.work}"
WOLF_VER="${WOLFSSL_VERSION:-v5.9.2-stable}"
PORT="${DTLS_INTEROP_PORT:-11150}"
mkdir -p "$WORK"

# --- 1. wolfSSL with DTLS 1.3 (release tarballs ship a pre-built configure, but only the source
#        tarball is a GitHub asset here, so build from the git tag; needs autoconf/automake/libtool) ---
CLIENT="$WORK/wolfssl/examples/client/client"
if [ ! -x "$CLIENT" ]; then
  echo ">> building wolfSSL $WOLF_VER (DTLS 1.3)"
  command -v autoconf >/dev/null || { echo "install autoconf automake libtool first"; exit 2; }
  rm -rf "$WORK/wolfssl"
  git clone --depth 1 -b "$WOLF_VER" https://github.com/wolfSSL/wolfssl.git "$WORK/wolfssl"
  ( cd "$WORK/wolfssl" && ./autogen.sh && \
    ./configure --enable-dtls --enable-dtls13 --enable-ed25519 --enable-curve25519 \
                --enable-aesgcm --disable-shared && make -j"$(nproc)" )
fi

# --- 2. throwaway Ed25519 leaf certificate + its raw 32-byte seed ---
if [ ! -f "$WORK/cert.der" ]; then
  echo ">> generating Ed25519 certificate"
  openssl genpkey -algorithm ed25519 -out "$WORK/ed.key" 2>/dev/null
  openssl pkey -in "$WORK/ed.key" -outform DER 2>/dev/null | tail -c 32 > "$WORK/seed.bin" # PKCS#8 tail = raw seed
  openssl req -x509 -new -key "$WORK/ed.key" -out "$WORK/cert.pem" -days 3650 -subj '/CN=dtls-interop' 2>/dev/null
  openssl x509 -in "$WORK/cert.pem" -outform DER -out "$WORK/cert.der" 2>/dev/null
fi

# --- 3. compile the harness against the library sources (DETWS_ENABLE_DTLS) ---
echo ">> compiling harness"
D="$ROOT/src/network_drivers/presentation"
g++ -O2 -std=gnu++17 -DDETWS_ENABLE_DTLS=1 -I"$ROOT/src" "$HERE/dtls_interop_server.cpp" \
  "$D/dtls/dtls_conn.cpp" "$D/dtls/dtls_record.cpp" "$D/dtls/dtls_handshake.cpp" \
  "$D/http3/tls13_msg.cpp" "$D/http3/tls13_kdf.cpp" "$D/http3/quic_hkdf.cpp" "$D/http3/quic_aead.cpp" \
  "$D/ssh/crypto/ssh_sha256.cpp" "$D/ssh/crypto/ssh_hmac_sha256.cpp" "$D/ssh/crypto/ssh_sha512.cpp" \
  "$D/ssh/crypto/ssh_curve25519.cpp" "$D/ssh/crypto/ssh_ed25519.cpp" -o "$WORK/harness"

# --- 4. run the harness + the wolfSSL DTLS 1.3 client (-u DTLS/UDP, -v 4 DTLS 1.3, -t X25519,
#        -d skip cert-chain verification of the throwaway cert) ---
echo ">> running interop on udp/$PORT"
"$WORK/harness" "$PORT" "$WORK/cert.der" "$WORK/seed.bin" >"$WORK/harness.log" 2>&1 &
HPID=$!
sleep 1
# Run the client from the wolfSSL dir so its default ./certs/ resolves (it still skips peer checks).
( cd "$WORK/wolfssl" && echo 'hello wolfssl!' |
  timeout 10 ./examples/client/client -u -v 4 -d -t -h 127.0.0.1 -p "$PORT" ) >"$WORK/client.log" 2>&1 || true
kill "$HPID" 2>/dev/null || true

echo "--- harness log ---"
grep -E 'HANDSHAKE|APPDATA|INTEROP|FAIL' "$WORK/harness.log" || true
grep -q 'INTEROP OK' "$WORK/harness.log" && { echo ">> PASS"; exit 0; } || { echo ">> FAIL"; exit 1; }
