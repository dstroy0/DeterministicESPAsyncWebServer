// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file provisioning_service.cpp
 * @brief First-boot WiFi provisioning / captive portal (DETWS_ENABLE_PROVISIONING).
 *
 * The catch-all DNS responder is a raw lwIP UDP socket (no add-on library);
 * credentials persist to NVS via Preferences.
 */

#include "provisioning_service.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Form-field parser (always compiled; the only non-trivial logic, unit-tested).
// ---------------------------------------------------------------------------

static int prov_hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool detws_prov_form_field(const char *body, const char *key, char *out, size_t cap)
{
    if (out && cap)
        out[0] = '\0';
    if (!body || !key || !out || cap == 0)
        return false;

    size_t klen = strlen(key);
    const char *val = nullptr;
    for (const char *p = body; *p; p++)
    {
        // Match the key only as a whole field name: at the start or after '&'.
        if ((p == body || p[-1] == '&') && strncmp(p, key, klen) == 0 && p[klen] == '=')
        {
            val = p + klen + 1;
            break;
        }
    }
    if (!val)
        return false;

    size_t o = 0;
    for (const char *q = val; *q && *q != '&'; q++)
    {
        char c = *q;
        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%')
        {
            int h = prov_hexval(q[1]);
            int l = (h >= 0) ? prov_hexval(q[2]) : -1;
            if (h >= 0 && l >= 0)
            {
                c = (char)((h << 4) | l);
                q += 2;
            }
        }
        if (o + 1 < cap)
            out[o++] = c;
        else
            break;
    }
    out[o] = '\0';
    return true;
}

// ---------------------------------------------------------------------------
// ESP32 captive portal (softAP + lwIP UDP DNS + form/save routes)
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_PROVISIONING && defined(ARDUINO)

#include "DeterministicESPAsyncWebServer.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

static DetWebServer *g_server = nullptr;
static struct udp_pcb *g_dns_pcb = nullptr;
static uint8_t g_ap_ip[4] = {192, 168, 4, 1};

static const char PROV_FORM[] = "<!DOCTYPE html><html><head><meta name=viewport content='width=device-width'>"
                                "<title>WiFi setup</title></head><body><h2>WiFi setup</h2>"
                                "<form method=POST action=/save>"
                                "SSID:<br><input name=ssid maxlength=32><br>"
                                "Password:<br><input name=psk type=password maxlength=63><br><br>"
                                "<input type=submit value=Save></form></body></html>";

// Catch-all DNS: answer every query with our softAP IP (captive-portal hijack).
static void prov_dns_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg;
    if (!p)
        return;
    uint8_t req[256];
    u16_t qlen = (p->len < sizeof(req)) ? p->len : sizeof(req);
    pbuf_copy_partial(p, req, qlen, 0);
    pbuf_free(p);
    if (qlen < 12)
        return; // smaller than a DNS header

    // Walk the (single) question to find where it ends: labels until a 0 byte,
    // then 2-byte QTYPE + 2-byte QCLASS.
    size_t qend = 12;
    while (qend < qlen && req[qend] != 0)
        qend += (size_t)req[qend] + 1;
    qend += 1 + 4;
    if (qend > qlen)
        return;

    uint8_t resp[300];
    if (qend + 16 > sizeof(resp))
        return;
    memcpy(resp, req, qend);
    resp[2] = 0x81; // QR=1, opcode 0, RD copied
    resp[3] = 0x80; // RA=1, RCODE 0
    resp[6] = 0x00;
    resp[7] = 0x01; // ANCOUNT = 1
    resp[8] = resp[9] = resp[10] = resp[11] = 0x00;

    size_t n = qend;
    resp[n++] = 0xC0; // name: pointer to the question at offset 0x0C
    resp[n++] = 0x0C;
    resp[n++] = 0x00;
    resp[n++] = 0x01; // TYPE A
    resp[n++] = 0x00;
    resp[n++] = 0x01; // CLASS IN
    resp[n++] = 0x00;
    resp[n++] = 0x00;
    resp[n++] = 0x00;
    resp[n++] = 0x3C; // TTL 60s
    resp[n++] = 0x00;
    resp[n++] = 0x04; // RDLENGTH 4
    resp[n++] = g_ap_ip[0];
    resp[n++] = g_ap_ip[1];
    resp[n++] = g_ap_ip[2];
    resp[n++] = g_ap_ip[3];

    struct pbuf *out = pbuf_alloc(PBUF_TRANSPORT, (u16_t)n, PBUF_RAM);
    if (out)
    {
        memcpy(out->payload, resp, n);
        udp_sendto(pcb, out, addr, port);
        pbuf_free(out);
    }
}

bool detws_provisioning_load(char *ssid, size_t ssid_cap, char *psk, size_t psk_cap)
{
    if (ssid && ssid_cap)
        ssid[0] = '\0';
    if (psk && psk_cap)
        psk[0] = '\0';
    Preferences prefs;
    if (!prefs.begin("wifi_prov", true))
        return false;
    String s = prefs.getString("ssid", "");
    String k = prefs.getString("psk", "");
    prefs.end();
    if (s.length() == 0)
        return false;
    strncpy(ssid, s.c_str(), ssid_cap - 1);
    ssid[ssid_cap - 1] = '\0';
    strncpy(psk, k.c_str(), psk_cap - 1);
    psk[psk_cap - 1] = '\0';
    return true;
}

void detws_provisioning_clear()
{
    Preferences prefs;
    if (prefs.begin("wifi_prov", false))
    {
        prefs.clear();
        prefs.end();
    }
}

static void prov_form_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    g_server->send(slot_id, 200, "text/html", PROV_FORM);
}

static void prov_save_handler(uint8_t slot_id, HttpReq *req)
{
    char ssid[33];
    char psk[64];
    bool have_ssid = detws_prov_form_field((const char *)req->body, "ssid", ssid, sizeof(ssid)) && ssid[0] != '\0';
    detws_prov_form_field((const char *)req->body, "psk", psk, sizeof(psk));
    if (!have_ssid)
    {
        g_server->send(slot_id, 400, "text/plain", "SSID required");
        return;
    }
    Preferences prefs;
    prefs.begin("wifi_prov", false);
    prefs.putString("ssid", ssid);
    prefs.putString("psk", psk);
    prefs.end();
    g_server->send(slot_id, 200, "text/html", "<html><body>Saved. Rebooting...</body></html>");
    delay(500);
    ESP.restart();
}

void detws_provisioning_begin(DetWebServer &server, const char *ap_ssid)
{
    g_server = &server;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid);
    IPAddress ip = WiFi.softAPIP();
    g_ap_ip[0] = ip[0];
    g_ap_ip[1] = ip[1];
    g_ap_ip[2] = ip[2];
    g_ap_ip[3] = ip[3];

    // Catch-all DNS on UDP/53 (raw lwIP - callback-driven, no polling).
    g_dns_pcb = udp_new();
    if (g_dns_pcb)
    {
        udp_bind(g_dns_pcb, IP_ANY_TYPE, 53);
        udp_recv(g_dns_pcb, prov_dns_recv, nullptr);
    }

    server.on("/save", HTTP_POST, prov_save_handler);
    server.on("/*", HTTP_GET, prov_form_handler); // any other path -> the form
}

#else // disabled / non-Arduino: stubs (form-field parser above stays available)

bool detws_provisioning_load(char *ssid, size_t ssid_cap, char *psk, size_t psk_cap)
{
    if (ssid && ssid_cap)
        ssid[0] = '\0';
    if (psk && psk_cap)
        psk[0] = '\0';
    return false;
}
void detws_provisioning_begin(DetWebServer &server, const char *ap_ssid)
{
    (void)server;
    (void)ap_ssid;
}
void detws_provisioning_clear()
{
}

#endif // DETWS_ENABLE_PROVISIONING && ARDUINO
