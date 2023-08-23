#include "main.h"

#include "inout.h"
#include "wifi.h"
#include "artnetToDmx.h"
#include "dmx.h"

#ifdef USE_WEBSERVER
#include "webServer.h"
#endif

#include <Arduino.h>

void setup()
{
  settings.setup();
  buttons.setup();
  statusLed.setup();
  
  settings.load();
  wifi.start();

// Start WebServer
#ifdef USE_WEBSERVER
  webServer.start();
#endif

  // Start DMX
  dmx.start();

  // Start listening for ArtNet UDP packets
  artnetToDmx.start();
}

void loop()
{
  // Check Wifi conection
  if (wifi.hasToReconnect())
  {
    dmx.pause();
    // Restart wifi
    wifi.start();
    dmx.unPause();
  }

// handle DNS requests
#ifdef USE_DNS
  wifi.handleDns();
#endif

// Handle web requests
#ifdef USE_WEBSERVER
  webServer.handleClient();
#endif

  // handle led
  statusLed.handle();

  // handle buttons
  buttons.handle();
  
  // call the ArtNet read function
  artnetToDmx.read();
}

void restart()
{
  dmx.end();

  statusLed.set(RESTARTING, true);
  statusLed.set(RESTARTING, true);
  statusLed.set(RESTARTING, true);

  ESP.restart();
}