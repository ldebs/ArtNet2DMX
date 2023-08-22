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
  bool allowHotSpot = true;
  String MAC_address;

  #ifdef USE_DNS
  DNSServer dnsServer;
  #endif

public:
  IPAddress ap_ip = IPAddress(2, 0, 0, 10);
  bool isHotSpot = false;

  void setBroadcastAddr();
  void hotSpot(bool def = false);
  void start();
  String getMac();
  bool notConnected();
  #ifdef USE_DNS
  void handleDns();
  #endif
};
extern Wifi wifi;

#endif
