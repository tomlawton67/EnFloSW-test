// Arduino Puff Release. Version 1.1. Last Mod Paul Nathan 18/06/2013, 21/03/2014 (minor optimisation), 02/05/2014 (fixed length messaging).

int pinTRIG = 4;

unsigned long Hold_ON, Hold_OFF; // us
unsigned int i, Ncycles;

const int L = 10; // message byte length (not including start and end chars)
char Buffer[L];
char Ncycles_bytes[2], Hold_OFF_bytes[4], Hold_ON_bytes[4];

unsigned long tNow, tCycleBegin; // us

void setup()
{
  pinMode(pinTRIG, OUTPUT);
  digitalWrite(pinTRIG, LOW);

  // turn off pin13 LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // set pin number to port bytes (speedup)
  pinTRIG = (1 << PIND4);

  // initialise buffer
  memset(Buffer, 0x00, L);

  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(1000);
}


void loop()
{
  // nothing to do in here!
}


void MainCycle()
{
  // loop Ncycles doing trigger high low timing as requested
  for (i = 1; i <= Ncycles; i++)
  {
    // First hold ON
    PORTD |= pinTRIG; //NPN transistor base high;
    tCycleBegin = micros();
    while ((micros() - tCycleBegin) < Hold_ON)
    {
      // do nothing until Hold_ON time has elapsed
    } 
    
    // Now hold OFF
    PORTD &= ~pinTRIG; //NPN transistor base low;
    tCycleBegin = micros();
    while ((micros() - tCycleBegin) < Hold_OFF)
    {
      // do nothing until Hold_OFF time has elapsed
    } 
    
    // inform PC of cycle number completed, in ASCII format
    Serial.println(i);
  }
  Serial.flush();
}


void serialEvent()
{
  if (Serial.peek() == '@') // beginning of instruction in correct place, read until termination char '#'
  {
    Serial.read(); // discard '@'
    // read all into buffer, then parse values as appropriate
    Serial.readBytes(Buffer, L);
    //Serial.write(reinterpret_cast<byte*>(&buffer), sizeof(buffer)); // echo for debug only

    Ncycles_bytes[0] = Buffer[1];
    Ncycles_bytes[1] = Buffer[0];
    memcpy(&Ncycles, &Ncycles_bytes, 2); // word is 2-bytes

    Hold_OFF_bytes[0] = Buffer[5];
    Hold_OFF_bytes[1] = Buffer[4];
    Hold_OFF_bytes[2] = Buffer[3];
    Hold_OFF_bytes[3] = Buffer[2];
    memcpy(&Hold_OFF, &Hold_OFF_bytes, 4); // uLong is 4-bytes

    Hold_ON_bytes[0] = Buffer[9];
    Hold_ON_bytes[1] = Buffer[8];
    Hold_ON_bytes[2] = Buffer[7];
    Hold_ON_bytes[3] = Buffer[6];
    memcpy(&Hold_ON, &Hold_ON_bytes, 4); // uLong is 4-bytes

    MainCycle();
  }
  else
  {
    // discard entire buffer
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }
}















