#include "dmx.h"

#include "settings.h"
#include "espDMX.h"

Dmx dmx;

// Init DMX communication
void Dmx::start()
{
    dmxA.begin();
    dmxB.begin();
}
// Pause and unpause DMX transmission
void Dmx::pause()
{
    dmxA.pause();
    dmxB.pause();
}
void Dmx::unPause()
{
    dmxA.unPause();
    dmxB.unPause();
}
// End DMX communication
void Dmx::end()
{
    dmxA.end();
    dmxB.end();
}

bool Dmx::sendTo(uint16_t universe, uint16_t length, uint8_t *data)
{
    if (universe == settings.artNetUniA)
    {
        dmxA.setChValues(data, length);
    }
    else if (universe == settings.artNetUniB)
    {
        dmxB.setChValues(data, length);
    }
    else
    {
        return false;
    }
    return true;
}