// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ota_rollback.cpp
 * @brief OTA rollback decision (pure) + esp_ota_ops commit/rollback (ESP32).
 */

#include "services/ota_rollback/ota_rollback.h"

#if DETWS_ENABLE_OTA_ROLLBACK

DetwsOtaAction detws_ota_decide(uint8_t img_state, bool self_test_ok, uint32_t ms_since_boot, uint32_t window_ms)
{
    if (img_state != DetwsOtaImg::DETWS_OTA_IMG_PENDING_VERIFY)
        return DetwsOtaAction::DETWS_OTA_WAIT; // not a freshly-updated image: nothing to do
    if (self_test_ok)
        return DetwsOtaAction::DETWS_OTA_COMMIT;
    if (ms_since_boot >= window_ms)
        return DetwsOtaAction::DETWS_OTA_ROLLBACK; // never confirmed in time -> self-heal
    return DetwsOtaAction::DETWS_OTA_WAIT;
}

#ifdef ARDUINO

#include "esp_ota_ops.h"
#include "services/clock.h" // detws_millis() (pulls in Arduino millis())

uint8_t detws_ota_img_state(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t st = ESP_OTA_IMG_UNDEFINED;
    if (!running || esp_ota_get_state_partition(running, &st) != ESP_OK)
        return DetwsOtaImg::DETWS_OTA_IMG_UNDEFINED;
    return (uint8_t)st;
}

void detws_ota_commit(void)
{
    esp_ota_mark_app_valid_cancel_rollback();
}

void detws_ota_rollback(void)
{
    esp_ota_mark_app_invalid_rollback_and_reboot(); // does not return
}

DetwsOtaAction detws_ota_rollback_tick(bool self_test_ok)
{
    DetwsOtaAction a =
        detws_ota_decide(detws_ota_img_state(), self_test_ok, detws_millis(), DETWS_OTA_CONFIRM_WINDOW_MS);
    if (a == DetwsOtaAction::DETWS_OTA_COMMIT)
        detws_ota_commit();
    else if (a == DetwsOtaAction::DETWS_OTA_ROLLBACK)
        detws_ota_rollback();
    return a;
}

#else // host build - no OTA partitions

uint8_t detws_ota_img_state(void)
{
    return DetwsOtaImg::DETWS_OTA_IMG_UNDEFINED;
}
void detws_ota_commit(void)
{
}
void detws_ota_rollback(void)
{
}
DetwsOtaAction detws_ota_rollback_tick(bool)
{
    return DetwsOtaAction::DETWS_OTA_WAIT;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_OTA_ROLLBACK
