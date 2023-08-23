/*
espDMX v2 library
Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/espDMX

espDMX Derivative Library
Copyright (c) 2023, Laurent Debat
https://www.ldebs.org/artnet2dmx

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/

#include "espDMX.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Arduino.h>
#include <user_interface.h>

#define UART_TX_FIFO_SIZE 0x80

// Declaration of two DMX instances
espDMX dmxA(0);
espDMX dmxB(1);

// Function prototypes
static void uart_ignore_char(char c);

// Interrupt handler for DMX interrupts
void ICACHE_RAM_ATTR espdmx_interrupt_handler()
{
    if (U0IS & (1 << UIFE))
    {                       // Check status flag for UART0
        U0IC = (1 << UIFE); // Clear status flag
        dmxA.transmit();    // Call interrupt function for UART0
    }

    if (U1IS & (1 << UIFE))
    {                       // Check status flag for UART1
        U1IC = (1 << UIFE); // Clear status flag
        dmxB.transmit();    // Call interrupt function for UART1
    }
}

// Transmit DMX data
void espDMX::transmit()
{
    // Check if DMX is initialized or valid
    if (state == DMX_NOT_INIT)
    {
        return;
    }

    // Check the current state for transmission
    if (state == DMX_DATA || state == DMX_FULL_UNI)
    {
        // Calculate the number of channels remaining to transmit
        uint16_t remainingCh = ((state == DMX_FULL_UNI) ? 512 : nbCh) - curCh;

        // Check if there are channels remaining to transmit
        if (remainingCh > 0)
        {
            // Calculate the available FIFO room for transmission
            uint16_t fifoSize = UART_TX_FIFO_SIZE - ((USS(universe) >> USTXC) & 0xff);

            // Limit the transmission size based on available FIFO room
            remainingCh = (remainingCh > fifoSize) ? fifoSize : remainingCh;

            // Set the DMX data on uART FIFO
            for (; remainingCh; --remainingCh)
                USF(universe) = chValues[curCh++];

            return;
        }
        else
        {
            // No channels remaining, transition to DMX_START state
            state = DMX_START;
            return;
        }
    }

    if (state == DMX_START)
    {
        // Disarm the interrupt to avoid interference during transmission
        disarmInterrupt();

        // Wait for the UART TX FIFO to be empty
        while (((USS(universe) >> USTXC) & 0xff) != 0)
        {
            if(handleIO)
                handleIO();
            yield();
        }

        // Allow the last channel to be fully sent before BREAK
        delayMicroseconds(44);

        // Send BREAK signal (~120us low signal)
        pinMode(txPin, OUTPUT);
        digitalWrite(txPin, LOW);
        delayMicroseconds(115);

        // Reset the channel counter
        curCh = 0;

        // Output full universe (512 channels) periodically
        if (millis() > lastTime + DMX_FULL_UNI_TIMING)
        {
            lastTime = millis();
            state = DMX_FULL_UNI;
        }
        else
        {
            // Regular transmission of data channels
            state = DMX_DATA;
        }

        // MAB (Mark After Break) (~12us high signal)
        digitalWrite(txPin, HIGH);
        delayMicroseconds(7);

        // Configure UART for DMX data transmission
        pinMode(txPin, SPECIAL);
        setBaudrate(DMX_TX_BAUD);
        USC0(universe) = DMX_TX_CONF;

        // Empty the UART TX FIFO
        flush();

        // Send DMX Start Code 0
        USF(universe) = 0;

        // Re-arm the interrupt for further transmission
        armInterrupt();
    }
}

// Clear the transmit FIFO buffer
void espDMX::flush()
{
    if (state == DMX_NOT_INIT)
        return;

    uint32_t tmp = 0x00000000;
    tmp |= (1 << UCTXRST);

    // Clear TX FIFO
    USC0(universe) |= (tmp);
    USC0(universe) &= ~(tmp);
}

// Enable DMX interrupts
void espDMX::enableInterrupt()
{
    if (state == DMX_NOT_INIT)
        return;

    // Clear all 9 interrupt bits
    USIC(universe) = 0x1ff;

    // Set TX FIFO Empty trigger point
    uint32_t conf1 = 0x00000000;
    conf1 |= (0x00 << UCFET);
    USC1(universe) = conf1;

    // Attach our interrupt handler function
    ETS_UART_INTR_ATTACH(&espdmx_interrupt_handler, this);

    // Disable RX FIFO Full Interrupt
    USIE(universe) &= ~(1 << UIFF);

    ETS_UART_INTR_ENABLE();
}

// Arm DMX interrupts
void espDMX::armInterrupt()
{
    if (state == DMX_NOT_INIT)
        return;

    // Enable TX FIFO Empty Interrupt
    USIE(universe) |= (1 << UIFE);
}

// Disarm DMX interrupts
void espDMX::disarmInterrupt()
{
    if (state == DMX_NOT_INIT)
        return;

    USIE(universe) &= ~(1 << UIFE);
    // ETS_UART_INTR_DISABLE(); // Never disable IRQ completely, as it may be needed by other Serial Interface!
}

// Set the baud rate for DMX communication
void espDMX::setBaudrate(int baud_rate)
{
    if (state == DMX_NOT_INIT)
        return;

    USD(universe) = (ESP8266_CLOCK / baud_rate);
}

// Clear the DMX data buffer
void espDMX::clearBuffer()
{
    for (int i = 0; i < 512; i++)
        chValues[i] = 0;

    nbCh = 0;
}

// Initialize a DMX instance
void espDMX::init()
{

    system_set_os_print(0);
    ets_install_putc1(&uart_ignore_char);

    // Initialize variables
    txPin = (universe == 0) ? 1 : 2;
    state = DMX_STOP;
    curCh = 0;
    lastTime = 0;

    // Initialize empty DMX buffer
    clearBuffer();

    // TX output set to idle
    pinMode(txPin, OUTPUT);
    digitalWrite(txPin, HIGH);

    enableInterrupt();
}

// Deinitialize a DMX instance
void espDMX::uninit()
{
    if (state == DMX_NOT_INIT)
        return;

    disarmInterrupt();

    pinMode(txPin, OUTPUT);
    digitalWrite(txPin, HIGH);

    state = DMX_NOT_INIT;
}

// Set DMX channel values in the DMX data buffer
void espDMX::setChValues(uint8_t *newChValues, uint16_t newNbCh)
{
    if (state == DMX_NOT_INIT)
        return;

    // set the new number of channel
    if (newNbCh > 512)
        newNbCh = 512;
    nbCh = newNbCh;

    // Put data into our buffer
    memcpy(chValues, newChValues, newNbCh);

    if (state == DMX_STOP)
    {
        state = DMX_START;
        transmit();
    }
}

// Constructor for espDMX class
espDMX::espDMX(uint8_t universe) : universe(universe)
{
}

// Initialize the DMX instance and delay for stabilization
void espDMX::begin(void (*_handleIO)())
{
    handleIO = _handleIO;
    if (state == DMX_NOT_INIT)
    {
        init();
        delay(5);
    }
    else
    {
        state = DMX_START;
        transmit();
    }
}

// Pause DMX transmission
void espDMX::pause()
{
    disarmInterrupt();
}

// Unpause DMX transmission
void espDMX::unPause()
{
    if (state == DMX_NOT_INIT)
        return;

    state = DMX_START;
    transmit();
}

// Deinitialize the DMX instance
void espDMX::end()
{
    uninit();
}

// Clear all DMX channel values in the DMX data buffer
void espDMX::clearChValues()
{
    if (state == DMX_NOT_INIT)
        return;

    clearBuffer();
}

// Get a pointer to the DMX data buffer
uint8_t *espDMX::getChValues()
{
    if (state == DMX_NOT_INIT)
        return 0;

    return chValues;
}

// Get the number of DMX channels currently set
uint16_t espDMX::getNbCh()
{
    if (state == DMX_NOT_INIT)
        return 0;

    return nbCh;
}

// Helper function to ignore UART characters
static void uart_ignore_char(char c)
{
    return;
}
