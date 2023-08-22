#include "settings.h"

#include "inout.h"

#include <Arduino.h>
#include <EEPROM.h>

Settings settings;

void Settings::setup()
{
  // status and settings
  EEPROM.begin(
      /*status*/ 2 +

      /*nodeName*/ 32 +
      /*wifiSSID*/ 32 +
      /*wifiPass*/ 32 +
      /*ip*/ 4 +
      /*gw*/ 4 +
      /*broadcast_ip*/ 4 +
      /*subnet*/ 4 +
      /*hotSpotDelay*/ 2 +
      /*dhcp*/ 1 +
      /*standAlone*/ 1 +
      /*artNetUniA*/ 1 +
      /*artNetUniB*/ 1 +
      /*artNetSub*/ 1 +
      /*ledIntensity*/ 1 +
      /*blinkTimeoutEighth*/ 1 +

      /*control*/ 1);
}

#define __SAVE(field, size)                        \
  addr = ((uint8_t *)&(field));                    \
  for (int i = 0; i < size; i++, eeAddr++, addr++) \
  EEPROM.write(eeAddr, *addr), control ^= *addr

bool Settings::save()
{
  statusLed.set(EEPROM_SAVING);

  // write settings
  uint8_t *addr = NULL;
  uint eeAddr = 0;
  uint8_t control = 0;

  char status[2] = {'O', 'K'};
  __SAVE(status, 2);
  // fields
  __SAVE(nodeName, 32);
  __SAVE(wifiSSID, 32);
  __SAVE(wifiPass, 32);
  __SAVE(ip, 4);
  __SAVE(gw, 4);
  __SAVE(broadcast_ip, 4);
  __SAVE(subnet, 4);
  __SAVE(hotSpotDelay, 2);
  __SAVE(dhcp, 1);
  __SAVE(standAlone, 1);
  __SAVE(artNetUniA, 1);
  __SAVE(artNetUniB, 1);
  __SAVE(artNetSub, 1);
  __SAVE(ledIntensity, 1);
  __SAVE(blinkTimeoutEighth, 1);
  // control
  uint8_t computedControl = control;
  __SAVE(control, 1);

  EEPROM.commit();

  // check
  uint8_t controlR = load();

  if (computedControl != controlR)
  {
    globalinfo += " we:" + HEX8(computedControl) + " != " + HEX8(controlR);
    statusLed.set(ERROR_SETTINGS_WRITE);
    return false;
  }

  // done
  statusLed.set(SUCCESS_SETTINGS);
  return true;
}

#define __LOAD(field, size)                        \
  addr = ((uint8_t *)&(field));                    \
  for (int i = 0; i < size; i++, eeAddr++, addr++) \
  *addr = EEPROM.read(eeAddr), control ^= *addr

uint8_t Settings::load()
{
  uint8_t *addr = NULL;
  uint eeAddr = 0;
  uint8_t control = 0;

  // check if settings have already been saved (status = OK)
  char status[2] = {'?', '?'};
  __LOAD(status, 2);
  if (status[0] != 'O' || status[1] != 'K')
  {
    statusLed.set(ERROR_NO_SETTINGS);
    return control;
  }

  // read settings
  __LOAD(nodeName, 32);
  __LOAD(wifiSSID, 32);
  __LOAD(wifiPass, 32);
  __LOAD(ip, 4);
  __LOAD(gw, 4);
  __LOAD(broadcast_ip, 4);
  __LOAD(subnet, 4);
  __LOAD(hotSpotDelay, 2);
  __LOAD(dhcp, 1);
  __LOAD(standAlone, 1);
  __LOAD(artNetUniA, 1);
  __LOAD(artNetUniB, 1);
  __LOAD(artNetSub, 1);
  __LOAD(ledIntensity, 1);
  __LOAD(blinkTimeoutEighth, 1);

  uint8_t controlR = 0;
  uint8_t computedControl = control;
  __LOAD(controlR, 1);

  if (computedControl != controlR)
  {
    globalinfo += " re:" + HEX8(computedControl) + " != " + HEX8(controlR);
    statusLed.set(ERROR_SETTINGS_READ);
    return computedControl;
  }

  // done
  statusLed.set(SUCCESS_SETTINGS);
  return computedControl;
}

void Settings::resetOrRestore()
{
  uint8_t *addr = NULL;
  uint eeAddr = 0;
  uint8_t control = 0;

  // reset
  char status[2] = {'?', '?'};
  __LOAD(status, 2);
  eeAddr = 0;
  if (status[0] == 'O' && status[1] == 'K')
  {
    // set settings status as not saved
    status[0] = 'X';
    status[1] = 'X';
    __SAVE(status, 2);
    EEPROM.commit();
    statusLed.set(SUCCESS_SETTINGS, true);
  }
  // restore
  else if (status[0] == 'X' && status[1] == 'X')
  {
    // set settings status as OK
    status[0] = 'O';
    status[1] = 'K';
    __SAVE(status, 2);
    EEPROM.commit();
    statusLed.set(SUCCESS_SETTINGS, true);
  }
  // cannot restore because settings have never been saved
  else
  {
    statusLed.set(ERROR_SETTINGS_WRITE);
  }
}