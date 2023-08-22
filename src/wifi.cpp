#include "wifi.h"

#ifdef USE_WEBSERVER
#include "webServer.h"
#endif
#include "inout.h"
#include "main.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#define DNS_PORT 53

Wifi wifi;

/**
 *  Connects our WiFi.
 *  If it can't connect and allowHotSpot is set, it will call startHotSpot
 */
void Wifi::start()
{
  statusLed.set(WIFI_CONNECTING);
  // Connect wifi
  WiFi.begin(settings.wifiSSID, settings.wifiPass);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(settings.nodeName);

  unsigned long connectTime = millis() + settings.hotSpotDelay;
  // Wait for WiFi to connect
  while (notConnected())
  {
    // delay to yield to esp core functions
    delay(10);

    // If it hasn't connected within the preset time, start hot spot
    if (allowHotSpot && millis() > connectTime)
    {
      // start our hotspot
      hotSpot(true);
    }

    // handle buttons and LED
    buttons.handle();
    statusLed.handle();
  }
  statusLed.set(SUCCESS_CONNECTED);

  // Get MAC Address
  getMac();

  // If static address, set our wifi to use it
  if (!settings.dhcp)
  {
    WiFi.config(settings.ip, settings.gw, settings.subnet);
  }
  // If DHCP, get the IPs
  else
  {
    ap_ip = WiFi.localIP();
    settings.ip = WiFi.localIP();
    settings.gw = WiFi.gatewayIP();
    settings.subnet = WiFi.subnetMask();
  }

  // Set our broadcast address
  setBroadcastAddr();

  allowHotSpot = false;
}

/**
 *  This starts our hot spot and webserver.  It doesn't start UDP listner for ArtNet however.
 *  It also resets our device after set timeouts.
 */
void Wifi::hotSpot(bool def)
{
  statusLed.set(WIFI_HOTSPOT);
  // Let other functions know we're a hot spot now
  isHotSpot = true;

  // generate the AP ssid & password
  String ssid = def ? DEFAULT_SSID : settings.wifiSSID;
  String password = def ? DEFAULT_PASSWORD : settings.wifiPass;
  ssid += '_';
  ssid += String(ESP.getChipId(),16);

  // start softAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Get IPs for DHCP
  if (settings.dhcp)
  {
    ap_ip = {192, 168, 42, 1};
    settings.ip = ap_ip;
    settings.subnet = IPAddress(255, 255, 255, 0);
  }
  // Set IP for Static
  else
    ap_ip = settings.ip;

  WiFi.softAPConfig(ap_ip, ap_ip, settings.subnet);
  delay(100);

  // Set our broadcast address
  setBroadcastAddr();

  // Get MAC Address
  getMac();

  #ifdef USE_DNS
  dnsServer.start(DNS_PORT, "*", ap_ip);
  #endif

  // If standAlone, return to main loop
  if (settings.standAlone)
    return;

  // Start webServer
  #ifdef USE_WEBSERVER
  webServer.start();
  #endif

  // Set timer
  unsigned long startTime = millis();

  // Status LEDs
  statusLed.set(WIFI_WAIT_CLIENT);
  // wait for clients to connect
  while ((millis() - startTime) < TIMEOUT_WAIT_CLIENT)
  {
    // check for any clients connected to the hotspot
    if (wifi_softap_get_station_num() != 0)
    {
      // Status LEDs
      statusLed.set(WIFI_HANDLE_CLIENT);
      // main loop
      while (true)
      {
        // handle DNS requests
        #ifdef USE_DNS
        handleDns();
        #endif
        // handle web requests only when in hotSpot mode
        #ifdef USE_WEBSERVER
        webServer.handleClient();
        #endif

        // reset timer when a client is connected
        if (wifi_softap_get_station_num() != 0)
          startTime = millis();

        // if timer reaches timeout, reset the node
        if ((millis() - startTime) > TIMEOUT_END_CLIENT)
          restart();

        // handle buttons
        buttons.handle();
        // handle Status LEDs
        statusLed.handle();

        // yield to core esp functions
        delay(10);
      }
    }

    // handle DNS requests
    #ifdef USE_DNS
    handleDns();
    #endif
    // handle buttons
    buttons.handle();
    // handle Status LEDs
    statusLed.handle();

    // yield to core esp functions
    delay(10);
  }

  restart();
}

/* setBroadcastAddr()
 *  Calculates and sets the broadcast address using the IP and subnet
 */
void Wifi::setBroadcastAddr()
{
  settings.broadcast_ip = ~settings.subnet | (settings.ip & settings.subnet);
}

/* getMac()
 *  This gets the MAC address and formats it for later.
 */
String Wifi::getMac()
{
  if (MAC_address.isEmpty())
  {
    char MAC_char[2 * 6 + 3 * 5 + 1] = "";

    WiFi.macAddress(MAC_array);

    // Format the MAC address into string
    sprintf(MAC_char, "%02X : %02X : %02X : %02X : %02X : %02X", MAC_array[0], MAC_array[1], MAC_array[2], MAC_array[3], MAC_array[4], MAC_array[5]);
    MAC_address = String(MAC_char);
  }
  return MAC_address;
}

bool Wifi::notConnected()
{
  return WiFi.status() != WL_CONNECTED;
}

#ifdef USE_DNS
void Wifi::handleDns()
{
  if (isHotSpot)
    dnsServer.processNextRequest();
};
#endif
