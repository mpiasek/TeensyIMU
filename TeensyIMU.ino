
// *********************
// ***** INCLUDES ******
// *********************

// Sensor includes:
#include "SysTickImpl.h"

// IMU includes:
#include "IMU.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif


// *********************
// ****** DEFINES ******
// *********************

// Sensor defines:
#define DISPLAY_ANGLES

#define sensor0 19
#define sensor1 9
#define sensor2 9
#define sensor3 9
#define sensor4 9
#define sensor5 9
#define sensor6 9
#define sensor7 9
#define sensor8 9
#define sensor9 9

// RINGBUFF_LEN must be a power of 2 because of optimzations that replace modulo with bitwise AND
#define RINGBUFF_LEN 0x100

#define RINGBUFF_MODULO_MASK (RINGBUFF_LEN -1)
// For example, the two following lines are equivalent:
//  Val % RINGBUFF_LEN
//  Val & RINGBUFF_MODULO_MASK

#define TICKS_PER_US 96

// the high order bit is used to indicate if the entry is a rising or falling edge.
#define RISING_EDGE 0x00000000
#define FALLING_EDGE 0x80000000

#define MILLIS_MULTIPLIER 1
#define MAX_COUNTER (MILLIS_MULTIPLIER * 9600000)

#define MAX_RECEIVERS 10

// IMU defines:
#define INTERRUPT_PIN_IMU_ONE 8
#define INTERRUPT_PIN_IMU_TWO 9

// USB or Bluetooth
const bool BLUETOOTH = false;

// ***********************
// * CLASSES AND STRUCTS *
// ***********************

// Sensor classes and structs:
class RingBuff
{
  public:
  short readerPos; // last place read from
  short writerPos; // next place to write
  // note: if readerPos == writerPos, that is the state we hit when
  // the ring buffer is full.  We do this so that the check for a full
  // ring buffer is as simple as possible in the interrupt routine
  // i.e. all it has to do is a quick comparison to see if it's safe to write.
  volatile unsigned int buff[RINGBUFF_LEN];

  RingBuff()
  {
    readerPos=0;
    writerPos=1;
  }
};

struct OotcPulseInfo
{
  int startTime;
  bool data;
  bool rotor;
  bool skip;

  OotcPulseInfo()
  {
    startTime=0;
    data=false;
    rotor=false;
    skip=false;
  }
};

class IrReceiver
{
  public:
  

  bool isAsserted;
  int lastRiseTime;
//  int lastDuration;

  float X,Y;

  IrReceiver()
  {
    X=0;
    Y=0;
    isAsserted = false;
    lastRiseTime=0;
  }  
  
};


// **********************
// ** GLOBAL VARIABLES **
// **********************

// Sensor global variables:

// this should be the same for all sensors.
int OotcPulseStartTime;

static RingBuff ringBuff;

OotcPulseInfo OotcInfo;

IrReceiver gReceiver[MAX_RECEIVERS];

SysTickImpl* SysTick = new SysTickImpl();

long int down=0, up=0;

// IMU global variables:
IMU imuOne;
IMU imuTwo;


// *********************
// ******* SETUP *******
// *********************

void setup() 
{
  Serial.begin(9600);

  // Pins used for sensors
  pinMode(sensor0, INPUT);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);
  pinMode(sensor5, INPUT);
  pinMode(sensor6, INPUT);
  pinMode(sensor7, INPUT);
  pinMode(sensor8, INPUT);
  pinMode(sensor9, INPUT);

  attachInterrupt(sensor0, ISR0, CHANGE);
  attachInterrupt(sensor1, ISR1, CHANGE);
  attachInterrupt(sensor2, ISR2, CHANGE);
  attachInterrupt(sensor3, ISR3, CHANGE);
  attachInterrupt(sensor4, ISR4, CHANGE);
  attachInterrupt(sensor5, ISR5, CHANGE);
  attachInterrupt(sensor6, ISR6, CHANGE);
  attachInterrupt(sensor7, ISR7, CHANGE);
  attachInterrupt(sensor8, ISR8, CHANGE);
  attachInterrupt(sensor9, ISR9, CHANGE);

  // BEWARE!!!  The following line changes the working of the Arduino's inner clock.
  // Specifically, it will make it run 100X slower.  So, if you call "delay(10)" you
  // will instead delay by a full second instead of 10 milliseconds.
  // This is needed in order to do the high precision timing needed to capture
  // the pulses from the lighthouse.  
  SysTick->SetLoad(9599999);
  
  // IMU
  pinMode(INTERRUPT_PIN_IMU_ONE, INPUT);
  pinMode(INTERRUPT_PIN_IMU_TWO, INPUT);
  
  attachInterrupt(INTERRUPT_PIN_IMU_ONE, ImuOneDataReady, RISING);
  attachInterrupt(INTERRUPT_PIN_IMU_TWO, ImuTwoDataReady, RISING);
  
  imuOne.Initialize(0x68, -3348, 9999, 806, 220, 76, -85);
  //imuTwo.Initialize(0x69, -1917, 9999, 1240, 220, 76, -85);
  // END IMU;
}


// *********************
// ******* LOOP ********
// *********************

void loop() 
{
  // Process Sensor Data
  ProcessRingBuff();
 
  // Process IMU Data
  if(imuOne.Process())
  {
        
        Serial.print("IMU ");
        Serial.print(imuOne.GetWorldQuaternion0());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion1());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion2());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldQuaternion3());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelX());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelY());
        Serial.print(" ");
        Serial.print(imuOne.GetWorldAccelZ());
        Serial.print(" ");
        Serial.println(imuOne.GetDeltaT());
        
  }
  // END IMU
}


// *********************
// ** PROCESS SENSORS **
// *********************

void ProcessRingBuff()
{
  while ((((ringBuff.writerPos - 1 - ringBuff.readerPos) + RINGBUFF_LEN) & RINGBUFF_MODULO_MASK) != 0)
  {
    ringBuff.readerPos = (ringBuff.readerPos+1) & RINGBUFF_MODULO_MASK;
    
    bool isFalling = false;

    // assign a reference.  No need to copy to a temp value.
    volatile unsigned int &rawVal = ringBuff.buff[ringBuff.readerPos];

    if (FALLING_EDGE & rawVal)
    {
      isFalling = true;
    }

    // this line limits us to 128 sensors.
    int sensor = (rawVal >>24) & 0x7F;

    unsigned int val = rawVal & 0x00FFFFFF;

    if (sensor >= MAX_RECEIVERS)
    {
      // this should NEVER happen.
      Serial.print("Invalid Sensor Number!!!");
      continue;
    }

    if (!isFalling)
    {
      // rising
      gReceiver[sensor].lastRiseTime = val;

      #define CLOCK_CYCLES_PER_ROTATION  1600000

      // the value below represents 170 degrees.  If we allow 180 degrees, we get hit with the
      // next sync pulse for the other laser and we erroniously consider it a laser sweep.
      #define MAX_CLOCK_CYCLES_PER_SWEEP (CLOCK_CYCLES_PER_ROTATION * 170 / 360)
      #define DEGREES_PER_ROTATION 360

      // Number of ticks since last flash
      int duration = (OotcInfo.startTime + MAX_COUNTER - val)%MAX_COUNTER;
      
      // if this looks like a pulse from a sweeping laser 
      if (duration < MAX_CLOCK_CYCLES_PER_SWEEP)
      {
        // hey, it looks like we see a laser sweeping.  Cool!

        //TODO: remove magic number. (Mila got by trial and error, but possible to calculate)
        float angle = duration * 0.000223143;

        if (OotcInfo.rotor)
        {
            gReceiver[sensor].Y = angle;      
        }
        else
        {
            gReceiver[sensor].X = angle;      
        }
      }
    }
    else
    {
      // falling
      int duration = (gReceiver[sensor].lastRiseTime + MAX_COUNTER - val) % MAX_COUNTER;  

      #define TICKS_PER_OOTC_CHANNEL 1000
      #define BASE_OOTC_TICKS 5500
      #define MAX_OOTC_TICKS (8 * TICKS_PER_OOTC_CHANNEL + BASE_OOTC_TICKS)

      // if this looks like an OOTC pulse...
      if (duration >= BASE_OOTC_TICKS && duration < MAX_OOTC_TICKS)
      {
        OotcInfo.startTime = gReceiver[sensor].lastRiseTime;
        
        // we have an OOTC pulse.  
        // for best accuracy, let's go figure out which sensor saw the beginning of the 
        // pulse first.  It's effectively random which interrupt would have fired first
        // so we want to find which one it was.  
        for (int i=0; i < MAX_RECEIVERS; i++)
        {
          int tempDuration = (gReceiver[i].lastRiseTime + MAX_COUNTER - val) % MAX_COUNTER;  

          if (tempDuration > MAX_OOTC_TICKS)
          {
            continue;
          }
          if (tempDuration < BASE_OOTC_TICKS)
          {
            // that's odd, wouldn't expect to see this very often
            // only case I can think of would be if all of the sensors have been unable
            // to see the OOTC pulses for a while, then you come back. i.e. leaving room and coming back.
            // and even then, this would only only happen rarely because you'd have to come back
            // very close to a multiple of MAX_COUNTER ticks.  Specifically, if you left
            // and came back, you'd only see this 100*(MAX_OOTC_TICKS - BASE_OOTC_TICKS)/MAX_COUNTER percent of the time.  That's rare.
            Serial.println("OOTC time underflow");
            continue;
          }
          // the longest duration will indicate the earliest start time.
          // this should be from whichever sensor triggered the first interrupt
          // that's the one we care about because it will be the most accurate.
          if (tempDuration > duration)
          {
            duration = tempDuration;
            OotcInfo.startTime = gReceiver[sensor].lastRiseTime;           
          }
        }

        // okay, now we have a quality OOTC start time and duration.  
        // first, let's poison the start times.  That way, we won't do the above check
        // again for the same OOTC pulse.  We'll poison the start times by makeing sure they're
        // showing as old enough that the next sensor that registers a pulse will see it being
        // too long/ old for the pulse to be an OOTC pulse, and it will throw it away.
        // since we only poison after the first falling edge is detected, we will ensure
        // that the first falling edge detected for the OOTC pulse from any sensor 
        // (which is going to be the most accurate time of the pulse) will be the only
        // falling edge we look at.  Combined with logic in the code above, this means
        // that for an OOTC pulse, we'll always look at the most accurate start time
        // received from any sensor, and the most accurate pulse end time received
        // from any sensor, even if they're from different sensors.

        // okay, let's poison.
        unsigned int poisonValue = (val + MAX_COUNTER - MAX_OOTC_TICKS - 10) % MAX_COUNTER; 
        //I think doing a "- 1" above would be sufficient, but doing a "-10" just to avoid any possible off-by-1 errors.

        for (int i=0; i < MAX_RECEIVERS; i++)
        {
          gReceiver[i].lastRiseTime = poisonValue;
        }        

        // Now we need to decode the OOTC pulse;
        unsigned int decodedPulseVal = (duration - BASE_OOTC_TICKS) / TICKS_PER_OOTC_CHANNEL;

        OotcInfo.rotor = decodedPulseVal & 0x01;
        OotcInfo.data =  decodedPulseVal & 0x02;
        OotcInfo.skip =  decodedPulseVal & 0x04;
#if 0
        Serial.print((ringBuff.writerPos + RINGBUFF_MAX - ringBuff.readerPos) % RINGBUFF_MAX);
        Serial.print(" ");
        Serial.print(OotcInfo.rotor);
        Serial.print(" ");
        Serial.print(OotcInfo.data);
        Serial.print(" ");
        Serial.println(OotcInfo.skip);

#endif        

#if 1
static int jumpCounter=0;
jumpCounter++;
if (jumpCounter %10 == 0)
{
        Serial.print("TS3633 ");
        for (int i=0; i < MAX_RECEIVERS; i++)
        {
          Serial.print(i);
          Serial.print(" ");
          Serial.print(gReceiver[i].X);
          Serial.print(" ");
          Serial.print(gReceiver[i].Y);
          Serial.print(" "); 
          gReceiver[i].X = 0;
          gReceiver[i].Y = 0;     
        }
        
        Serial.println("");
}
#endif      
      }
    }

    static int counter = 0;
    counter++;
    if (counter % 100 == 0 & false)
    {
        Serial.print((ringBuff.writerPos + RINGBUFF_LEN - ringBuff.readerPos) & RINGBUFF_MODULO_MASK);
        Serial.print(" ");
        Serial.print(isFalling);
        Serial.print(" ");
        Serial.print(sensor);
        Serial.print(" ");
        Serial.println(val);
    }
  }
}

// *********************
// ******* ISRs ********
// *********************

//    gReceiver[receiver].buff[gReceiver[receiver].writerPos] = SysTick->VAL  + ((10000-(millis()%10000)) * 84000);\

#define RISING_INTERRUPT_BODY(receiver) \
  if (ringBuff.readerPos != ringBuff.writerPos)\
  {\
    ringBuff.buff[ringBuff.writerPos] = SysTick->GetVal() | (receiver<<24);\
    ringBuff.writerPos = (ringBuff.writerPos+1) & RINGBUFF_MODULO_MASK;\
  }

//    gReceiver[receiver].buff[gReceiver[receiver].writerPos] = (SysTick->VAL + ((10000-(millis()%10000)) * 84000)) | FALLING_EDGE;\

#define FALLING_INTERRUPT_BODY(receiver)\
  if (ringBuff.readerPos != ringBuff.writerPos)\
  {\
    ringBuff.buff[ringBuff.writerPos] = (SysTick->GetVal()) | FALLING_EDGE | (receiver<<24);\
    ringBuff.writerPos = (ringBuff.writerPos+1) & RINGBUFF_MODULO_MASK;\
  }

void ISR0()
{
  if(digitalRead(sensor0) == LOW) {
    RISING_INTERRUPT_BODY(0)
  }
  else {
    FALLING_INTERRUPT_BODY(0)
  }
}

void ISR1()
{
  if(digitalRead(sensor1) == LOW) {
    RISING_INTERRUPT_BODY(1)
  }
  else {
    FALLING_INTERRUPT_BODY(1)
  }
}

void ISR2()
{
  if(digitalRead(sensor2) == LOW) {
    RISING_INTERRUPT_BODY(2)
  }
  else {
    FALLING_INTERRUPT_BODY(2)
  }
}

void ISR3()
{
  if(digitalRead(sensor3) == LOW) {
    RISING_INTERRUPT_BODY(3)
  }
  else {
    FALLING_INTERRUPT_BODY(3)
  }
}

void ISR4()
{
  if(digitalRead(sensor4) == LOW) {
    RISING_INTERRUPT_BODY(4)
  }
  else {
    FALLING_INTERRUPT_BODY(4)
  }
}

void ISR5()
{
  if(digitalRead(sensor5) == LOW) {
    RISING_INTERRUPT_BODY(5)
  }
  else {
    FALLING_INTERRUPT_BODY(5)
  }
}

void ISR6()
{
  if(digitalRead(sensor6) == LOW) {
    RISING_INTERRUPT_BODY(6)
  }
  else {
    FALLING_INTERRUPT_BODY(6)
  }
}

void ISR7()
{
  if(digitalRead(sensor7) == LOW) {
    RISING_INTERRUPT_BODY(7)
  }
  else {
    FALLING_INTERRUPT_BODY(7)
  }
}

void ISR8()
{
  if(digitalRead(sensor8) == LOW) {
    RISING_INTERRUPT_BODY(8)
  }
  else {
    FALLING_INTERRUPT_BODY(8)
  }
}

void ISR9()
{
  if(digitalRead(sensor9) == LOW) {
    RISING_INTERRUPT_BODY(9)
  }
  else {
    FALLING_INTERRUPT_BODY(9)
  }
}

// IMU ISRs
void ImuOneDataReady() {
    imuOne.SetInterrupt(true);
}

void ImuTwoDataReady() {
    imuTwo.SetInterrupt(true);
}

