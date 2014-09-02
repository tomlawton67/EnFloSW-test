#include <EEPROM.h>


int N;


void setup()
{
  Serial.begin(115200);
}


void loop()
{
  // Enter EEPROM size in bytes to perform fist-time initialisation
  // send with no line ending!
  if (Serial.available() > 0)
  {
    N = Serial.parseInt();
    Serial.print("Initialising EEPROM of length ");
    Serial.print(N);
    Serial.println(" bytes");
    Serial.println();

    // set byte 0 = 255 for first-time address initialisation
    EEPROM.write(0, 255);

    // set bytes 1...N-1 = zero
    for (int i = 1; i < N; i++)
      EEPROM.write(i, 0);

    // perform dump
    Serial.println("EEPROM dump");
    for (int i = 0; i < N; i++)
      Serial.println(EEPROM.read(i));
  }
}



