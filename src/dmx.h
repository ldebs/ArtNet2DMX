#ifndef __DMX_H__
#define __DMX_H__

#include "settings.h"

#include <Arduino.h>

#define MAX_DMX_CH 512

class Dmx
{
private:
    class DmxSerial
    {
        friend class Dmx;
        friend class Store;

    private:
        uint8_t dmxData[MAX_DMX_CH];
        uint16_t len;
        HardwareSerial *serial;

        void setup(bool isDmxA);
        void setData(uint16_t length, uint8_t *data);
        void send();
        void clear();
    };

    bool onHold;
#ifndef DEBUG_SERIAL
    DmxSerial dmxA;
#endif
#ifdef USE_DMXB
    DmxSerial dmxB;
#endif

public:
    // Init DMX communication
    void setup();
    // Pause and unpause DMX transmission
    void pause();
    void unPause();
    // send data to universe
    bool setData(uint16_t universe, uint16_t length, uint8_t *data);
    inline DmxSerial &getDmx(bool isDmxA)
    {
        return
#ifndef DEBUG_SERIAL
#ifdef USE_DMXB
            isDmxA ? dmxA :
#else
            dmxA
#endif
#endif
#ifdef USE_DMXB
                   dmxB
#endif
            ;
    }

    inline void handleDmx()
    {
        if (!onHold)
        {
#ifndef DEBUG_SERIAL
            dmxA.send();
#endif
#ifdef USE_DMXB
            dmxB.send();
#endif
        }
    }
};
extern Dmx dmx;

#endif
