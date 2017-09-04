// Code for AD7609 based Data acquisition system. Written and tested by Paul Nathan 20/11/2014. Rev 1: 18/08/2015. Rev 2: 21/12/2015. Rev 3: 08/06/2016 (Serial write outside of ISR)
// Board: Teensy 3.1 / 3.2; USB type: Serial; CPU speed: 144MHz (opt)
// !! Remember to cut Vusb to 5V link!!

#pragma GCC optimize ("O3")

#include <SPI.h>

IntervalTimer SampleClock;

#define pin_CS      10
#define pin_OVR     0
#define pin_ACQ     1
#define pin_OS0     2
#define pin_OS1     3
#define pin_OS2     4
#define pin_STBY    5
#define pin_RANGE   6
#define pin_CONVST  7
#define pin_RESET   8
#define pin_BUSY    9
#define pin_TRIG    23

#define spi_AD7609 SPISettings(24000000, MSBFIRST, SPI_MODE2)


float dt = 1000.0; // us

byte DataBuffer[18];  // 18 bits x 8 Ch = 144 bits (= 18 bytes) largest size it will ever be

const byte L = 10;
char SerIn[L];       // Config message format is ~,OS0,OS1,OS2,RANGE,TRIG,Nchan,[dt...  ]
//                                                  0   1   2    3    4     5    6 7 8 9
byte SerOut[L + 1];
char Cmd;

char dt_bytes[4];

unsigned long Overrun = 0;
const byte ack[3] = {'#', '#', '#'}; // acknowledgement for end of clocking. Repeated for robustness against overrun corruption

byte OS[3] = {0, 0, 0};
byte Range = 1;
byte TriggerMode = 0;
byte NchanBytes = 18;
byte SleepState = 0; // (0, 1, 2) = (wakeup, standby, shutdown)

volatile byte Clocking = 0;
volatile byte DataReady = 0;


void setup()
{
  SampleClock.priority(1); // Give sample clock highest priority after systick (0). (Note default is 128)

  Configure_Pins();

  SPI.begin();

  Serial.begin(12000000);
  //while (!Serial);

  //delay(30);
  Reset();
}


void loop()
{
  if (DataReady)
  {
    Serial.write(DataBuffer, NchanBytes); // Faster throughput by putting serial write here rather than in ISR
    if (dt > 1000.0)
    {
      if (NchanBytes < 7) Serial.send_now(); // Need to do this at slow speed else no stream!
    }

    DataReady = 0;
  }
  SerInCheck(); // Check for host PC commands
}


void Configure_Pins()
{
  pinMode(pin_CS, OUTPUT);
  pinMode(pin_OVR, OUTPUT);
  pinMode(pin_ACQ, OUTPUT);
  pinMode(pin_OS0, OUTPUT);
  pinMode(pin_OS1, OUTPUT);
  pinMode(pin_OS2, OUTPUT);
  pinMode(pin_STBY, OUTPUT);
  pinMode(pin_RANGE, OUTPUT);
  pinMode(pin_CONVST, OUTPUT);
  pinMode(pin_RESET, OUTPUT);
  pinMode(pin_BUSY, INPUT);
  pinMode(pin_TRIG, INPUT_PULLUP); // Floating state of trigger is high

  digitalWriteFast(pin_CS, HIGH);
  digitalWriteFast(pin_OVR, LOW);
  digitalWriteFast(pin_ACQ, LOW);
  digitalWriteFast(pin_OS0, LOW);
  digitalWriteFast(pin_OS1, HIGH);
  digitalWriteFast(pin_OS2, LOW);
  digitalWriteFast(pin_STBY, HIGH);
  digitalWriteFast(pin_RANGE, HIGH);
  digitalWriteFast(pin_CONVST, HIGH);
  digitalWriteFast(pin_RESET, LOW);

  // configure interrupt(s)
  attachInterrupt(digitalPinToInterrupt(pin_BUSY), Busy_FALLING, FALLING); // on low, read newest conversion
  SPI.usingInterrupt(digitalPinToInterrupt(pin_BUSY));
}


void Pulse()
{
  // Hold low for 25ns.
  digitalWriteFast(pin_CONVST, LOW); // Low

  __asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); //27.78ns delay at 144MHz.

  digitalWriteFast(pin_CONVST, HIGH); // High
  Clocking = 1;
}


void Busy_FALLING()
{
  Clocking = 0;
  // Pulse ISR can change volatile clocking global between being reset above and the following check
  // if it goes high, this means new data was being converted during the upcoming conversion
  // this is an overrun, set flags

  SPI.beginTransaction(spi_AD7609);
  digitalWriteFast(pin_CS, LOW);

  SPI.transfer(DataBuffer, NchanBytes);

  digitalWriteFast(pin_CS, HIGH);
  SPI.endTransaction();

  if (Clocking)
  {
    ++Overrun;
    digitalWriteFast(pin_OVR, HIGH);
  }

  DataReady = 1;
}


void Trigger()
{
  //switch (digitalReadFast(pin_TRIG)) // check trigger state high or low
  //{
  //case 0: // Low. Begin sampling
  ACQ_Begin();
  //break;
  //default: // High. Stop sampling
  //ACQ_End();
  //break;
  //}
}


void Sleep(byte NewSleepState)
{
  switch (NewSleepState)
  {
    case 0: // wakeup
      switch (SleepState)
      {
        case 0: // already awake, do nothing

          break;
        case 1: // re-config range bit and wait 100us
          digitalWriteFast(pin_RANGE, Range);
          digitalWriteFast(pin_STBY, HIGH);
          delayMicroseconds(100);
          break;
        case 2: // re-config range bit, wait 13ms, apply reset
          digitalWriteFast(pin_RANGE, Range);
          digitalWriteFast(pin_STBY, HIGH);
          delay(13);
          Reset();
          break;
      }
      break;
    case 1: // standby
      digitalWriteFast(pin_RANGE, HIGH);
      digitalWriteFast(pin_STBY, LOW);
      break;
    case 2: // shutdown
      digitalWriteFast(pin_RANGE, LOW);
      digitalWriteFast(pin_STBY, LOW);
      break;
  }
  SleepState = NewSleepState;
}


void Reset()
{
  ACQ_End();
  Overrun = 0;
  digitalWriteFast(pin_OVR, LOW);

  digitalWriteFast(pin_RESET, HIGH);
  delayMicroseconds(1);
  digitalWriteFast(pin_RESET, LOW);
  delayMicroseconds(1);

  // Gets rid of first spurious sample glitch
  Busy_FALLING();
  DataReady = 0;
}


inline void ACQ_Begin()
{
  SampleClock.begin(Pulse, dt);

  Overrun = 0;
  digitalWriteFast(pin_ACQ, HIGH);
  digitalWriteFast(pin_OVR, LOW);
}


inline void ACQ_End()
{
  SampleClock.end();

  digitalWriteFast(pin_ACQ, LOW);
  detachInterrupt(pin_TRIG);
}


void SerInCheck()
{
  if (Serial.available()) // only proceed if byte(s) from PC waiting in buffer
  {
    Cmd = Serial.read();
    if (Cmd == '@') // Acquisition begin
    {
      ACQ_Begin();
    }
    else if (Cmd == '#') // Acquisition stop
    {
      ACQ_End();
      Serial.write(ack, 3); // acknowledge clocking is over to allow proper clearing of host input buffer surplus bytes (sent in between end of sample requests and end_ack cmd)
    }
    else if (Cmd == '?') // Send back overrun status bit
    {
      byte tmp[4];
      memcpy(&tmp[0], &Overrun, 4); // 4-byte float for dt
      Serial.write(tmp, 4);
    }
    else if (Cmd == '$') // Sleep state change, $0 or $1 or $2 (wakeup, standby, shutdown)
    {
      Sleep(static_cast<int>(static_cast<byte>(Serial.read()))); // 0...2
    }
    else if (Cmd == '%') // Reset
    {
      Reset();
    }
    else if (Cmd == '~') // Configuration message
    {
      // Config message format is ~,OS0,OS1,OS2,RANGE,TRIG,Nchan,[dt...  ]
      //                             0   1   2    3    4     5    6 7 8 9
      Serial.readBytes(SerIn, L);

      // Oversampling ratio
      OS[0] = static_cast<int>(static_cast<byte>(SerIn[0]));
      OS[1] = static_cast<int>(static_cast<byte>(SerIn[1]));
      OS[2] = static_cast<int>(static_cast<byte>(SerIn[2]));
      digitalWriteFast(pin_OS0, OS[0]);
      digitalWriteFast(pin_OS1, OS[1]);
      digitalWriteFast(pin_OS2, OS[2]);

      // Input voltage Range
      Range = static_cast<int>(static_cast<byte>(SerIn[3]));
      digitalWriteFast(pin_RANGE, Range);

      // Number of channels (bytes to receive)
      NchanBytes = static_cast<int>(static_cast<byte>(SerIn[5]));

      // Sampling interval
      dt_bytes[0] = SerIn[9];
      dt_bytes[1] = SerIn[8];
      dt_bytes[2] = SerIn[7];
      dt_bytes[3] = SerIn[6];
      memcpy(&dt, &dt_bytes, 4); // 4-byte float for dt

      // Trigger mode
      TriggerMode = static_cast<int>(static_cast<byte>(SerIn[4]));
      switch (TriggerMode)
      {
        case 0:
          pinMode(pin_TRIG, INPUT_PULLUP);
          detachInterrupt(pin_TRIG);
          break;
        case 1:
          pinMode(pin_TRIG, INPUT_PULLUP);
          attachInterrupt(pin_TRIG, Trigger, FALLING);
          break;
        case 2:
          pinMode(pin_TRIG, INPUT_PULLDOWN);
          attachInterrupt(pin_TRIG, Trigger, RISING);
          break;
      }
    }
    else if (Cmd == '&') // Send back config
    {
      // Output message format is ~,OS0,OS1,OS2,RANGE,TRIG,Nchan,[dt...   ]
      //                          0  1   2   3    4     5    6    7 8 9 10
      SerOut[0] = static_cast<byte>('~');
      memcpy(&SerOut[1], &OS, 3);
      SerOut[4] = Range;
      SerOut[5] = TriggerMode;
      SerOut[6] = NchanBytes;
      SerOut[7] = dt_bytes[3];
      SerOut[8] = dt_bytes[2];
      SerOut[9] = dt_bytes[1];
      SerOut[10] = dt_bytes[0];
      Serial.write(SerOut, L + 1);
    }
    else if (Cmd == '!') //debug
    {
      Serial.println("Hello"); // insert debug variable output in place of "Hello"!
    }
    //    else
    //    {
    //      // discard entire buffer
    //      while (Serial.available())
    //        Serial.read();
    //    }
  }
}
