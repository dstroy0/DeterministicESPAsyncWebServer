// WifiSniffer - channel-hopping 802.11 traffic analyzer + channel-agility roaming decision.
//
// A passive RF-diagnostics panel: sweep the 2.4 GHz channels, decode every 802.11 MAC header the
// radio hears, tally frames by type, and keep a per-channel survey of the strongest AP. That
// survey is what a channel-agility roam decides on - "is another channel enough better than mine
// to be worth moving?" (dws_wifi_should_roam's RSSI hysteresis).
//
//   Wi-Fi radio --dws_promisc_begin--> sink --dws_wifi_parse--> stats tally
//                                                            \-> per-channel survey -> roam decision
//                     ^                                                                    |
//                     +----------------- dws_wifi_sniffer_tick() hops on the dwell --------+
//
// The capture itself is owned by services/promisc (one owner for the radio); services/wifi_sniffer
// adds the decode, the tally, the channel-hop schedule, and the survey. Everything except the thin
// radio binding is pure and host-tested in test/test_wifi_sniffer.
//
// Strictly passive: no injection, no association. Sniffing a network you do not administer may be
// unlawful where you are - point it at your own.
//
// Build flags (whole build): DWS_ENABLE_WIFI_SNIFFER=1 DWS_ENABLE_PROMISC=1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/clock.h" // dws_millis
#include "services/wifi_sniffer/wifi_sniffer.h"

static const uint8_t CHAN_FIRST = 1; // sweep 1..11 (the US 2.4 GHz plan)
static const uint8_t CHAN_LAST = 11;
static const uint16_t DWELL_MS = 250; // per-channel dwell; a beacon interval is ~102 ms
static const uint8_t ROAM_HYSTERESIS_DB = 8;

void setup()
{
    Serial.begin(115200);
    delay(300);

    // Radio up for capture only - promiscuous mode does not associate.
    init_wifi_radio_physical(0);

    if (!dws_wifi_sniffer_begin(CHAN_FIRST, CHAN_LAST, DWELL_MS))
    {
        Serial.println("sniffer: failed to start promiscuous capture");
        return;
    }
    Serial.printf("Sniffing channels %u-%u, %u ms dwell\n", CHAN_FIRST, CHAN_LAST, DWELL_MS);
}

void loop()
{
    dws_wifi_sniffer_tick(); // hops to the next channel when the dwell elapses

    static uint32_t last_report = 0;
    if (dws_millis() - last_report < 5000)
        return;
    last_report = dws_millis();

    const WifiStats *st = dws_wifi_sniffer_stats();
    const WifiSurvey *sv = dws_wifi_sniffer_survey();
    const WifiScan *sc = dws_wifi_sniffer_scan();

    Serial.printf("\n-- ch %u, sweep %lu -- frames %lu (mgmt %lu, ctrl %lu, data %lu, other %lu)\n", sc->channel,
                  (unsigned long)sc->sweeps, (unsigned long)st->total, (unsigned long)st->mgmt, (unsigned long)st->ctrl,
                  (unsigned long)st->data, (unsigned long)st->other);

    for (uint8_t ch = CHAN_FIRST; ch <= CHAN_LAST; ch++)
    {
        const WifiChannelSurvey *e = dws_wifi_survey_get(sv, ch);
        if (!e || e->frames == 0)
            continue;
        Serial.printf("  ch %2u: %6lu frames, best %d dBm from %02X:%02X:%02X:%02X:%02X:%02X\n", ch,
                      (unsigned long)e->frames, (int)e->best_rssi, e->best_bssid[0], e->best_bssid[1], e->best_bssid[2],
                      e->best_bssid[3], e->best_bssid[4], e->best_bssid[5]);
    }

    // Channel-agility: is any other channel enough stronger than the one we are on?
    const WifiChannelSurvey *cur = dws_wifi_survey_get(sv, sc->channel);
    uint8_t cand_ch = 0;
    int8_t cand_rssi = 0;
    if (cur && cur->best_rssi != DWS_WIFI_RSSI_NONE && dws_wifi_survey_best(sv, sc->channel, &cand_ch, &cand_rssi))
    {
        bool roam = dws_wifi_should_roam(cur->best_rssi, cand_rssi, ROAM_HYSTERESIS_DB);
        Serial.printf("  roam? ch %u (%d dBm) -> ch %u (%d dBm): %s\n", sc->channel, (int)cur->best_rssi, cand_ch,
                      (int)cand_rssi, roam ? "YES" : "no");
    }
}
