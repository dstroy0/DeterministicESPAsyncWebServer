// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file device_id.h
 * @brief Stable device UUID derived from the chip MAC (DETWS_ENABLE_DEVICE_ID).
 *
 * detws_uuid_from_mac() computes a deterministic RFC 4122 version-5 UUID from a
 * 6-byte MAC: namespace = the RFC 4122 DNS namespace, name = the lowercase MAC
 * hex, hashed with the library's SHA-1. The same MAC always yields the same
 * UUID, so it is a stable device identity (mDNS hostname, MQTT client ID, ...)
 * that needs no storage. detws_device_uuid() reads the ESP32 factory MAC and
 * formats it (ESP32 only). Pure, host-testable core; no heap.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DEVICE_ID_H
#define DETERMINISTICESPASYNCWEBSERVER_DEVICE_ID_H

#include "DetWebServerConfig.h"
#include <stdint.h>

#if DETWS_ENABLE_DEVICE_ID

/** @brief Length of a formatted UUID string including the null terminator. */
#define DETWS_UUID_STR_LEN 37

/**
 * @brief Format a deterministic RFC 4122 v5 UUID from a 6-byte MAC.
 *
 * @param mac  six MAC bytes.
 * @param out  buffer of at least DETWS_UUID_STR_LEN bytes; receives
 *             "xxxxxxxx-xxxx-5xxx-yxxx-xxxxxxxxxxxx" (lowercase, null-terminated).
 */
void detws_uuid_from_mac(const uint8_t mac[6], char out[DETWS_UUID_STR_LEN]);

#ifdef ARDUINO
/**
 * @brief Format this device's UUID from its ESP32 factory (WiFi STA) MAC.
 * @param out  buffer of at least DETWS_UUID_STR_LEN bytes.
 */
void detws_device_uuid(char out[DETWS_UUID_STR_LEN]);
#endif

#endif // DETWS_ENABLE_DEVICE_ID
#endif // DETERMINISTICESPASYNCWEBSERVER_DEVICE_ID_H
