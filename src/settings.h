#ifndef __SETTING_H__
#define __SETTING_H__

#include <IPAddress.h>
#include <user_interface.h>

#define FIRMWARE_VERSION "v2.0.0"

// DNS
#define USE_DNS

// Wifi
#define TIMEOUT_WAIT_CLIENT 60000
#define TIMEOUT_END_CLIENT 120000
#define CONNECT_TIMEOUT 30000

// ArtNet
#define DEFAULT_NODE "ArtNet2DMX"
#define DEFAULT_SSID "ArtNet2DMX"
#define DEFAULT_PASSWORD "ArtNet2DMX"

// button 1
#define PIN_BUT1 5
#define BUT1_PUSHED LOW

// dmx
#define PIN_D1 1
#define PIN_D2 2

// status leds
#define DEFAULT_BLINK_BPM 60
#define DEFAULT_LED_INTENSITY 20
#define PIN_LED_R 12
#define PIN_LED_G 14

// Helpers
#define BLINK_CONVERT(x) (60000. / (x * 8.))
#define INTENSITY_CONVERT(x) (255. * x / (510. - x))
#define INTENSITY_UNCONVERT(y) (510. * y / (y + 255.))
#define BIN4(nibble) String((nibble)&0x8?1:0) + ((nibble)&0x4?1:0) + ((nibble)&0x2?1:0) + ((nibble)&0x1?1:0)
#define BIN8(byte) BIN4((byte) >> 4) + BIN4(byte)
#define BIN16(short) BIN8((short) >> 8) + " " + BIN8((short))
#define HEX8(byte) String((byte) >> 4, 16) + String((byte) & 0xf, 16)
#define HEX16(short) HEX8((short) >> 8) + " " + HEX8((short) & 0xFF)

class Settings
{
public:
  char nodeName[32] = DEFAULT_NODE;
  char wifiSSID[32] = DEFAULT_SSID;
  char wifiPass[32] = DEFAULT_PASSWORD;
  uint32_t ip = IPAddress(192, 168, 42, 1);
  uint32_t gw = IPAddress(192, 168, 42, 1);
  uint32_t broadcast_ip = IPAddress(192, 168, 42, 255);
  uint32_t subnet = IPAddress(255, 255, 255, 0);
  uint16_t hotSpotDelay = CONNECT_TIMEOUT;
  bool dhcp = true;
  bool standAlone = false;
  uint8_t artNetUniA = 0;
  uint8_t artNetUniB = 1;
  uint8_t artNetSub = 0;
  uint8_t ledIntensity = DEFAULT_LED_INTENSITY;
  uint8_t blinkTimeoutEighth = BLINK_CONVERT(DEFAULT_BLINK_BPM);

  void setup();
  bool save();
  uint8_t load();
  void resetOrRestore();
};
extern Settings settings;

#endif
