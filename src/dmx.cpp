#include "dmx.h"

#include "inout.h"

Dmx dmx;

#define DATA_BAUD_RATE 250000
#define BREAK_BAUD_RATE 90000

void Dmx::DmxSerial::setup(bool isDmxA)
{
#ifndef DEBUG_SERIAL
    if (isDmxA)
        serial = &Serial;
#ifdef USE_DMXB
    else
#endif
#endif
#ifdef USE_DMXB
        serial = &Serial1;
#endif

    serial->begin(DATA_BAUD_RATE, SERIAL_8N2);

    clear();
}

void Dmx::DmxSerial::clear()
{
    for (uint16_t i = 0; i < MAX_DMX_CH; i++)
        dmxData[i] = 0;
    len = 0;
}

void Dmx::DmxSerial::setData(uint16_t length, uint8_t *data)
{
    memcpy(dmxData, data, length);
    len = length;
}

void Dmx::DmxSerial::send()
{
    if (len > 0)
    {
        // switch to another baud rate, see https://forum.arduino.cc/index.php?topic=382040.0
        serial->flush();
        serial->begin(BREAK_BAUD_RATE, SERIAL_8N2);
        while (serial->available())
            serial->read();
        // send the break as a "slow" byte
        serial->write(0);
        // switch back to the original baud rate
        serial->flush();
        serial->begin(DATA_BAUD_RATE, SERIAL_8N2);
        while (serial->available())
            serial->read();

        serial->write(0); // Start-Byte
        // send out the value of the selected channels (up to 512)
        for (uint16_t i = 0; i < len; i++)
        {
            serial->write(dmxData[i]);
        }
    }
}

// Init DMX communication
void Dmx::setup()
{
#ifndef DEBUG_SERIAL
    dmxA.setup(true);
#endif
#ifdef USE_DMXB
    dmxB.setup(false);
#endif
    onHold = false;
}
// Pause and unpause DMX transmission
void Dmx::pause()
{
    onHold = true;
}
void Dmx::unPause()
{
    onHold = false;
}

bool Dmx::setData(uint16_t universe, uint16_t length, uint8_t *data)
{
#ifndef DEBUG_SERIAL
    if (universe == settings.artNetUniA)
    {

        dmxA.setData(length, data);
    }
    else
#endif
#ifdef USE_DMXB
        if (universe == settings.artNetUniB)
    {
        dmxB.setData(length, data);
    }
    else
#endif
    {
        return false;
    }
    return true;
}
