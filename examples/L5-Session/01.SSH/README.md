# 01.SSH - a zero-heap SSH server

**Layer:** L5 Session · **Build flags:** `DETWS_ENABLE_SSH`

## What this example teaches

This stands up a real SSH 2.0 server (RFC 4253/4252/4254) on port 22: it loads
an RSA host key from NVS, authenticates clients by password and/or public key,
and echoes channel data back. The crypto runs on the ESP32 hardware accelerator
and no connection state touches the heap.

**Listening on a non-HTTP protocol.** The same `DetWebServer` can host other L5
protocols. You open an SSH listener with `server.listen(port, PROTO_SSH)` and
then `begin()` (no port argument needed when you have explicitly added
listeners):

```cpp
server.listen(22, PROTO_SSH);
int32_t result = server.begin();
ssh_conn_setup();   // one-time wiring of the SSH dispatcher's outbound path (after begin)
```

**The host key lives in NVS, not in RAM.** `ssh_rsa_load_pubkey()` loads only the
public half at startup; the private key is read per-signature into a stack buffer
and wiped, so it is never held in static RAM. You must provision the DER key once
per device (namespace `ssh_host_key`, key `priv_der`) - see `docs/SSH.md`:

```cpp
if (ssh_rsa_load_pubkey() != 0) {
    Serial.println("No SSH host key in NVS - see docs/SSH.md (Host key provisioning)");
    return;
}
```

**Auth via callbacks.** You decide who gets in. A password callback and a
public-key callback are installed before `begin()`; the server verifies the
client's signature itself once your pubkey callback accepts the key:

```cpp
ssh_auth_set_password_cb(ssh_password_auth);  // return true to accept user/pass
ssh_auth_set_pubkey_cb(ssh_pubkey_auth);      // return true to accept (user, key blob)
ssh_channel_set_data_cb(ssh_on_data);         // bytes from the client
```

**Channel echo.** The data callback receives the channel id the bytes arrived on
and sends them back with `ssh_conn_send(slot, channel, data, len)` - the skeleton
for a remote console. With `DETWS_SSH_MAX_CHANNELS > 1` the client can open several
channels over one connection and each is tagged by id.

**Hardening.** Define `DETWS_SSH_ALLOW_PASSWORD 0` to compile password auth out
entirely (publickey-only), and failed attempts are bounded by
`SSH_MAX_AUTH_ATTEMPTS`. Use a constant-time compare and real credential storage
in production - the demo's `strcmp` is illustrative only.

## Build and run

`DETWS_ENABLE_SSH` must reach the library build, so pass it as a build flag:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_SSH=1" \
  --lib="." examples/L5-Session/01.SSH/01.SSH.ino
```

Provision a host key first (see `docs/SSH.md`), then:

```sh
ssh -p 22 admin@<ip>     # password: s3cret  (type; the server echoes it back)
```

## Annotated source

The complete sketch ([01.SSH.ino](01.SSH.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Enable the SSH stack for this sketch (overrides the default-off config).
#define DETWS_ENABLE_SSH 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/ssh/ssh_auth.h"    // ssh_auth_set_*_cb
#include "network_drivers/presentation/ssh/ssh_channel.h" // ssh_channel_set_data_cb
#include "network_drivers/presentation/ssh/ssh_conn.h"    // ssh_conn_send / ssh_conn_setup
#include "network_drivers/presentation/ssh/ssh_rsa.h"     // ssh_rsa_load_pubkey
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// --- Authentication callbacks ----------------------------------------------

// Return true to accept the username/password. Use a constant-time compare and
// real credential storage in production; this is illustrative only.
static bool ssh_password_auth(const char *user, const char *pass)
{
    return strcmp(user, "admin") == 0 && strcmp(pass, "s3cret") == 0;
}

// Return true if (user, public-key blob) is authorized. Compare the raw blob
// against your authorized_keys (the server verifies the client's signature
// itself once you accept the key here).
static bool ssh_pubkey_auth(const char *user, const uint8_t *blob, size_t blob_len)
{
    (void)blob;
    (void)blob_len;
    // e.g. return user_key_matches(user, blob, blob_len);
    return strcmp(user, "admin") == 0; // accept any key for "admin" in this demo
}

// --- Channel data: echo received bytes back to the client -------------------

static void ssh_on_data(uint8_t slot, uint32_t channel, const uint8_t *data, size_t len)
{
    ssh_conn_send(slot, channel, data, len); // echo
}

void setup()
{
    Serial.begin(115200);

    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());

    // Load the RSA host key's public half from NVS (the private key is read
    // per-signature into a stack buffer and wiped; never held in static RAM).
    if (ssh_rsa_load_pubkey() != 0)
    {
        Serial.println("No SSH host key in NVS - see docs/SSH.md (Host key provisioning)");
        return;
    }

    // Install SSH callbacks before begin().
    ssh_auth_set_password_cb(ssh_password_auth);
    ssh_auth_set_pubkey_cb(ssh_pubkey_auth);
    ssh_channel_set_data_cb(ssh_on_data);

    // Listen for SSH on port 22 (and, optionally, HTTP on 80 alongside it).
    server.listen(22, PROTO_SSH);
    int32_t result = server.begin();
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }

    // One-time wiring of the SSH dispatcher's outbound path. Call after begin().
    ssh_conn_setup();

    Serial.println("SSH server started on port 22");
}

void loop()
{
    // Drives accept/rx for every listener, including the PROTO_SSH handshake,
    // user-auth, and channel data pumping.
    server.handle();
}
```
