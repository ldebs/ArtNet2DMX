#include "store.h"

#include "dmx.h"

#ifdef USE_WEBSERVER
#include "webServer.h"
#endif

#include <Arduino.h>
#include <inttypes.h>
#include <LittleFS.h>

Store store;


bool Store::newScene(char *sceneName){
  // Open scene list for reading & appending
  File f = LittleFS.open("/scenes.txt", "a+");
  if (!f)
    return 0;
  
  uint16_t sceneNum = 0;
  
  // Read line by line from the file.  We only need the last number
  while(f.available()) {
    sceneNum = f.readStringUntil(':').toInt();
    f.readStringUntil('\n');
  }

  sceneNum++;

  // Write the scene number and name to our scene list file
  f.print(sceneNum);
  f.print(":");
  f.println(sceneName);
  f.close();

  // Close scene list file
  f.close();

  // Save values.  Return result
  return sceneSave(sceneNum);
}

bool Store::sceneSave(uint16_t sceneNum){
  // Open scene list for writing - clears file first
  File f = LittleFS.open("/" + String(sceneNum) + ".save", "w");
  if (!f)
    return 0;

  // Get pointers to our DMX data buffers
#ifndef DEBUG_SERIAL
  byte* dmxDataA = dmx.getDmx(true).dmxData;
#endif
#ifdef USE_DMXB
  byte* dmxDataB = dmx.getDmx(false).dmxData;
#endif

  uint16_t numChans;
#ifndef DEBUG_SERIAL
  // Get last channel with non-zero value
  numChans = dmx.getDmx(true).len;
  while (numChans > 0) {
    if (dmxDataA[numChans] != 0)
      break;
    numChans--;
  }
  numChans++;

  // Start file with number of channels in universe A
  f.print(numChans);
  f.print(":");
  
  // Write dmxA channel values
  for (int x = 0; x < numChans; x++)
    f.write(dmxDataA[x]);
#endif
  
#ifdef USE_DMXB
  // Get last channel with non-zero value
  numChans = dmx.getDmx(false).len;
  while (numChans > 0) {
    if (dmxDataB[numChans] != 0)
      break;
    numChans--;
  }
  numChans++;

  // Print number of channels in universe B
  f.print(":");
  f.print(numChans);
  f.print(":");
  
  // Write dmxB channel values
  for (int x = 0; x < numChans; x++)
    f.write(dmxDataB[x]);
#endif
  
  // Close file
  f.close();
  
  return true;
}

bool Store::sceneDelete(uint16_t sceneNum){
  File a = LittleFS.open("/scenes.tmp", "w");
  File f = LittleFS.open("/scenes.txt", "r");
  
  // Copy each line to tmp file - skip our deleted one
  while(f.available()) {
    uint16_t num = f.readStringUntil(':').toInt();
    String tmp = f.readStringUntil('\n');

    if (num != sceneNum) {
      a.print(num);
      a.print(":");
      a.println(tmp);
    }
  }
  
  a.close();
  f.close();

  // Delete save scene and scenes.txt then rename scenes.tmp
  if (LittleFS.remove("/" + String(sceneNum) + ".save")) {
    if (LittleFS.remove("/scenes.txt")) {
      if (LittleFS.rename("/scenes.tmp", "/scenes.txt"))
        return true;
    }
  }

  // Return 0 if failed
  return false;
}

bool Store::sceneLoad(uint16_t sceneNum){
  // Open scene list for reading
  File f = LittleFS.open("/" + String(sceneNum) + ".save", "r");
  if (!f)
    return 0;

  uint16_t numChans;

#ifndef DEBUG_SERIAL
  // get numChans for universe A
  numChans = f.readStringUntil(':').toInt();
  byte savedDMX[numChans];

  // Get DMX values from file
  for (int x = 0; x < numChans; x++)
    savedDMX[x] = f.read();

  // Send channel data to DMX output
  dmx.getDmx(true).setData(numChans, savedDMX);
#endif
  
#ifdef USE_DMXB
  // Get numChans for universe B
  numChans = f.readStringUntil(':').toInt();
  
  byte savedDMXB[numChans];

  // Get DMX values from file
  for (int x = 0; x < numChans; x++) {
    savedDMXB[x] = f.read();
  }

  // Send channel data to DMX output
  dmx.getDmx(false).setData(numChans, savedDMXB);
#endif

  // Close file
  f.close();

#ifdef USE_WEBSERVER
  webServer.outputScene = true;
  webServer.outputSceneNum = sceneNum;
#endif
  
  return true;
}

void Store::scenesClear(){
#ifndef DEBUG_SERIAL
  dmx.getDmx(true).clear();
#endif
#ifdef USE_DMXB
  dmx.getDmx(false).clear();
#endif
#ifdef USE_WEBSERVER
  webServer.outputScene = false;
#endif
}
