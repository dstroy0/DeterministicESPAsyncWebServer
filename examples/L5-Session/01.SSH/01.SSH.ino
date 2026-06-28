// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 01.SSH.ino
 * @brief SSH server example: host key from NVS, auth callbacks, channel echo.
 *
 * Demonstrates the SSH server stack (RFC 4253/4252/4254):
 *   - Enabling SSH (DETWS_ENABLE_SSH) and listening on PROTO_SSH
 *   - Loading the RSA-2048 host key from NVS (see docs/SSH.md "Host key
 *     provisioning" - you must store a DER key under namespace "ssh_host_key",
 *     key "priv_der" once per device before this runs)
 *   - Password auth (ssh_auth_set_password_cb) and publickey auth
 *     (ssh_auth_set_pubkey_cb)
 *   - A channel data callback that echoes received bytes back to the client
 *     with ssh_conn_send()
 *
 * Hardening: define DETWS_SSH_ALLOW_PASSWORD 0 to compile password auth out and
 * accept publickey only. Failed attempts are bounded by SSH_MAX_AUTH_ATTEMPTS.
 *
 * Connect with:  ssh -p 22 admin@<ip>      (password below)
 * Then type; the server echoes each line back over the channel.
 */

// Enable the SSH stack for this sketch (overrides the default-off config).
#define DETWS_ENABLE_SSH 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/ssh/ssh_auth.h"
#include "network_drivers/presentation/ssh/ssh_channel.h"
#include "network_drivers/presentation/ssh/ssh_conn.h"
#include "network_drivers/presentation/ssh/ssh_rsa.h"
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

static void ssh_on_data(uint8_t slot, const uint8_t *data, size_t len)
{
    ssh_conn_send(slot, data, len); // echo
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
