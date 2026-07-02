// 19.Ethernet - run the server over a wired Ethernet PHY instead of Wi-Fi.
//
// Some deployments want a wired uplink (PoE cameras, panel-mount controllers, noisy RF
// environments). With DETWS_ENABLE_ETHERNET the physical layer gains init_eth_physical()
// alongside init_wifi_physical() - a thin wrapper over the Arduino ETH library for an RMII
// PHY (LAN8720 / TLK110 / RTL8201 / DP83848). Once the link has an IP the server accepts on
// it with no other change: the egress reporting already classifies a wired route as
// DETIFACE_ETH, so per-route interface filters and everything else just work.
//
// The PHY pins / type / clock come from the standard ETH_PHY_* build flags (below) - set
// them for your board. Needs an ESP32 with an Ethernet PHY to run.
//
// Build flags (whole build), tuned here for a LAN8720 board:
//   DETWS_ENABLE_ETHERNET=1
//   ETH_PHY_TYPE=ETH_PHY_LAN8720 ETH_PHY_ADDR=1 ETH_PHY_POWER=-1
//   ETH_PHY_MDC=23 ETH_PHY_MDIO=18 ETH_CLK_MODE=ETH_CLOCK_GPIO0_IN

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <ETH.h>

DetWebServer server;

static unsigned long request_count = 0;

void handle_root(uint8_t slot_id, HttpReq *)
{
    request_count++;
    server.send(slot_id, 200, "text/plain", "Served over wired Ethernet.");
}

void handle_status(uint8_t slot_id, HttpReq *)
{
    request_count++;
    char body[128];
    snprintf(body, sizeof(body), "{\"link\":\"ethernet\",\"count\":%lu,\"uptime_ms\":%lu,\"free_heap\":%u}",
             request_count, millis(), ESP.getFreeHeap());
    server.send(slot_id, 200, "application/json", body);
}

void setup()
{
    Serial.begin(115200);

    init_eth_physical(); // ETH.begin() with the ETH_PHY_* build-flag config
    Serial.print("Bringing up Ethernet");
    while (!eth_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(ETH.localIP());

    server.on("/", HTTP_GET, handle_root);
    server.on("/api/status", HTTP_GET, handle_status);

    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on :80 over Ethernet");
}

void loop()
{
    server.handle();
}
