/*The MIT License (MIT)

Copyright (c) 2014 Nathanaël Lécaudé
https://github.com/natcl/Artnet, http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811

Copyright (c) 2016,2019 Stephan Ruloff
https://github.com/rstephan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <ArtnetWifi.h>


const char ArtnetWifi::artnetId[] = ART_NET_ID;

ArtnetWifi::ArtnetWifi() : artDmxCallback(nullptr) {}

void ArtnetWifi::begin(String hostname)
{
  Udp.begin(ART_NET_PORT);
  host = hostname;
  sequence = 1;
  physical = 0;
}

uint16_t ArtnetWifi::read(void)
{
  packetSize = Udp.parsePacket();

  if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
  {
    DEBUG("A"+String(packetSize));
      senderIp =  Udp.remoteIP();
      int nbRead = Udp.read(artnetPacket, MAX_BUFFER_ARTNET);
      if((nbRead-=8)<0) return 0; // not enougth bytes read

      // Check that packetID is "Art-Net" else ignore
      if (memcmp(artnetPacket, artnetId, sizeof(artnetId)) != 0) {
        return 0;
      }

      if((nbRead-=2)<0) return 0; // not enougth bytes read
      opcode = artnetPacket[8] | artnetPacket[9] << 8;

      if (opcode == ART_DMX)
      {
        // protocol version
        if((nbRead-=2)<0) return 0; // not enougth bytes read
        uint16_t protVer = artnetPacket[10] << 8 | artnetPacket[11];
        if(protVer<14) return 0; // bad protocol version

        if((nbRead-=1)<0) return 0; // not enougth bytes read
        sequence = artnetPacket[12];

        if((nbRead-=1)<0) return 0; // not enougth bytes read
        physical = artnetPacket[13];

        if((nbRead-=2)<0) return 0; // not enougth bytes read
        uint8_t subUni = artnetPacket[14];
        uint8_t net = artnetPacket[15];
        incomingUniverse = net << 8 | subUni;

        if((nbRead-=2)<0) return 0; // not enougth bytes read
        dmxDataLength = artnetPacket[16] << 8 | artnetPacket[17];
        if(dmxDataLength<2 || dmxDataLength>512) return 0; // bad value
        /* ignored
        if(dmxDataLength%2 == 1) return 0; // should be even
        */
        if((nbRead-=dmxDataLength)<0) return 0; // not enougth bytes read
        
        DEBUG("Ac");

        if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START);
        if (artDmxFunc) {
          artDmxFunc(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START);
        }
        return ART_DMX;
      }
      if (opcode == ART_POLL)
      {
        return ART_POLL;
      }
      if (opcode == ART_SYNC)
      {
        return ART_SYNC;
      }
  }

  return 0;
}

uint16_t ArtnetWifi::makePacket(void)
{
  uint16_t len;
  uint16_t version;

  memcpy(artnetPacket, artnetId, sizeof(artnetId));
  opcode = ART_DMX;
  artnetPacket[8] = opcode;
  artnetPacket[9] = opcode >> 8;
  version = 14;
  artnetPacket[10] = version >> 8;
  artnetPacket[11] = version;
  artnetPacket[12] = sequence;
  sequence++;
  if (sequence == 0) {
    sequence = 1;
  }
  artnetPacket[13] = physical;
  artnetPacket[14] = outgoingUniverse;
  artnetPacket[15] = outgoingUniverse >> 8;
  len = dmxDataLength + (dmxDataLength % 2); // make a even number
  artnetPacket[16] = len >> 8;
  artnetPacket[17] = len;

  return len;
}

int ArtnetWifi::write(void)
{
  uint16_t len;

  len = makePacket();
  Udp.beginPacket(host.c_str(), ART_NET_PORT);
  Udp.write(artnetPacket, ART_DMX_START + len);

  return Udp.endPacket();
}

int ArtnetWifi::write(IPAddress ip)
{
  uint16_t len;

  len = makePacket();
  Udp.beginPacket(ip, ART_NET_PORT);
  Udp.write(artnetPacket, ART_DMX_START + len);

  return Udp.endPacket();
}

void ArtnetWifi::setByte(uint16_t pos, uint8_t value)
{
  if (pos > 512) {
    return;
  }
  artnetPacket[ART_DMX_START + pos] = value;
}

void ArtnetWifi::printPacketHeader(void)
{
  DEBUG("packet size = ");
  DEBUG(packetSize);
  DEBUG("\topcode = ");
  DEBUG(opcode, HEX);
  DEBUG("\tuniverse number = ");
  DEBUG(incomingUniverse);
  DEBUG("\tdata length = ");
  DEBUG(dmxDataLength);
  DEBUG("\tsequence n0. = ");
  DEBUGLN(sequence);
}

void ArtnetWifi::printPacketContent(void)
{
  for (uint16_t i = ART_DMX_START ; i < dmxDataLength ; i++){
    DEBUG(artnetPacket[i], DEC);
    DEBUG("  ");
  }
  DEBUGLN('\n');
}
