# Project Strap-On

Project Strap-On is built to work with the [HTC Vive](https://www.vive.com/ca/). It is a completely open sourse alternative to the [SteamVR Tracking HDK](https://www.triadsemi.com/product/steamvr-tracking-hdk/). It aims to extend the tracking capabilities of a Vive.

This repository contains helpful information for building your own Strap-On. The current design of the Strap-On is a device that you can attach to your wrist to add some more tracking capability. It implements the [Vive Lighthouse system](http://gizmodo.com/this-is-how-valve-s-amazing-lighthouse-tracking-technol-1705356768) as well as an [IMU system](http://doc-ok.org/?p=1478) to provide positional information through serial. It is currently designed to output `x,y,z` positional data relative to the Lighthouse, so really only requires a Lighthouse, not a whole HTC Vive.

## Required Items

1. [HTC Vive Lighthouse](https://www.vive.com/us/accessory/base-station/)
2. [Teensy 3.2](https://www.pjrc.com/store/teensy32.html)
3. Several [TS3633-CM1 modules](https://www.triadsemi.com/product/ts3633-cm1/)*
4. An IMU (our code works with the MPU6050)
5. Various electronics components including a soldering iron, solder, wires, wire cutters, resistors, headers pins (both male and female)
6. Micro USB-B Cable (the common usb cable used to charge Android phones)

\* Or build your own using the cheaper [TS3633 only modules](https://www.triadsemi.com/product/ts3633/)

**Note:** This design can be extended to work with any shape object, but in order to build the one we have here you will also need:

1. Access to a 3D printer
2. Printed versions of the PCB boards under the `PCB/` directory <TODO: add PCB directory>




## How to Upload Arduino Code to a Teensy
* This code was written for the [**Teensy 3.2**](https://www.pjrc.com/store/teensy32.html)
* There is a Teensy plug-in for the Arduino IDE called [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
* Follow the instruction in the above link to download and install the Teensyduino
* If you don't have the Arduino IDE, you can download it [here](https://www.arduino.cc/en/main/software) 

## Notes
- Remember to **CHECK THE PINS** for Photodiodes and for IMUs
- Remember that the IMU SCL and SDA pins need to connect to Teensy pins 19 and 18

