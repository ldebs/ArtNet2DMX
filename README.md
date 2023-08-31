# ArtNet2DMX

## Overview

ArtNet2DMX is a versatile ESP8266-based project that functions as an Art-Net to DMX gateway. It enables you to wirelessly control DMX-based lighting fixtures using the Art-Net protocol over a WiFi network. With support for two configurable universes via the ESP's dual UARTs, this project is suitable for a wide range of lighting control applications.

## Features

- Converts Art-Net control data to DMX signals.
- Supports two configurable DMX universes.
- Easily configure the device via a web server.
- Designed for ESP8266-based boards, including ESP-01M (ESP8285).
- Comes with default settings for quick setup.
- Access configuration through an initial connection to the device's access point.

## Prerequisites

Before getting started, make sure you have the following:

- An ESP8266-based development board (e.g., NodeMCU, Wemos D1 Mini).
- A MAX485 Module and AMS1117 and some LEDs and resitors (see my [OSHW Lab](https://oshwlab.com/ldebat/artnet2dmx) project for wiring)
- Access to a WiFi network.
- Art-Net-compatible software or controller for sending lighting control data.
- Basic knowledge of the DMX lighting control protocol.
- PlatformIO for uploading firmware to the ESP8266.

## Installation

1. Clone or download this repository to your development environment.
2. Open the project in PlatformIO.
3. Customize the settings in the `lib/Settings/settings.h` file to match your setup.
4. Upload the firmware to your ESP8266 using PlatformIO.

## Initial Configuration

1. Power on the ESP8266.
2. Connect to the ArtNet2DMX access point (e.g., SSID: ArtNet2DMX-XXXXX).
3. Open a web browser and navigate to `http://192.168.42.1`.
4. Configure your WiFi network settings and other parameters.
5. Save the changes, and the ESP8266 will restart and connect to your WiFi network.

## Usage

1. Ensure the ESP8266 is connected to your WiFi network.
2. Start your Art-Net software or controller.
3. Configure the Art-Net universe to match your ArtNet2DMX settings.
4. Send Art-Net control data to the ESP8266's IP address.

The ArtNet2DMX gateway will receive Art-Net data and convert it into DMX signals to control your lighting fixtures.

## Customization

All configuration settings, including pin assignments, access point names, timeouts, and LED control defaults, are located in the `settings.h` file. Customize these settings to meet your specific requirements.

## License

[![Creative Commons License](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)
  
**ArtNet2DMX** by [ldebs](https://www.ldebs.org/artnet2dmx) is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).

Permissions beyond the scope of this license may be available at [https://www.ldebs.org/artnet2dmx](https://www.ldebs.org/artnet2dmx).

## Acknowledgments

- This project is based on the work of Matthew Tong and the [ESP8266_ArtNetNode_DMX](https://github.com/mtongnz/ESP8266_ArtNetNode_DMX) project and Robert Oostenveld and the [esp8266_artnet_dmx512](https://github.com/robertoostenveld/esp8266_artnet_dmx512) project. Many thanks to Matthew and Robert for their work.
- The ESP8266 community for their excellent work in developing the ESP8266 platform.
- The Art-Net protocol developers for enabling versatile lighting control.

Feel free to contribute to this project or report any issues you encounter. Enjoy wireless lighting control with ArtNet2DMX!

Feel free to [buy me a coffee](https://www.buymeacoffee.com/ldebs) ^^
