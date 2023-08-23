#ifndef __WIFI_H__
#define __WIFI_H__

#include "settings.h"

#include <Arduino.h>
#include <IPAddress.h>
#include <inttypes.h>
#include <DNSServer.h>

class Wifi
{
private:
  uint8_t MAC_array[6];
  String MAC_address;
  bool isAp = false;

#ifdef USE_DNS
  DNSServer dnsServer;
#endif

  bool connect();
  void setupConfigSta();
  bool accessPoint(bool def = false);
  void standAlone(String ssid, String password, IPAddress ap_ip, IPAddress subnet);
  void setupConfigAp(IPAddress ap_ip, IPAddress subnet);
  void updateWifiSettings();

public:
  void computeBroadcast();
  void start();
  String getMac();
  bool isNotConnected();
#ifdef USE_DNS
  void handleDns();
#endif

  inline bool hasToReconnect()
  {
    return !isAp && isNotConnected();
  }
  uint32_t broadcast_ip;
};
extern Wifi wifi;

#endif
