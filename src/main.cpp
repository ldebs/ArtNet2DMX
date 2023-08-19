#include "settings.h"
#include "inout.h"
#include "wifi.h"
#include "webServer.h"
#include "artnetToDmx.h"
#include "dmx.h"

#include <Arduino.h>

void setup()
{
  settings.setup();
  buttons.setup();
  statusLed.setup();

  // If we cant load settings from EEPROM, start the hotspot
  settings.load();
  if (statusLed.get() == ERROR_NO_SETTINGS || statusLed.get() == ERROR_SETTINGS_READ)
    wifi.hotSpot(true);

  // Start hotspot if we're in standAlone mode otherwise we start the wifi
  if (settings.standAlone)
    wifi.hotSpot();
  else
  {
    wifi.start();
  }

  // Start WebServer
  webServer.start();

  // Start DMX
  dmx.start();

  // Start listening for ArtNet UDP packets
  artnetToDmx.start();
}

void loop()
{
  // Check WiFi conection
  if (!settings.standAlone && wifi.notConnected())
  {
    // Connect WiFi
    wifi.start();
  }

  // call the read function
  artnetToDmx.read();

  // handle DNS requests
  wifi.handleDns();

  // Handle web requests
  webServer.handleClient();

  // handle led
  statusLed.handle();

  // handle buttons
  buttons.handle();
}

void restart()
{
  dmx.end();

  statusLed.set(RESTARTING,true);
  statusLed.set(RESTARTING,true);
  statusLed.set(RESTARTING,true);

  ESP.restart();
}