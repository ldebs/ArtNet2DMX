#include "artnetToDmx.h"

#include "inout.h"
#include "dmx.h"

#include <functional>
using namespace std::placeholders;

ArtnetToDmx artnetToDmx;

void ArtnetToDmx::start()
{
    artnet.begin(settings.nodeName);
    artnet.setArtDmxCallback(ArtnetToDmx::onDmxFrame);
}

void ArtnetToDmx::read()
{
    artnet.read();
}

void ArtnetToDmx::onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
    // TODO deal with sequence number
    if (dmx.sendTo(universe, length, data))
    {
        statusLed.set(TO_DMX);
    }
    else
    {
        statusLed.set(FROM_ARTNET);
    }
}
