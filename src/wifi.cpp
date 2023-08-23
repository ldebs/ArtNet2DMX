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
 * @brief Initializes and manages the WiFi connection.
 *
 * This function attempts to connect to a predefined WiFi network as configured in settings.
 * If the device is set to standalone mode, it starts an access point using the settings values.
 * If both of these attempts fail, it starts an access point with default values for configuration
 * through a web server if this feature is active.
 *
 * If all connection attempts fail, the function triggers a device restart for another attempt.
 */
void Wifi::start()
{
  bool error = true;

  if (!settings.standAlone
#ifdef USE_WEBSERVER
   && HOTSPOT_SSID != settings.wifiSSID
#endif
   )
  {
    // setup WiFi config
    setupConfigSta();
    // try to connect to Wifi defined in settings
    statusLed.set(WIFI_CONNECTING);
    error = connect();
  }
  else if(settings.standAlone)
  {
    statusLed.set(WIFI_HOTSPOT);
    // start as access point with settings values
    error = accessPoint(false);
  }

#ifdef USE_WEBSERVER
  // If Wifi connection or standAlone modes have failed
  else
  {
    statusLed.set(WIFI_HOTSPOT);
    // start as access point with default values to configure through the webserver
    error = accessPoint(true);
  }
#endif

  if (error)
  {
    // restart and try again
    restart();
  }

  statusLed.set(SUCCESS_CONNECTED);

}

void Wifi::setupConfigSta()
{
  WiFi.hostname(settings.nodeName);

  // If static address, set our wifi to use it
  if (!settings.dhcp)
  {
    WiFi.config(settings.ip, settings.gateway, settings.subnet);
  }

  isAp = false;
}

bool Wifi::connect()
{
  bool timeout = false;

  // Connect wifi
  WiFi.begin(settings.wifiSSID, settings.wifiPass);
  WiFi.mode(WIFI_STA);

  unsigned long connectTime = millis() + settings.wifiTimeout;
  // Wait for WiFi to connect
  while (isNotConnected() && !timeout)
  {
    // delay to yield to esp core functions
    delay(10);

    // update timeout
    timeout = millis() > connectTime;

    // handle buttons and LED
    buttons.handle();
    statusLed.handle();
  }

  if (!timeout)
  {
    updateWifiSettings();
  }

  return timeout;
}

void Wifi::updateWifiSettings()
{
  // If DHCP, get the IPs
  if (settings.dhcp)
  {
    if (isAp)
    {
      settings.ip = WiFi.softAPIP();
      settings.gateway = settings.ip;
    }
    else
    {
      settings.ip = WiFi.localIP();
      settings.gateway = WiFi.gatewayIP();
    }
    settings.subnet = WiFi.subnetMask();
  }

  // compute broadcast address
  computeBroadcast();
}

void Wifi::setupConfigAp(IPAddress ap_ip, IPAddress subnet)
{
  WiFi.hostname(settings.nodeName);
  WiFi.softAPConfig(ap_ip, ap_ip, subnet);

  isAp = true;
}

void Wifi::standAlone(String ssid, String password, IPAddress ap_ip, IPAddress subnet)
{
  setupConfigAp(ap_ip, subnet);
  // start softAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  // update settings
  delay(100);
  updateWifiSettings();
}

/**
 * Configures and activates the Access Point (AP) mode for Wi-Fi.
 *
 * @param def Use default hotspot configuration if true, otherwise use settings configuration.
 */
bool Wifi::accessPoint(bool def)
{
  String ssid, password;
  uint32_t ap_ip, subnet;

#ifdef USE_WEBSERVER
  if (def)
  {
    // use hotspot config
    ssid = HOTSPOT_SSID;
    password = HOTSPOT_PASSWORD;
    ap_ip = IPAddress(HOTSPOT_IP);
    subnet = IPAddress(HOTSPOT_SUBNET);
  }
  else
#endif
  {
    // use settings config
    ssid = settings.wifiSSID;
    password = settings.wifiPass;
    ap_ip = settings.dhcp ? IPAddress(DEFAULT_IP) : IPAddress(settings.ip);
    subnet = settings.dhcp ? IPAddress(DEFAULT_SUBNET) : IPAddress(settings.subnet);
  }

  standAlone(ssid, password, ap_ip, subnet);

#ifdef USE_DNS
  dnsServer.start(DNS_PORT, "*", ap_ip);
#endif

  return true;
}

/**
 *  Calculates and sets the broadcast address using the IP and subnet
 */
void Wifi::computeBroadcast()
{
  broadcast_ip = ~settings.subnet | (settings.ip & settings.subnet);
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

bool Wifi::isNotConnected()
{
  return WiFi.status() != WL_CONNECTED;
}

#ifdef USE_DNS
void Wifi::handleDns()
{
  if (isAp)
    dnsServer.processNextRequest();
};
#endif
