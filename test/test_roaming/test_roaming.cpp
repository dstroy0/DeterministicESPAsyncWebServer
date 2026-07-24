// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Wi-Fi roaming decision layer (services/roaming): the pure policy that fuses the
// current RSSI, a candidate neighbour list, and an optional 802.11v BTM hint into a roam/stay decision.

#include "services/roaming/roaming.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static const uint8_t CUR[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
static const uint8_t AP_A[6] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
static const uint8_t AP_B[6] = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

static const DwsRoamPolicy POLICY = {/*roam_rssi_threshold_dbm=*/-70, /*hysteresis_db=*/8};

void test_stay_when_link_strong()
{
    // Strong current link (-50); even a stronger candidate does not trigger a roam below the threshold.
    DwsRoamNeighbor nb[2] = {{{0}, 6, -45}, {{0}, 11, -80}};
    memcpy(nb[0].bssid, AP_A, 6);
    memcpy(nb[1].bssid, AP_B, 6);
    DwsRoamDecision d;
    dws_roam_decide(CUR, -50, nb, 2, nullptr, &POLICY, &d);
    TEST_ASSERT_FALSE(d.roam);
    TEST_ASSERT_EQUAL(DWS_ROAM_NONE, d.reason);
}

void test_roam_on_low_rssi_to_strongest()
{
    // Weak current link (-78) and AP_A is clearly stronger (-55): roam to AP_A.
    DwsRoamNeighbor nb[2] = {{{0}, 6, -55}, {{0}, 11, -85}};
    memcpy(nb[0].bssid, AP_A, 6);
    memcpy(nb[1].bssid, AP_B, 6);
    DwsRoamDecision d;
    dws_roam_decide(CUR, -78, nb, 2, nullptr, &POLICY, &d);
    TEST_ASSERT_TRUE(d.roam);
    TEST_ASSERT_EQUAL(DWS_ROAM_LOW_RSSI, d.reason);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(AP_A, d.target_bssid, 6);
    TEST_ASSERT_EQUAL_UINT8(6, d.target_channel);
}

void test_hysteresis_blocks_marginal_roam()
{
    // Weak link (-78) but the best candidate is only 4 dB better (< 8 dB hysteresis): stay.
    DwsRoamNeighbor nb[1] = {{{0}, 6, -74}};
    memcpy(nb[0].bssid, AP_A, 6);
    DwsRoamDecision d;
    dws_roam_decide(CUR, -78, nb, 1, nullptr, &POLICY, &d);
    TEST_ASSERT_FALSE(d.roam);
}

void test_btm_imminent_forces_roam()
{
    DwsRoamNeighbor nb[2] = {{{0}, 6, -60}, {{0}, 11, -50}};
    memcpy(nb[0].bssid, AP_A, 6);
    memcpy(nb[1].bssid, AP_B, 6);
    DwsRoamBtm btm;
    memset(&btm, 0, sizeof(btm));
    btm.present = true;
    btm.disassoc_imminent = true;
    btm.has_preferred = true;
    memcpy(btm.preferred_bssid, AP_A, 6); // network names AP_A even though AP_B is stronger

    // A strong current link (-45) would normally stay, but disassoc-imminent forces the roam to the
    // preferred AP_A.
    DwsRoamDecision d;
    dws_roam_decide(CUR, -45, nb, 2, &btm, &POLICY, &d);
    TEST_ASSERT_TRUE(d.roam);
    TEST_ASSERT_EQUAL(DWS_ROAM_BTM_IMMINENT, d.reason);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(AP_A, d.target_bssid, 6);

    // No preferred -> roam to the strongest candidate (AP_B).
    btm.has_preferred = false;
    dws_roam_decide(CUR, -45, nb, 2, &btm, &POLICY, &d);
    TEST_ASSERT_TRUE(d.roam);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(AP_B, d.target_bssid, 6);
}

void test_btm_suggested_honoured_only_if_not_weaker()
{
    DwsRoamNeighbor nb[1] = {{{0}, 11, -55}}; // AP_B at -55
    memcpy(nb[0].bssid, AP_B, 6);
    DwsRoamBtm btm;
    memset(&btm, 0, sizeof(btm));
    btm.present = true;
    btm.has_preferred = true;
    memcpy(btm.preferred_bssid, AP_B, 6);

    // Current link -50 (strong), suggested AP_B is -55 (weaker): do NOT chase into a worse AP.
    DwsRoamDecision d;
    dws_roam_decide(CUR, -50, nb, 1, &btm, &POLICY, &d);
    TEST_ASSERT_FALSE(d.roam);

    // If AP_B is at least as strong as the current link, honour the steering hint.
    nb[0].rssi_dbm = -48;
    dws_roam_decide(CUR, -50, nb, 1, &btm, &POLICY, &d);
    TEST_ASSERT_TRUE(d.roam);
    TEST_ASSERT_EQUAL(DWS_ROAM_BTM_SUGGESTED, d.reason);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(AP_B, d.target_bssid, 6);
}

void test_never_targets_current_and_guards()
{
    // The neighbour list contains only the current BSSID -> nothing to roam to even on a weak link.
    DwsRoamNeighbor nb[1] = {{{0}, 6, -30}};
    memcpy(nb[0].bssid, CUR, 6);
    DwsRoamDecision d;
    dws_roam_decide(CUR, -90, nb, 1, nullptr, &POLICY, &d);
    TEST_ASSERT_FALSE(d.roam);

    // A null policy falls back to a conservative default (threshold -75, hysteresis 8).
    DwsRoamNeighbor good[1] = {{{0}, 6, -50}};
    memcpy(good[0].bssid, AP_A, 6);
    dws_roam_decide(CUR, -80, good, 1, nullptr, nullptr, &d);
    TEST_ASSERT_TRUE(d.roam);
    TEST_ASSERT_EQUAL(DWS_ROAM_LOW_RSSI, d.reason);

    // Null out is a no-op (no crash); an empty neighbour list stays.
    dws_roam_decide(CUR, -90, nullptr, 0, nullptr, &POLICY, nullptr);
    dws_roam_decide(CUR, -90, nullptr, 0, nullptr, &POLICY, &d);
    TEST_ASSERT_FALSE(d.roam);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_stay_when_link_strong);
    RUN_TEST(test_roam_on_low_rssi_to_strongest);
    RUN_TEST(test_hysteresis_blocks_marginal_roam);
    RUN_TEST(test_btm_imminent_forces_roam);
    RUN_TEST(test_btm_suggested_honoured_only_if_not_weaker);
    RUN_TEST(test_never_targets_current_and_guards);
    return UNITY_END();
}
