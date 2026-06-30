// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file shim.h
 * @brief One place for the portable C standard-library (and Arduino) headers the library uses.
 *
 * The same handful of `<stdint.h>` / `<stddef.h>` / `<string.h>` (and friends) were included
 * file by file across the whole codebase. This shim pulls them in once, with a comment on each
 * naming the symbols this codebase actually uses from it, so the dependency surface is visible
 * in one place. Portable, host-safe source files include `shared_primitives/shim.h` instead of
 * re-listing the standard headers.
 *
 * Scope: only the portable C/C++ standard headers plus Arduino.h (guarded by `ARDUINO`, so a
 * host / native codec build never pulls it in). Layer-specific headers - lwIP, FreeRTOS,
 * ESP-IDF (`esp_*`), and mbedTLS - are deliberately NOT here: they belong to the transport /
 * TLS / network layers, the codec test environments do not have their include paths, and
 * routing them through a global shim would couple every pure codec to those stacks. Per
 * project policy this shim never includes `<stdlib.h>` (parse by hand; see det_numparse.h).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#pragma once

#ifndef DETERMINISTICESPASYNCWEBSERVER_SHIM_H
#define DETERMINISTICESPASYNCWEBSERVER_SHIM_H

#include <atomic>

#include <stddef.h> // size_t, ptrdiff_t, NULL
#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <string.h> // memcpy, memmove, memset, memcmp, memchr, strlen, strnlen, strcmp, strncmp,
                    // strcasecmp, strncasecmp, strchr, strrchr, strstr, strncpy
#include <assert.h> // assert (host-side invariant checks)
#include <math.h>   // floating-point math for the telemetry helpers (e.g. sqrtf for std-dev)
#include <stdarg.h> // va_list, va_start, va_end (the log / format varargs helpers)
#include <stdio.h>  // snprintf, vsnprintf, sscanf (format into / scan from fixed buffers)
#include <time.h>   // time_t, struct tm, gmtime_r, strftime (HTTP-date / clock formatting)

#if defined(ARDUINO)
#include <mbedtls/base64.h>
#include <mbedtls/error.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform.h> // mbedtls_platform_set_calloc_free
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h> // peer-cert pin hashing (client verification)
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_ticket.h> // RFC 5077 session tickets (server-side resumption)
#include <mbedtls/version.h>
#include <mbedtls/x509_crt.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "lwip/def.h"     // lwip_ntohl - allowlist host-order conversion
#include "lwip/ip_addr.h" // ip_2_ip4 / ip4_addr_get_u32 for interface tagging
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

#include "mdns.h"

#include <Arduino.h> // millis, micros, delay, Serial, pinMode/digitalWrite, String (ESP32 only)
#include <FS.h>
#include <Preferences.h> // Preferences for the ESP32 NVS config store
#include <WiFi.h>
#include <esp_mac.h>    // esp_read_mac()
#include <esp_system.h> // esp_random() for the Digest nonce CSPRNG
#else
#include "FS.h"
#endif

#include "DetWebServerConfig.h"

#include "shared_primitives/det_can.h"
#include "shared_primitives/det_hex.h"
#include "shared_primitives/det_mime.h"
#include "shared_primitives/det_numparse.h"
#include "shared_primitives/det_ring.h" // DetAtomic + the shared SPSC ring drain primitive
#include "shared_primitives/det_utf8.h"

#include "network_drivers/application/web_assets.h" // String literal web assets (HTML, JS, CSS) for the built-in web UI

#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/cbor/cbor.h"
#include "network_drivers/presentation/http_parser/http_parser.h"
#include "network_drivers/presentation/json/json.h"
#include "network_drivers/presentation/multipart/multipart.h"
#include "network_drivers/presentation/presentation.h"
#include "network_drivers/presentation/sha1/sha1.h"
#include "network_drivers/presentation/sse/sse.h"

#include "network_drivers/presentation/ssh/ssh_sha256.h"

#include "network_drivers/presentation/ssh/ssh_aes256ctr.h"
#include "network_drivers/presentation/ssh/ssh_auth.h"
#include "network_drivers/presentation/ssh/ssh_bignum.h"
#include "network_drivers/presentation/ssh/ssh_channel.h"
#include "network_drivers/presentation/ssh/ssh_conn.h"
#include "network_drivers/presentation/ssh/ssh_dh.h"
#include "network_drivers/presentation/ssh/ssh_forward.h"
#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
#include "network_drivers/presentation/ssh/ssh_keymat.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include "network_drivers/presentation/ssh/ssh_rsa.h"
#include "network_drivers/presentation/ssh/ssh_server.h"
#include "network_drivers/presentation/ssh/ssh_transport.h"

#include "network_drivers/presentation/deflate/deflate.h"
#include "network_drivers/presentation/inflate/inflate.h"
#include "network_drivers/presentation/websocket/websocket.h"

#include "network_drivers/session/proto_handler.h"
#include "network_drivers/session/scratch.h" // per-dispatch arena (keeps the decode buffers off the worker stack)
#include "network_drivers/session/worker.h"

#include "network_drivers/tls/det_tls.h"
#include "network_drivers/transport/det_client.h"
#include "network_drivers/transport/listener.h"
#include "network_drivers/transport/transport.h"
#include "network_drivers/transport/udp_transport.h"

#include "services/audit_log/audit_log.h"
#include "services/auth_lockout/auth_lockout.h"
#include "services/config_store/config_store.h"
#include "services/csrf/csrf.h"
#include "services/det_clock.h"                 // detws_millis() for the stateless Digest nonce timestamp
#include "services/dns_resolver/dns_resolver.h" // shared host->IP resolve (one DNS owner)
#include "services/j1939/j1939.h"
#include "services/protobuf/protobuf.h"
#include "services/snmp/snmp_agent.h"
#include "services/snmp/snmp_ber.h"
#include "services/snmp/snmp_crypto.h"
#include "services/snmp/snmp_v3.h"
#include "services/webdav/webdav.h"

#endif // DETERMINISTICESPASYNCWEBSERVER_SHIM_H
