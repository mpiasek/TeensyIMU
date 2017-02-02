// Produces orientation and accleration information in the world frame using the MPU-6050
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Modified by Lewis Cooper 01/07/2017
//
// Changelog:
//      2013-05-08 - added seamless Fastwire support
//                 - added note about gyro calibration
//      2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//      2012-06-20 - improved FIFO overflow handling and simplified read process
//      2012-06-19 - completely rearranged DMP initialization code and simplification
//      2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//      2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//      2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                 - add 3D math helper file to DMP6 example sketch
//                 - add Euler output and Yaw/Pitch/Roll output formats
//      2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//      2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//      2012-05-30 - basic DMP initialization working
//      2017-01-07 - Changed to accomadate project strap-on

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

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
===============================================
*/
#define INTERRUPT_PIN_IMU_ONE 2
#define INTERRUPT_PIN_IMU_TWO 3

#include "IMU.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

IMU imuOne;
IMU imuTwo;

void setup() {

    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
    // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.
    Serial.begin(38400);
    
    pinMode(INTERRUPT_PIN_IMU_ONE, INPUT);
    pinMode(INTERRUPT_PIN_IMU_TWO, INPUT);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_IMU_ONE), ImuOneDataReady, RISING);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_IMU_TWO), ImuTwoDataReady, RISING);
    imuOne.Initialize(0x68, -3348, 9999, 210, 220, 76, -85);
    imuTwo.Initialize(0x69, -3348, 9999, 806, 220, 76, -85);
}

void loop() { 
    
    /*if(imuOne.Process())
    {
        Serial.print("Info1: ");
        Serial.print(imuOne.GetWorldYaw() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuOne.GetWorldPitch() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuOne.GetWorldRoll() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelX());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelY());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelZ());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion0());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion1());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion2());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion3());
        Serial.print(" ");
        Serial.println(imuOne.GetDeltaT()); 
   }
   
   if(imuTwo.Process())
    {
        Serial.print("Info2: ");
        Serial.print(imuTwo.GetWorldYaw() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldPitch() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldRoll() * 180/M_PI);
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldAccelX());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldAccelY());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldAccelZ());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldQuaternion0());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldQuaternion1());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldQuaternion2());
        Serial.print(" ");
        Serial.print(imuTwo.GetWorldQuaternion3());
        Serial.print(" ");
        Serial.println(imuTwo.GetDeltaT()); 
   }*/

   if(imuOne.Process())
   {
      if(imuTwo.Process())
      {
         /*Serial.print(imuOne.GetWorldPitch() * 180/M_PI);
         Serial.print(" ");
         Serial.println(imuTwo.GetWorldPitch() * 180/M_PI);*/
         Serial.print(imuOne.GetGravityX());
         Serial.print(" ");
         Serial.print(imuOne.GetGravityY());
         Serial.print(" ");
         Serial.print(imuOne.GetGravityZ());
         Serial.print(" ");
         Serial.print(imuTwo.GetGravityX());
         Serial.print(" ");
         Serial.print(imuTwo.GetGravityY());
         Serial.print(" ");
         Serial.println(imuTwo.GetGravityZ());
      }
   }
}

void ImuOneDataReady() {
    imuOne.SetInterrupt(true);
}

void ImuTwoDataReady() {
    imuTwo.SetInterrupt(true);
}
