#ifndef __WIFI_H__
#define __WIFI_H__

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
  DNSServer dnsServer;

public:
  IPAddress ap_ip = IPAddress(2, 0, 0, 10);
  bool isHotSpot = false;

  void setBroadcastAddr();
  void hotSpot(bool def = false);
  void start();
  String getMac();
  bool notConnected();
  void handleDns();
};
extern Wifi wifi;

#endif
