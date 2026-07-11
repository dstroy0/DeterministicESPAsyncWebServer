// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file radio_power.cpp
 * @brief Modem-sleep mode names (pure) + esp_wifi apply/readback (ESP32).
 */

#include "services/radio_power/radio_power.h"

#if DETWS_ENABLE_RADIO_POWER

const char *detws_radio_ps_name(uint8_t mode)
{
    switch (mode)
    {
    case DetwsRadioPs::DETWS_PS_MIN_MODEM:
        return "min_modem";
    case DetwsRadioPs::DETWS_PS_MAX_MODEM:
        return "max_modem";
    case DetwsRadioPs::DETWS_PS_NONE:
        return "none";
    default:
        return "none";
    }
}

#ifdef ARDUINO

#include "esp_wifi.h"

namespace
{
wifi_ps_type_t to_esp_ps(uint8_t mode)
{
    if (mode == DetwsRadioPs::DETWS_PS_MIN_MODEM)
        return WIFI_PS_MIN_MODEM;
    if (mode == DetwsRadioPs::DETWS_PS_MAX_MODEM)
        return WIFI_PS_MAX_MODEM;
    return WIFI_PS_NONE;
}

// Bulk-transfer keep-awake refcount, owned in one context (owner-context guard).
struct RadioBusyCtx
{
    int held;
};
RadioBusyCtx s_busy;
} // namespace

void detws_radio_power_apply(void)
{
    esp_wifi_set_ps(to_esp_ps(DETWS_RADIO_WIFI_PS));
#if DETWS_RADIO_MAX_TX_DBM > 0
    esp_wifi_set_max_tx_power((int8_t)(DETWS_RADIO_MAX_TX_DBM * 4)); // API unit: 0.25 dBm
#endif
}

uint8_t detws_radio_ps_get(void)
{
    wifi_ps_type_t m = WIFI_PS_NONE;
    if (esp_wifi_get_ps(&m) != ESP_OK)
        return DetwsRadioPs::DETWS_PS_NONE;
    if (m == WIFI_PS_MIN_MODEM)
        return DetwsRadioPs::DETWS_PS_MIN_MODEM;
    if (m == WIFI_PS_MAX_MODEM)
        return DetwsRadioPs::DETWS_PS_MAX_MODEM;
    return DetwsRadioPs::DETWS_PS_NONE;
}

void detws_radio_busy_hold(void)
{
    if (s_busy.held++ == 0)
        esp_wifi_set_ps(WIFI_PS_NONE); // modem sleep off while a bulk transfer is in flight
}

void detws_radio_busy_release(void)
{
    if (s_busy.held > 0 && --s_busy.held == 0)
        detws_radio_power_apply(); // last transfer done: restore the configured mode
}

#else // host build - no radio

void detws_radio_power_apply(void)
{
}
uint8_t detws_radio_ps_get(void)
{
    return DetwsRadioPs::DETWS_PS_NONE;
}
void detws_radio_busy_hold(void)
{
    // no-op on the host build: there is no radio to keep awake (the ESP32 branch above holds the
    // modem-sleep refcount).
}
void detws_radio_busy_release(void)
{
    // no-op on the host build (see detws_radio_busy_hold).
}

#endif // ARDUINO

#endif // DETWS_ENABLE_RADIO_POWER
