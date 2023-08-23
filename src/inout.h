#ifndef __INOUT_H__
#define __INOUT_H__

#include "settings.h"

#include <Arduino.h>

#define L_RG(RPattern, GPattern) ((RPattern << 8) | GPattern)
#define L_RL(pat, nbit) (((pat << nbit) & 0xFF) | (pat >> (8 - nbit)))
#define L_RR(pat, nbit) ((pat >> nbit) | ((pat << (8 - nbit)) & 0xFF))
#define L_ON /****/ 0b11111111
#define L_OFF /***/ 0b00000000
#define L_1L /****/ 0b11111110
#define L_1M /****/ 0b11110000
#define L_1S /****/ 0b10000000
#define L_2LL /***/ 0b11101110
#define L_2MM /***/ 0b11001100
#define L_2LS /***/ 0b11101000
#define L_2SL /***/ 0b10001110
#define L_2SS /***/ 0b10001000
#define L_2SS_ /**/ 0b10100000
#define L_3SSS /**/ 0b10101000
#define L_3LSS /**/ 0b11101010
#define L_3LLS /**/ 0b11011010
#define L_4S /****/ 0b10101010

typedef enum
{
  // workflow
  OFF = /*******************/ L_RG(L_OFF, L_OFF),
  STARTING = /**************/ L_RG(L_ON, L_ON),
  RESTARTING = /************/ L_RG(L_1L, L_1L),
  EEPROM_SAVING = /*********/ L_RG(L_2LL, L_2SS),
  RESET = /*****************/ L_RG(L_1S, L_1S),
  RESET_OR_RESTORE = /******/ L_RG(L_1S, L_4S),
  // Wifi
  WIFI_HOTSPOT = /**********/ L_RG(L_1M, L_OFF),
  WIFI_WAIT_CLIENT = /******/ L_RG(L_1M, L_RL(L_1M, 4)),
  WIFI_HANDLE_CLIENT = /****/ L_RG(L_1M, L_RL(L_2SS_, 4)),
  WIFI_CONNECTING = /*******/ L_RG(L_OFF, L_2SS_),
  // transmission
  FROM_ARTNET = /***********/ L_RG(L_4S, L_4S),
  TO_DMX = /****************/ L_RG(L_4S, L_RL(L_4S, 1)),
  // SUCCESS : red led always off
  SUCCESS = /***************/ L_RG(L_OFF, L_ON),
  SUCCESS_SETTINGS = /******/ L_RG(L_OFF, L_2LL),
  SUCCESS_CONNECTED = /*****/ L_RG(L_OFF, L_1S),
  SUCCESS_OTA = /***********/ L_RG(L_OFF, L_3SSS),
  // ERROR : red led always on
  ERROR_SEND = /************/ L_RG(L_ON, L_4S),
  ERROR_OTA_NO_SPACE = /****/ L_RG(L_ON, L_1L),
  ERROR_OTA_FILE_WRITE = /**/ L_RG(L_ON, L_2LL),
  ERROR_OTA_FAIL = /********/ L_RG(L_ON, L_3LLS),
  ERROR_SETTINGS_WRITE = /**/ L_RG(L_ON, L_2SS),
  ERROR_NO_SETTINGS = /*****/ L_RG(L_ON, L_3SSS),
  ERROR_SETTINGS_READ = /***/ L_RG(L_ON, L_2SS_),
  ERROR_UNKNOWN = /*********/ L_RG(L_ON, L_OFF),
} Status;

class StatusLed
{
private:
  uint8_t current = 0;
  Status status;
  unsigned long nextBlinkTime;
  bool hasToBlink(bool force = false);
  void write();

public:
  void setup();
  void set(Status newStatus, bool force = false);
  Status get();
  void handle(bool force = false);
};

class Buttons{
private:
  unsigned long but1 = 0;
  void up();
  void on();
  void down();
public:
  void setup();
  void handle();
};

extern StatusLed statusLed;
extern Buttons buttons;

#ifdef USE_WEBSERVER
extern String globalinfo;
#endif

#endif
