/*
espDMX v2 library
Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/espDMX

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/

#ifndef espDMX_h
#define espDMX_h

#include <inttypes.h>
#include "Stream.h"
#include <HardwareSerial.h>

#define DMX_TX_CONF SERIAL_8N2 // SERIAL_8N2
#define DMX_TX_BAUD 250000
#define DMX_FULL_UNI_TIMING 1000 // How often to output full 512 channel universe (in milliseconds)

// DMX states
enum dmx_state
{
    DMX_STOP,     // DMX transmission is stopped
    DMX_START,    // DMX transmission is starting
    DMX_FULL_UNI, // DMX transmission of full 512 channel universe
    DMX_DATA,     // DMX transmission of data channels
    DMX_NOT_INIT  // DMX is not initialized
};

void ICACHE_RAM_ATTR espdmx_interrupt_handler();

// Class declaration for espDMX
class espDMX
{
private:
    /** DMX universe number associated with the struct instance */
    uint8_t universe;
    /** Pin number for DMX transmission */
    uint8_t txPin;
    /** Current state of DMX transmission (dmx_state enum) */
    uint8_t state = DMX_NOT_INIT;
    /** Number of DMX channels to be transmitted */
    uint16_t nbCh;
    /** Current channel being transmitted */
    uint16_t curCh;
    /** DMX data buffer for channel values */
    uint8_t chValues[512];
    /** Time of the last full universe transmission */
    unsigned long lastTime;

    // Friend function to handle DMX interrupt
    friend void espdmx_interrupt_handler();

    void transmit();
    void enableInterrupt();
    void armInterrupt();
    void disarmInterrupt();
    void setBaudrate(int baud_rate);
    void clearBuffer();
    void flush();
    void init();
    void uninit();

public:
    // Constructor for espDMX class
    espDMX(uint8_t universe);

    // Begin DMX communication
    void begin();

    // Pause and unpause DMX transmission
    void pause();
    void unPause();

    // End DMX communication
    void end();

    // Set DMX channel values in the DMX data buffer
    void setChValues(uint8_t *newChValues, uint16_t newNbCh);

    // Clear all DMX channel values in the DMX data buffer
    void clearChValues();

    // Get a pointer to the DMX data buffer
    uint8_t *getChValues();

    // Get the number of DMX channels currently set
    uint16_t getNbCh();
};

// External instances for two DMX universes
extern espDMX dmxA;
extern espDMX dmxB;

#endif
