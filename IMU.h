/*
Wrapper class for use of the MPU 6050 using Jeff Rowling's implementation
by: Lewis Cooper
date: January 7th, 2017

Info: This wrapper class will setup the IMU to return values at 100Hz using
an interrupt pin attached to the IMU's DMP to ping the micro controller when new
information is ready.

IMPORTANT: Can only support two IMUs. Choose between two addresses 0x68 and 0x69. 
The AD0 pin must be set low for 0x68 and high for 0x69.
*/

#include "MPU6050_6Axis_MotionApps20.h"

class IMU {
    public:
        IMU();
      
        int Initialize(int address, int accelOffsetX, int accelOffsetY, int accelOffsetZ, int gyroOffsetX, int gyroOffsetY, int gyroOffsetZ); //Provide the address and acceleration offsets (9999 means ignore)
        //The idea is to get the worldAccel readings to be 0 at rest or close (sensor noise). 
        //Examine RAW (GetAccel method) acceleration values when the device is on a flat surface with the
        //z-axis pointing upwards. Ideally with +/-2g sensitivity you should get 0 0 8192 plus noise.
        int Process(); //returns 1 when the result is new
        
        float GetWorldQuaternion0(); //returns Quaternion portion in the world frame
        float GetWorldQuaternion1(); //returns Quaternion portion in the world frame
        float GetWorldQuaternion2(); //returns Quaternion portion in the world frame
        float GetWorldQuaternion3(); //returns Quaternion portion in the world frame
        float GetWorldAccelX(); //returns Acceleration in X axis in the world frame
        float GetWorldAccelY(); //returns Acceleration in Y axis in the world frame
        float GetWorldAccelZ(); //returns Acceleration in Z axis in the world frame
        float GetWorldYaw(); //returns Yaw in world frame
        float GetWorldPitch(); //returns Pitch in the world frame
        float GetWorldRoll(); //returns Roll in the world frame

        float GetGravityX(); //returns X component of gravity in local frame
        float GetGravityY(); //returns Y component of gravity in local frame
        float GetGravityZ(); //returns Z component of gravity in local frame
        
        float GetAccelX(); //returns raw readings in LSB/g (+8192 per +g)
        float GetAccelY(); //returns raw readings in LSB/g
        float GetAccelZ(); //returns raw readings in LSB/g
        
        float GetDeltaT(); //return in us
       
        bool GetInterrupt();
        void SetInterrupt(bool value);
        
    private:
        int _address;
        MPU6050 _mpu;
        
        // MPU control/status vars
        unsigned char _mpuIntStatus;   // holds actual interrupt status byte from MPU
        unsigned char _devStatus;      // return status after each device operation (0 = success, !0 = error)
        unsigned int _packetSize;    // expected DMP packet size (default is 42 bytes)
        unsigned int _fifoCount;     // count of all bytes currently in FIFO
        unsigned char _fifoBuffer[64]; // FIFO storage buffer

        // orientation/motion vars
        Quaternion _q;           // [w, x, y, z]         quaternion container
        VectorInt16 _aa;         // [x, y, z]            accel sensor measurements
        VectorInt16 _aaReal;     // [x, y, z]            gravity-free accel sensor measurements
        VectorInt16 _aaWorld;    // [x, y, z]            world-frame accel sensor measurements
        VectorFloat _gravity;    // [x, y, z]            gravity vector
        float _ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

        unsigned long _deltat;
        unsigned long _Before;
        
        volatile bool _mpuInterrupt;
};
