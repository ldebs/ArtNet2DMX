#ifndef __DMX_H__
#define __DMX_H__

#include "settings.h"

#include <Arduino.h>

class Dmx
{
private:
    bool started = false;

public:
    // Init DMX communication
    void start();
    // Pause and unpause DMX transmission
    void pause();
    void unPause();
    // End DMX communication
    void end();
    // send data to universe
    bool sendTo(uint16_t universe, uint16_t length, uint8_t *data);
};
extern Dmx dmx;

#endif
