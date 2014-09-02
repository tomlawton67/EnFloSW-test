#include <EEPROM.h>


int N;


void setup()
{
  Serial.begin(115200);
}


void loop()
{
  // Enter EEPROM size in bytes
  // send with no line ending!
  if (Serial.available() > 0)
  {
    N = Serial.parseInt();

    // perform dump
    Serial.println("EEPROM dump");
    for (int i = 0; i < N; i++)
    {
      Serial.println(EEPROM.read(i));
    }
  }
}



