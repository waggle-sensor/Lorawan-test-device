# Lorawan-test-device
This repo shows hardware and software configurations to build a device used to test Lorawan on WSN or rpi_lorawan WSN modules.
---

## Table of Contents
1. [Features](#features)
1. [Hardware Needed](#hardware-needed)
1. [Assembling The Device](#assembling-the-device)
1. [Installing Software](#installing-software)
1. [Connect MKR WAN 1310 to Computer](#connect-mkr-wan-1310-to-computer)
1. [Running The Software](#running-the-software)
1. [Testing Device Using Chirpstack](#testing-device-using-chirpstack)
1. [Future Work](#future-work)

>TODO: Add section about getting the deveui of the mkrwan. Use a setup.ino file that prints deveui?

## Features

- Waits to perform the test until a button is pressed.
- Sends a join request to lorawan network then a packet. If both are succesful shows a pass in the display.
- Shows a fail in the display if join request or packet sending are not succesful.
- Shows a loading animation in the display while the test is being performed.
- You can use the device to test multiple gateways by ensuring that the Lorawan Network servers (Chirpstack) are configured with the same [App key](./main/arduino_secrets.h) and `deveui` as the device.
- Compatible with our [lorawan testing script](https://github.com/waggle-sensor/surya-tools/tree/main?tab=readme-ov-file#lorawan-test-script-overview).

## Hardware Needed

- Micro USB Wire (other end must be able to connect to your computer)
- [Arduino MKR WAN 1310](https://store-usa.arduino.cc/products/arduino-mkr-wan-1310?selectedStore=us)
- [Male/Male Jumper Wires](https://www.amazon.com/Solderless-Multicolored-Electronic-Breadboard-Protoboard/dp/B09FP517VM)
- [Breadboard](https://www.adafruit.com/product/65)
- [OLED Display](https://www.amazon.com/UCTRONICS-SSD1306-Self-Luminous-Display-Raspberry/dp/B072Q2X2LL)

>TODO: Add button you end up using

## Assembling The Device

<img src='./images/wiring.jpg' alt='wiring' height='200'>

<img src='./images/setup.jpg' alt='wiring' height='200'>

>TODO: include instructions on how to build it

## Installing Software

1. To connect to the `MKR WAN 1310` board, you will need to install the [Arduino IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)
1. Once you installed the IDE, you need to further install the board's software support by following the [SAMD21 core for MKR boards Documentation](https://docs.arduino.cc/software/ide-v1/tutorials/getting-started/cores/arduino-samd)

   <img src='./images/software_download.jpeg' alt='software download' height='200'>
1. You will also need the library for mkrwan. Under Library Manager, look up "mkrwan" and install `MKRWAN by Arduino`.

   <img src='./images/mkrwan_lib_download.jpeg' alt='mkrwan library download' height='400'>
    
    >NOTE: At the time of configuring the board `MKRWAN_v2` was not used because of bug issues related to the library.

1. Two Arduino libraries must be installed to start using the display. The SSD1306 driver library is used to initialize the display and provide low level display functions. The GFX library provides graphics functions for displaying text, drawing lines and circles, etc. Both these libraries are available from Adafruit.

    > NOTE: if these two libraries are not found in the library manager, you can also follow this [tutorial](https://startingelectronics.org/tutorials/arduino/modules/OLED-128x64-I2C-display/) 

1. Install the SSD1306 Driver Library. Under Library Manager, look up "Adafruit SSD1306" and install `Adafruit SSD1306 by Adafruit`.

   <img src='./images/SSD1306_lib_download.png' alt='SSD1306 library download' height='400'>

1. Install the GFX Library. Under Library Manager, look up "adafruit gfx library" and install `Adafruit GFX Library by Adafruit`.

   <img src='./images/gfx_lib_download.png' alt='GFX library download' height='400'>

1. An additional library must be installed to get "i2c header", look up "Adafruit BusIO library" and install `Adafruit BusIO by Adafruit`.

   <img src='./images/busio_lib_download.png' alt='BusIO library download' height='400'>

## Connect MKR WAN 1310 to Computer

1. Connect the board to your computer with the Micro USB wire
   - You should see a green light glow on the board

1. Go to Tools in Arduino IDE and select `Board` then select `Arduino SAMD Boards (32-bits ARM Cortex-M0+)` for the type of board. Finally select `Arduino MKR WAN 1310` for the board as shown: 

    <img src='./images/tools_board.jpeg' alt='Tools/Board' height='300'>

1. Then select the correct serial port for the arduino as shown:

    <img src='./images/tools_port.jpeg' alt='Tools/Board' height='200'>

    >NOTE: If Arduino MKR WAN 1310 is not showing up, pressing the "RST" (reset) button twice in quick succession will put the board in bootloader mode. Instead of running a sketch the Arduino will wait until a sketch is uploaded helping the board to show up in your Arduino IDE. 

## Running the Software
>TODO: finish this section, and delete the example

1. Retrieve your MKR WAN 1310's DevEUI by using `setup.ino` in your `Arduino IDE`. The serial monitor will display your DevEUI. When you run `setup.ino` you should see a orange light glow on the board

    <img src='./images/DevEUI.jpeg' alt='DevEUI' height='50'>

    >NOTE: The program will fail because the device hasn't been given an App key

1. Using the LoRaWAN Network Server’s portal or API, add your device with the option to use `OTAA` set to yes.

1. Once you've added your device, generate an application key.

1. Provide the application key to `arduino_secrets.h`

1. Run `setup.ino` in your `Arduino IDE`, if the device connects successfully the serial monitor will display the values the device is sending and your Network Server will receive a `join request` then the device's values.

    <img src='./images/values_sent.jpeg' alt='values sent' height='100'>

    >NOTE: `main.ino` and `setup.ino` is using `US915` as its LoraWAN region. This must be changed in both files for different countries.

1. Finally for your Network Server to decript the uplink packets, use its portal or API to provide the codec: `codec.js`.

>NOTE: Once you are done debugging the device, you can comment out the serial commands such as print().

## Testing Device Using Chirpstack
>TODO: finish this section, and delete the example

[Chirpstack](https://www.chirpstack.io/) was used as our Network Server and a `RAK Discover Kit 2` was used as our Gateway.

1. Retrieve your MKR WAN 1310's DevEUI by using `main.ino` in your `Arduino IDE`. The serial monitor will display your DevEUI.

    <img src='./images/DevEUI.jpeg' alt='DevEUI' height='50'>

    >NOTE: The program will fail because the device hasn't been given an App key

1. Using Chirpstack's UI, add your device using `OTAA` following [Chirpstack's Documentation](https://www.chirpstack.io/docs/guides/connect-device.html)

1. Once you've added your device, generate an application key or create one via the 'OTAA Keys' tab.

    <img src='./images/app_key.png' alt='App key' height='400'>

1. Provide the application key to `arduino_secrets.h`

    >NOTE: Chripstack does not use `APP EUI` when connecting devices via `OTAA` so this can be left as is.

1. Run `setup.ino` in your `Arduino IDE`, if the device connects successfully the serial monitor will display the values the device is sending and chirpstack will receive a `join request` then the device's values.

    <img src='./images/values_sent.jpeg' alt='values sent' height='100'>

    <img src='./images/uplink_packet.png' alt='Uplink Packet' height='500'>

    >NOTE: The device might take a couple of minutes to join the LoRaWAN network.

1. Finally for chirpstack to decript the uplink packets, provide the `codec.js` via the Device Profile's Codec tab.

    <img src='./images/codec_tab.png' alt='Codec Tab' height='500'>

Viewing the uplink packets by clicking `up` in the device's events tab will now display the measurements and its values.

<img src='./images/decoded_packets.png' alt='Decoded Packets' height='700'>

# Future Work
- Configure to use LED if display is not detected.
- include instructions on how to connect the hardware