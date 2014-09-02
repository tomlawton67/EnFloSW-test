// Developed by Paul Nathan, last mod. 18/01/2014

int PinTx_EN;
unsigned int txDelay;

const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
char Buffer[L];

const char thisAddress = 0;
const long baud = 476000;


// MASTER DUE CODE

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1     2       3    4    5    6    7    8    9     10     11


void setup()
{
  PinTx_EN = 2;

  pinMode(PinTx_EN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(A11, OUTPUT); // buf
  pinMode(A10, OUTPUT); // chk
  pinMode(A9, OUTPUT); // frm

  digitalWrite(A11, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A9, LOW);

  digitalWrite(PinTx_EN, LOW);  // Disable transmit mode

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
      Serial.readBytes(Buffer, L); // read msg into buffer

      // calculate checksum and place on end of buffer
      Buffer[Lm2] = static_cast<char>(CalcChecksum());

      digitalWrite(PinTx_EN, HIGH);  // Enable transmit mode on RS-485 transceiver

      Serial1.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485
      Serial1.flush(); // wait until write complete
      delayMicroseconds(txDelay); // don't miss out last byte!

      digitalWrite(PinTx_EN, LOW);  // Disable transmit mode on RS-485 transceiver
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
      Serial1.readBytes(Buffer, L); // read msg into buffer (excludes char argument!) 

      // only interested in messages destined for this address
      if (Buffer[2] == thisAddress)
      {
        // Check if rx checksum equals calculated checksum, 0 good, 1 bad
        if (Buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          Buffer[Lm2] = 0;
          digitalWrite(A10, LOW);
        }
        else
        {
          Buffer[Lm2] = 1;
          digitalWrite(A10, HIGH);
        }

        Serial.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to PC
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


inline byte CalcChecksum()
{
  byte sum = 0;
  for (int i = 0; i < Lm2; i++)
  {
    sum += static_cast<byte>(Buffer[i]);
  }
  return sum;
}
































