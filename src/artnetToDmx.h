#ifndef __ARTNET_H__
#define __ARTNET_H__

#include <ArtnetWifi.h>

class ArtnetToDmx
{
private:
  ArtnetWifi artnet;
  static void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);
public:
  void start();
  void read();
};

extern ArtnetToDmx artnetToDmx;


#endif