// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file power_mgmt.cpp
 * @brief The power governor's decision + its device binding (see power_mgmt.h).
 */

#include "services/power_mgmt/power_mgmt.h"

#if DWS_ENABLE_POWER_MGMT

#include <stdio.h>

// ---------------------------------------------------------------------------
// Pure decision
// ---------------------------------------------------------------------------

void dws_power_cfg_defaults(PowerCfg *cfg)
{
    if (!cfg)
        return;
    cfg->mhz_max = DWS_POWER_MHZ_MAX;
    cfg->mhz_min = DWS_POWER_MHZ_MIN;
    cfg->busy_pct = DWS_POWER_BUSY_PCT;
    cfg->temp_hot_c = DWS_POWER_TEMP_HOT_C;
    cfg->temp_cool_c = DWS_POWER_TEMP_COOL_C;
    cfg->recover_ms = DWS_POWER_RECOVER_MS;
}

PowerPlan dws_power_plan(const PowerCfg *cfg, uint8_t load_pct, int16_t temp_c, bool brownout_boot,
                         uint32_t since_boot_ms, bool was_throttled)
{
    PowerPlan p;
    p.cpu_mhz = 0;
    p.throttled = false;
    p.recovering = false;
    if (!cfg)
        return p;

    // Hysteresis: once throttled, hold it until the die drops to the *cool* threshold. With one
    // threshold a part sitting at the limit would flap between ceiling and floor every tick.
    // INT16_MIN means "no sensor on this part", which must not read as ice-cold and un-throttle.
    bool have_temp = temp_c != INT16_MIN;
    if (have_temp)
        p.throttled = was_throttled ? (temp_c > cfg->temp_cool_c) : (temp_c >= cfg->temp_hot_c);
    else
        p.throttled = false;

    // A supply that just failed under load gets a gentle restart rather than an immediate return to
    // the clock that browned it out.
    p.recovering = brownout_boot && since_boot_ms < cfg->recover_ms;

    if (p.recovering || p.throttled)
    {
        p.cpu_mhz = cfg->mhz_min;
        return p;
    }
    if (load_pct > 100)
        load_pct = 100;
    p.cpu_mhz = (load_pct >= cfg->busy_pct) ? cfg->mhz_max : cfg->mhz_min;
    return p;
}

size_t dws_power_json(const PowerPlan *plan, int16_t temp_c, char *out, size_t cap)
{
    if (!plan || !out || cap == 0)
        return 0;
    int n;
    if (temp_c == INT16_MIN) // no sensor: report null rather than a sentinel that reads as a reading
        n = snprintf(out, cap, "{\"cpu_mhz\":%u,\"throttled\":%s,\"recovering\":%s,\"temp_c\":null}",
                     (unsigned)plan->cpu_mhz, plan->throttled ? "true" : "false", plan->recovering ? "true" : "false");
    else
        n = snprintf(out, cap, "{\"cpu_mhz\":%u,\"throttled\":%s,\"recovering\":%s,\"temp_c\":%d}",
                     (unsigned)plan->cpu_mhz, plan->throttled ? "true" : "false", plan->recovering ? "true" : "false",
                     (int)temp_c);
    if (n < 0 || (size_t)n >= cap)
    {
        out[0] = '\0';
        return 0; // fail closed rather than emit a truncated object
    }
    return (size_t)n;
}

// ---------------------------------------------------------------------------
// Device binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include <Arduino.h>
#include <esp_system.h>

#if defined(CONFIG_BT_ENABLED)
#include <esp_bt.h>
#endif

/** @brief Owned state: the latched boot reason and whether BT has already been released. */
struct PowerCtx
{
    bool brownout_latched;
    bool boot_checked;
    bool bt_released;
};
static PowerCtx s_pwr = {false, false, false};

bool dws_power_brownout_boot(void)
{
    // Read once and latch: the reset reason describes this boot, so it must not change under a
    // caller polling it every tick through the recovery window.
    if (!s_pwr.boot_checked)
    {
        s_pwr.brownout_latched = (esp_reset_reason() == ESP_RST_BROWNOUT);
        s_pwr.boot_checked = true;
    }
    return s_pwr.brownout_latched;
}

int16_t dws_power_temp_c(void)
{
#if defined(SOC_TEMP_SENSOR_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) ||  \
    defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32P4)
    float t = temperatureRead();
    // The driver reports a sentinel far outside any real die temperature when the sensor is not up.
    if (t < -60.0f || t > 200.0f)
        return INT16_MIN;
    return (int16_t)(t + (t < 0 ? -0.5f : 0.5f));
#else
    return INT16_MIN; // classic ESP32 has no usable internal sensor
#endif
}

uint16_t dws_power_cpu_mhz(void)
{
    return (uint16_t)getCpuFrequencyMhz();
}

bool dws_power_apply(const PowerPlan *plan)
{
    if (!plan || plan->cpu_mhz == 0)
        return false;
    if (dws_power_cpu_mhz() == plan->cpu_mhz)
        return false; // already there; re-setting the clock is not free
    return setCpuFrequencyMhz((uint32_t)plan->cpu_mhz);
}

bool dws_power_gate_bt(void)
{
#if defined(CONFIG_BT_ENABLED)
    if (s_pwr.bt_released)
        return false;
    // Disable before release: releasing an enabled controller's memory is rejected, and the
    // whole point is to actually drop the domain rather than report a success that did not happen.
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED)
        esp_bt_controller_disable();
    bool ok = esp_bt_controller_mem_release(ESP_BT_MODE_BTDM) == ESP_OK;
    s_pwr.bt_released = ok;
    return ok;
#else
    return false; // BT not built in, so there is nothing holding the domain
#endif
}

#endif // ARDUINO

#endif // DWS_ENABLE_POWER_MGMT
