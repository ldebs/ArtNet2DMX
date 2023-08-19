#include "store.h"

#include "settings.h"
#include "espDMX.h"
#include "webServer.h"

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
  byte* dmxDataA = dmxA.getChValues();
  byte* dmxDataB = dmxB.getChValues();

  
  // Get last channel with non-zero value
  uint16_t numChans = dmxA.getNbCh();
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
    
  yield();
  
  // Get last channel with non-zero value
  numChans = dmxB.getNbCh();
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
  for (int x = 0; x < numChans; x++) {
    f.write(dmxDataB[x]);

  }
  
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

  // get numChans for universe A
  uint16_t numChans = f.readStringUntil(':').toInt();

  byte savedDMX[numChans];

  // Get DMX values from file
  for (int x = 0; x < numChans; x++)
    savedDMX[x] = f.read();

  // Send channel data to DMX output
  yield();
  dmxA.clearChValues();
  yield();
  dmxA.setChValues(savedDMX, numChans);
  yield();
  
  // Get numChans for universe B
  Serial.println(f.readStringUntil(':'));
  numChans = f.readStringUntil(':').toInt();
  
  byte savedDMXB[numChans];

  // Get DMX values from file
  for (int x = 0; x < numChans; x++) {
    savedDMXB[x] = f.read();
  }

  // Send channel data to DMX output
  yield();
  dmxB.clearChValues();
  yield();
  dmxB.setChValues(savedDMXB, numChans);
  yield();

  // Close file
  f.close();

  webServer.outputScene = true;
  webServer.outputSceneNum = sceneNum;
  
  return true;
}

void Store::scenesClear(){
  dmxA.clearChValues();
  dmxB.clearChValues();
  webServer.outputScene = false;
}
