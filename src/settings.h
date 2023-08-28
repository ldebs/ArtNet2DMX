#ifndef __SETTING_H__
#define __SETTING_H__

#include <IPAddress.h>
#include <user_interface.h>

#define FIRMWARE_VERSION "v2.0.0"

// Features
// #define USE_DNS
// #define USE_WEBSERVER

#define MAX_STR_SIZE 32

// Wifi
// /!\ size of strings should be < MAX_STR_SIZE
#define DEFAULT_NODE "ArtNet2DMX"
#define DEFAULT_SSID "ArtNet2DMX"
#define DEFAULT_PASSWORD "ArtNet2DMX"
#ifdef USE_WEBSERVER
#define HOTSPOT_SSID ("ArtNet2DMX" + '_' + String(ESP.getChipId(), 16))
#define HOTSPOT_PASSWORD "ArtNet2DMX"
#define HOTSPOT_IP 192, 168, 42, 1
#define HOTSPOT_SUBNET 255, 255, 255, 0
#endif
#define TIMEOUT_WAIT_CLIENT 60000
#define TIMEOUT_END_CLIENT 120000
#define CONNECT_TIMEOUT 30000
#define DEFAULT_IP 192, 168, 42, 1
#define DEFAULT_GW 192, 168, 42, 1
#define DEFAULT_SUBNET 255, 255, 255, 0
#define DEFAULT_DHCP true;
#define DEFAULT_STANDALONE false;

// ArtNet
#define DEFAULT_ARTNETUNIA 0;
#define DEFAULT_ARTNETUNIB 1;
#define DEFAULT_ARTNETSUB 0;

// button 1
#define PIN_BUT1 5
#define BUT1_PUSHED LOW

// status leds
#define DEFAULT_BLINK_BPM 50
#define DEFAULT_LED_INTENSITY 15
#define PIN_LED_R 12
#define PIN_LED_G 14

// Helpers
#define BLINK_CONVERT(x) (60000. / (x * 8.))
#define INTENSITY_CONVERT(x) (255. * x / (510. - x))
#define INTENSITY_UNCONVERT(y) (510. * y / (y + 255.))
#define BIN4(nibble) String((nibble)&0x8 ? 1 : 0) + ((nibble)&0x4 ? 1 : 0) + ((nibble)&0x2 ? 1 : 0) + ((nibble)&0x1 ? 1 : 0)
#define BIN8(byte) BIN4((byte) >> 4) + BIN4(byte)
#define BIN16(short) BIN8((short) >> 8) + " " + BIN8((short))
#define HEX8(byte) String((byte) >> 4, 16) + String((byte)&0xf, 16)
#define HEX16(short) HEX8((short) >> 8) + " " + HEX8((short)&0xFF)

class Settings
{
  inline void setToDefault()
  {
    strcpy(nodeName, DEFAULT_NODE);
    strcpy(wifiSSID, DEFAULT_SSID);
    strcpy(wifiPass, DEFAULT_PASSWORD);
    ip = IPAddress(DEFAULT_IP);
    gateway = IPAddress(DEFAULT_GW);
    subnet = IPAddress(DEFAULT_SUBNET);
    wifiTimeout = CONNECT_TIMEOUT;
    dhcp = DEFAULT_DHCP;
    standAlone = DEFAULT_STANDALONE;
    artNetUniA = DEFAULT_ARTNETUNIA;
    artNetUniB = DEFAULT_ARTNETUNIB;
    artNetSub = DEFAULT_ARTNETSUB;
    ledIntensity = DEFAULT_LED_INTENSITY;
    blinkTimeoutEighth = BLINK_CONVERT(DEFAULT_BLINK_BPM);
    loaded = false;
  }

  // transient fields
  bool loaded;

public:
  // saved fields
  char nodeName[MAX_STR_SIZE];
  char wifiSSID[MAX_STR_SIZE];
  char wifiPass[MAX_STR_SIZE];
  uint32_t ip;
  uint32_t gateway;
  uint32_t subnet;
  uint16_t wifiTimeout;
  bool dhcp;
  bool standAlone;
  uint8_t artNetUniA;
  uint8_t artNetUniB;
  uint8_t artNetSub;
  uint8_t ledIntensity;
  uint8_t blinkTimeoutEighth;

  inline Settings() { setToDefault(); }
  void setup();
  bool save();
  uint8_t load();
  void resetOrRestore();
};
extern Settings settings;

#endif
