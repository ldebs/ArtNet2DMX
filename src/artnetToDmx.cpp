#include "artnetToDmx.h"

#include "inout.h"
#include "dmx.h"

#include <functional>
using namespace std::placeholders;

ArtnetToDmx artnetToDmx;
uint8_t ArtnetToDmx::lastSequence;

void ArtnetToDmx::start()
{
    lastSequence = 0;
    artnet.begin(settings.nodeName);
    artnet.setArtDmxCallback(ArtnetToDmx::onDmxFrame);
}

void ArtnetToDmx::read()
{
    artnet.read();
}

void ArtnetToDmx::onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
    DEBUG(String("Ao(")+String(universe)+","+String(length)+","+String(sequence)+")");
    // deal with sequence
    int16_t diffSeq = ((int16_t)sequence) - ((int16_t)lastSequence);
    bool noBigGap = (diffSeq < 0 ? -diffSeq : diffSeq) < 128;
    // if packet is before last one
    if ((noBigGap && diffSeq < 0) || (!noBigGap && diffSeq > 0))
    {
        // ignore packet
        statusLed.set(FROM_ARTNET);
        return;
    }
    lastSequence = sequence;

    // if packet can be sent to one of the handled universes
    if (dmx.setData(universe, length, data))
    {
        // dmx sent ok
        statusLed.set(TO_DMX);
        return;
    }

    // ignore packet
    statusLed.set(FROM_ARTNET);
}
