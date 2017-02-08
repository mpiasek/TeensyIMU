#include "IMU.h"

IMU::IMU(void)
{
    _address = 0x68; //default
    _deltat = 0;
    _Before = 0;
    _mpuInterrupt = false;
}

int IMU::Initialize(int address, int accelOffsetX, int accelOffsetY, int accelOffsetZ, int gyroOffsetX, int gyroOffsetY, int gyroOffsetZ) 
{
    _address = address;
    
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    _mpu.initialize(_address);
    // load and configure the DMP
    _devStatus = _mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    _mpu.setXGyroOffset(gyroOffsetX);
    _mpu.setYGyroOffset(gyroOffsetY);
    _mpu.setZGyroOffset(gyroOffsetZ);
    //The idea is to get the worldAccel readings to be 0 at rest or atleast close. 
    //Examine raw acceleration values when the device is on a flat surface with the
    //z-axis pointing upwards. Ideally with +/-2g sensitivity you should get 0 0 8192.
    if(accelOffsetX != 9999) _mpu.setXAccelOffset(accelOffsetX);
    if(accelOffsetY != 9999) _mpu.setYAccelOffset(accelOffsetY);
    if(accelOffsetZ != 9999) _mpu.setZAccelOffset(accelOffsetZ);
    
    if (_devStatus == 0) {
        _mpu.setDMPEnabled(true);  
        _mpuIntStatus = _mpu.getIntStatus();

        // get expected DMP packet size for later comparison
        _packetSize = _mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        while(1);
    }
}

int IMU::Process()
{
    unsigned long Now = 0;  

    if (_mpuInterrupt) {
        // reset interrupt flag and get INT_STATUS byte
        _mpuInterrupt = false;
        _mpuIntStatus = _mpu.getIntStatus();
    
        // get current FIFO count
        _fifoCount = _mpu.getFIFOCount();
    
        // check for overflow (this should never happen unless our code is too inefficient)
        if ((_mpuIntStatus & 0x10) || _fifoCount == 1024) {
            // reset so we can continue cleanly
            _mpu.resetFIFO();
    
        // otherwise, check for DMP data ready interrupt (this should happen frequently)
        } 
        else if (_mpuIntStatus & 0x02) 
        {
            // wait for correct available data length, should be a VERY short wait
            while (_fifoCount < _packetSize) _fifoCount = _mpu.getFIFOCount();
    
            // read a packet from FIFO
            _mpu.getFIFOBytes(_fifoBuffer, _packetSize);
            
            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            _fifoCount -= _packetSize;
    
            _mpu.dmpGetQuaternion(&_q, _fifoBuffer);
            _mpu.dmpGetAccel(&_aa, _fifoBuffer);
            _mpu.dmpGetGravity(&_gravity, &_q);
            _mpu.dmpGetYawPitchRoll(_ypr, &_q, &_gravity);
            _mpu.dmpGetLinearAccel(&_aaReal, &_aa, &_gravity);
            _mpu.dmpGetLinearAccelInWorld(&_aaWorld, &_aaReal, &_q);
            
            //Get the change in time
            Now = millis();
            _deltat = (float) (Now - _Before);
            _Before = Now; 

            return 1;
        }
    }

    return 0;
}

float IMU::GetWorldQuaternion0()
{
    return _q.w;
}

float IMU::GetWorldQuaternion1()
{
    return _q.x;
}

float IMU::GetWorldQuaternion2()
{
    return _q.y;
}

float IMU::GetWorldQuaternion3()
{
    return _q.z;
}

float IMU::GetWorldAccelX()
{
    return _aaWorld.x;
}

float IMU::GetWorldAccelY()
{
    return _aaWorld.y;
}

float IMU::GetWorldAccelZ()
{
    return _aaWorld.z;
}

float IMU::GetGravityX()
{
    return _gravity.x;
}

float IMU::GetGravityY()
{
    return _gravity.y;
}

float IMU::GetGravityZ()
{
    return _gravity.z;
}
        
float IMU::GetWorldYaw()
{
    return _ypr[0];
}

float IMU::GetWorldPitch()
{
    return _ypr[1];
}

float IMU::GetWorldRoll()
{
    return _ypr[2];
}

float IMU::GetAccelX()
{
    return _aa.x;
}

float IMU::GetAccelY()
{
    return _aa.y;
}

float IMU::GetAccelZ()
{
    return _aa.z;
}

float IMU::GetDeltaT()
{
    return _deltat;
}
        
void IMU::SetInterrupt(bool value)
{
    _mpuInterrupt = value;
}

bool IMU::GetInterrupt()
{
    return _mpuInterrupt;
}

