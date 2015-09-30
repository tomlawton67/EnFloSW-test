// Code for Sharp GP2Y0E03 IR distance sensor (triangulation) running on Arduino. Written and tested by Paul Nathan 03/10/2014
#include <Wire.h>


const int ADDR = 0x80 >> 1;
const byte DATA = 0x5E;
const byte n = 0x02; // set to 640mm max with 0.156mm resolution. 0x01 for 1280mm max (at half the resolution)
const float fact = 1.0 / (1.6 * float(1 << n));

float dist;
byte err, c[2], dist_bytes[4], buf[4];

const unsigned long Loop_dt = 20000; //us // 50Hz refresh (sensor response meaured to be 55Hz)
unsigned long t_now, t_last;

byte isActive;


void setup()
{
  Serial.begin(115200);
  Wire.begin(); // Join I2C bus as master (no address specified)         

  Configure_GP2Y0E03();

  t_last = micros();
}


void loop()
{
  SerialLoop(); // when doing nothing, check serial port for command

  t_now = micros();
  if ((t_now - t_last) >= Loop_dt) // convert sample only every dt interval
  {
    t_last = t_now;

    if (isActive == 1) ConvertSample();
  }
}


inline void ConvertSample()
{
  Wire.beginTransmission(ADDR);
  Wire.write(DATA);
  err = Wire.endTransmission(false);

  if (err == 0)
  {
    Wire.requestFrom(ADDR, 2, true);
    if (Wire.available() == 2)
    {
      c[0] = Wire.read(); // High byte 11:4 on register 0x5F (DATA)
      c[1] = Wire.read(); // Low byte 3:0 on register 0x5F (DATA + 1)
      dist = static_cast<float>((static_cast<word>(c[0]) << 4) + static_cast<word>(c[1])) * fact; // calculation from application note (optimised)
    }
    else
    {
      while (Wire.available() > 0)
      {
        Wire.read(); // Discard Wire buffer. Stale / intermediate data
      }
      dist = -1.0; // error output (non-sensical distance value)
    }
  }
  else
  {
    dist = -1.0; // error output (non-sensical distance value)
  }
}


inline void SerialLoop()
{
  if (Serial.available() >= 2)
  {
    if (Serial.read() == '@') // only respond to correct attention command. Discard in any case
    {
      switch (static_cast<int>(static_cast<byte>(Serial.read())))
      {
      case 0: // Sleep
        setActive(0);
        dist = -1.0;
        break;
      case 1: // Read (and auto-activate if necessary)
        if (isActive == 0)
        {
          setActive(1);
          ConvertSample();
        } // else will use latest sample from loop
        break;
      }
      memcpy(&dist_bytes, &dist, 4); // float to 4 bytes, Endian corrected output in buf to go over USB serial
      buf[0] = dist_bytes[3];
      buf[1] = dist_bytes[2];
      buf[2] = dist_bytes[1];
      buf[3] = dist_bytes[0];
      Serial.write(buf, 4); //mm // send out most recent distance value
    } 
  }
}


void setActive(byte ACTIVE)
{
  Wire.beginTransmission(ADDR);
  Wire.write(0xE8);
  Wire.write(0b00000001 & ~ACTIVE); // ACTIVE = 0 is standby, 1 is active. Device register requires the opposite!
  Wire.endTransmission(true);

  if (ACTIVE == 1) delay(50); // warm-up time
  isActive = ACTIVE;
}


void Configure_GP2Y0E03()
{
  // Software RESET
  Wire.beginTransmission(ADDR);
  Wire.write(0xEF);
  Wire.write(0x00);
  Wire.endTransmission(true); 
  Wire.beginTransmission(ADDR);
  Wire.write(0xEC);
  Wire.write(0xFF);
  Wire.endTransmission(true); 
  Wire.beginTransmission(ADDR);
  Wire.write(0xEE);
  Wire.write(0x06);
  Wire.endTransmission(true); 
  Wire.beginTransmission(ADDR);
  Wire.write(0xEC);
  Wire.write(0x7F);
  Wire.endTransmission(true); 

  delay(1);

  // Set shift bit 
  Wire.beginTransmission(ADDR);
  Wire.write(0x35);
  Wire.write(n);
  Wire.endTransmission(true);

  // Set to Stand-by
  setActive(0);
}









