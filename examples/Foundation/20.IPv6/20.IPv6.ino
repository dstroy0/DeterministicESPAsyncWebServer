// 20.IPv6 - serve over IPv6 (dual-stack), alongside IPv4.
//
// The TCP and UDP listeners already bind IPADDR_TYPE_ANY, so the moment the interface has an
// IPv6 address the server answers over v6 with no extra work. DETWS_ENABLE_IPV6 turns IPv6 on
// for the Wi-Fi netif (init_ipv6_physical() -> SLAAC: a link-local address, plus a global one
// if the network advertises a prefix). The DetIp address core
// (network_drivers/network/ip.h) parses, formats (RFC 5952 canonical), and classifies both
// families - used here to print and report the acquired address.
//
// Build flag (whole build, not just this sketch):
//   DETWS_ENABLE_IPV6=1

#include "dwserver.h"
#include "network_drivers/network/ip.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static const char *scope_name(DetIpScope s)
{
    switch (s)
    {
    case DetIpScope::DET_IP_SCOPE_LOOPBACK:
        return "loopback";
    case DetIpScope::DET_IP_SCOPE_LINK_LOCAL:
        return "link-local";
    case DetIpScope::DET_IP_SCOPE_PRIVATE:
        return "unique-local";
    case DetIpScope::DET_IP_SCOPE_MULTICAST:
        return "multicast";
    case DetIpScope::DET_IP_SCOPE_GLOBAL:
        return "global";
    default:
        return "unspecified";
    }
}

void handle_root(uint8_t slot_id, HttpReq *)
{
    DetIp v6;
    char buf[160];
    if (net_global_ipv6(&v6))
    {
        char addr[DET_IP_STR_MAX];
        det_ip_format(&v6, addr, sizeof(addr));
        snprintf(buf, sizeof(buf), "Served over IPv6. My global address is [%s] (%s).", addr,
                 scope_name(det_ip_classify(&v6)));
    }
    else
    {
        snprintf(buf, sizeof(buf), "Served over IPv4 (no global IPv6 address yet).");
    }
    server.send(slot_id, 200, "text/plain", buf);
}

void setup()
{
    Serial.begin(115200);

    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    init_ipv6_physical(); // enable IPv6 (SLAAC) on the Wi-Fi netif

    Serial.print("IPv4: ");
    Serial.println(WiFi.localIP());
    Serial.print("Waiting for a global IPv6 address");
    for (int i = 0; i < 40 && !ipv6_ready(); i++)
    {
        delay(250);
        Serial.print('.');
    }

    DetIp v6;
    if (net_global_ipv6(&v6))
    {
        char addr[DET_IP_STR_MAX];
        det_ip_format(&v6, addr, sizeof(addr));
        Serial.printf("\nIPv6: %s\n", addr);
        Serial.printf("Try: curl -g 'http://[%s]/'\n", addr);
    }
    else
    {
        Serial.println("\nNo global IPv6 yet (the network may not advertise a prefix); link-local still works.");
    }

    server.on("/", HTTP_GET, handle_root);

    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on :80 (dual-stack IPv4 + IPv6)");
}

void loop()
{
    server.handle();
}
