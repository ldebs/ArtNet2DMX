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
  DEBUG_BEGIN();
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
  dmx.setup();

  // Start listening for ArtNet UDP packets
  artnetToDmx.start();
}

void loop()
{
  DEBUG("L{");
  // Check Wifi conection
  if (wifi.hasToReconnect())
  {
    DEBUG("Lw");
    dmx.pause();
    // Restart wifi
    wifi.start();
    dmx.unPause();
  }

// handle DNS requests
#ifdef USE_DNS
  DEBUG("Ld");
  wifi.handleDns();
#endif

// Handle web requests
#ifdef USE_WEBSERVER
  DEBUG("Ls");
  webServer.handleClient();
#endif

  // handle led
  DEBUG("Ll");
  statusLed.handle();

  // handle buttons
  DEBUG("Lb");
  buttons.handle();
  
  // call the ArtNet read function
  DEBUG("La");
  artnetToDmx.read();

  DEBUG("Lx");
  dmx.handleDmx();

  DEBUGLN("}L");
}

void restart()
{
  dmx.pause();

  statusLed.set(RESTARTING, true);
  statusLed.set(RESTARTING, true);
  statusLed.set(RESTARTING, true);

  ESP.restart();
}