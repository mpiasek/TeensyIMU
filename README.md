# TeensyLighthouseIMU

This repository contains all the information needed to build your on Strap-On for your Vive. A Strap-On is a device that you can attach to your wrist to add some more tracking capability. It implements the Vive Lighthouse system as well as an IMU system to provide positional information through serial. It is currently designed to output `x,y,z` positional data relative to the Lighthouse, so theoretically only requires a Lighthouse, not a whole HTC Vive.

There a several items required to build your own:

1. [HTC Vive Lighthouse](https://www.vive.com/us/accessory/base-station/)
2. [Teensy 3.2](https://www.pjrc.com/store/teensy32.html)
3. Several [TS3633-CM1 modules](https://www.triadsemi.com/product/ts3633-cm1/)*
4. IMU <TODO: specify model>
5. Various electronics components including a soldering iron, solder, wires, wire cutters, resistors, headers pins (both male and female)
6. Micro USB-B Cable (a common one used to charge Android phones)

\* Or build your own using the cheaper [TS3633 only modules](https://www.triadsemi.com/product/ts3633/)

This design can be extended to work with any shape object, but in order to build the one we have here you will also need:

1. Access to a 3D printer
2. Printed versions of the PCB boards under the PCB/ directory <TODO: add PCB directory>




## How to Upload Arduino Code to a Teensy
* This code was written for the [**Teensy 3.2**](https://www.pjrc.com/store/teensy32.html)
* There is a Teensy plug-in for the Arduino IDE called [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
* Follow the instruction in the above link to download and install the Teensyduino
* If you don't have the Arduino IDE, you can download it [here](https://www.arduino.cc/en/main/software) 

## Notes
- Remember to **CHECK THE PINS** for Photodiodes and for IMUs
- Remember that the IMU SCL and SDA pins need to connect to Teensy pins 19 and 18

