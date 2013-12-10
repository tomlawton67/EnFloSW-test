// Developed by Paul Nathan, last mod. 05/07/2013

int pinTX_ENABLE, L, Lm1, Lm2;
unsigned int txDelay;

char buffer[12];

const char thisAddress = 0;
const long baud = 476000;

// MASTER DUE CODE

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D, R/W, pin, val, val ,val, val, rxCHK, txCHK


void setup()
{
  pinTX_ENABLE = 2;
  L = 12; // msg length

  pinMode(pinTX_ENABLE, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(A11, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A9, OUTPUT);

  digitalWrite(A11, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A9, LOW);

  digitalWrite(pinTX_ENABLE, LOW);  // Disable transmit mode

  Lm1 = L - 1;
  Lm2 = L - 2;

  txDelay = (8 * 1000000) / baud; //us

  Serial.begin(276000); // send/receive messages to/from host controller PC via USB
  Serial1.begin(baud); // send/receive messages to/from RS-485 network
}


void loop()
{

  // Check if message waiting in USB serial buffer
  // if so, construct buffer and resend over RS-485 network
  // first check for buffer overrun, if so, turn on warning LED (pin13)
  if (Serial.available() > 63) digitalWrite(A11, HIGH); 
  while (Serial.available() >= L) // only do something if a full block of data is waiting from PC
  {
    if (Serial.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(buffer, 0x00, L); // initialise buffer  
      Serial.readBytes(buffer, L); // read msg into buffer

      // calculate checksum and place on end of buffer
      buffer[Lm2] = static_cast<char>(CalcChecksum());

      digitalWrite (pinTX_ENABLE, HIGH);  // Enable transmit mode on RS-485 transceiver

      Serial1.write(reinterpret_cast<byte*>(&buffer), L); // write buffer to RS-485
      Serial1.flush(); // wait until write complete
      delayMicroseconds(txDelay); // don't miss out last byte!

      digitalWrite (pinTX_ENABLE, LOW);  // Disable transmit mode on RS-485 transceiver
      digitalWrite(A11, LOW); // Successful msg cycle, reset any error lights
      digitalWrite(A10, LOW);
      digitalWrite(A9, LOW);
    }
    else
    {
      Serial.read(); // discard invalid byte
      digitalWrite(A9, HIGH); // warn of invalid byte(s)
    }
  }

  // Now check if message waiting in RS-485 transceiver buffer
  // if so, construct buffer and send to host controller PC
  // first check for buffer overload, if so, turn on warning LED (pin13)
  if (Serial1.available() > L) digitalWrite(A11, HIGH); 
  while (Serial1.available() >= L) // only do something if data is waiting from RS-485 network
  {
    if (Serial1.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(buffer, 0x00, L); // initialise buffer  
      Serial1.readBytes(buffer, L); // read msg into buffer (excludes char argument!) 

      // only interested in messages destined for this address
      if (buffer[2] == thisAddress)
      {
        // Check if rx checksum equals calculated checksum, 0 good, 1 bad
        if (buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          buffer[Lm2] = 0;
          digitalWrite(A10, LOW);
        }
        else
        {
          buffer[Lm2] = 1;
          digitalWrite(A10, HIGH);
        }

        Serial.write(reinterpret_cast<byte*>(&buffer), L); // write buffer to PC
        Serial.flush(); // wait until write complete (no need for txDelay on this serial)

        digitalWrite(A11, LOW); // Successful msg cycle, reset any error lights
        digitalWrite(A9, LOW);
      }
    }
    else
    {
      Serial1.read(); // discard invalid byte
      digitalWrite(A9, HIGH); // warn of invalid byte(s)
    }
  }

}


byte CalcChecksum()
{
  byte sum = 0;
  for (int i = 0; i < Lm2; i++)
  {
    sum += static_cast<byte>(buffer[i]);
  }
  return (sum);
}














































