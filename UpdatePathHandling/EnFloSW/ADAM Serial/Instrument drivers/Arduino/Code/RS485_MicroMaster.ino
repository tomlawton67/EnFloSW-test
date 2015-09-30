// Code for RS-323 / RS-485 transceiver (specific to "Arduino network" messaging protocol)
// Developed by Paul Nathan, last mod. 09/06/2014


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
//unsigned int txDelay;
char Buffer[L], tmp;

const char thisAddress = 0; // Master always has address 0
const long baud = 276000;   // USB link speed
const long baud1 = 476000;  // speed over RS-485 network


void setup()
{
  Serial_Setup();
  Serial1_Setup();
}

void loop()
{
  Serial_Loop();
  Serial1_Loop();
}


// MASTER CODE (ARDUINO MICRO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C/I, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1      2       3      4    5    6    7    8    9     10     11


void Serial_Setup()
{
  Serial.begin(baud); // USB serial
}


void Serial1_Setup()
{
  // Configure pins
  PinTx_EN = 13;
  Pin_frm = 12;
  Pin_chk = 11;
  Pin_buf = 10;

  pinMode(PinTx_EN, OUTPUT);
  pinMode(Pin_frm, OUTPUT);
  pinMode(Pin_chk, OUTPUT);
  pinMode(Pin_buf, OUTPUT);

  digitalWrite(Pin_frm, LOW);
  digitalWrite(Pin_chk, LOW);
  digitalWrite(Pin_buf, LOW);
  digitalWrite(PinTx_EN, LOW);  // Disable transmit mode

  // Set Pin numbers to port bytes for direct register manipulation
  PinTx_EN = (1 << PINC7);
  Pin_frm = (1 << PIND6);
  Pin_chk = (1 << PINB7);
  Pin_buf = (1 << PINB6);

  //txDelay = (8 * 1000000) / baud; //us

  Serial1.begin(baud1); // RS-485 serial (via Rx / Tx on micro)
}


inline void Serial_Loop()
{
  // Check if message waiting in USB serial buffer
  // if so, construct buffer and resend over RS-485 network
  // first check for buffer overrun, if so, turn on warning LED (pin13)
  if (Serial.available() > 63) PORTB |= Pin_buf;
  while (Serial.available() >= L) // only do something if a full block of data is waiting from PC
  {
    if (Serial.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(buffer, 0x00, L); // initialise buffer
      Serial.readBytes(Buffer, L); // read msg into buffer

      // calculate checksum and place on end of buffer
      Buffer[Lm2] = static_cast<char>(CalcChecksum());

      // Relay message over RS-485 ##########
      PORTC |= PinTx_EN;  // Enable transmit mode on RS-485 transceiver (Tx_EN = 1)

      Serial1.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485
      Serial1.flush(); // wait until write complete
      //delayMicroseconds(txDelay); // don't miss out last byte! // appears to be un-necessary on Micro

      PORTC &= ~PinTx_EN; // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
      // ####################################
      PORTD &= ~Pin_frm; // Successful msg cycle, reset any error lights
      PORTB &= ~Pin_chk;
      PORTB &= ~Pin_buf;
    }
    else
    {
      Serial.read(); // discard invalid byte
      PORTD |= Pin_frm; // warn of invalid byte(s), turn on frm LED
    }
  }
}


inline void Serial1_Loop()
{
  // Check if message waiting in RS-485 transceiver buffer
  // if so, construct buffer and send to host controller PC
  // first check for buffer overload, if so, turn on warning LED (pin13)
  if (Serial1.available() > 63) PORTB |= Pin_buf;
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
          PORTB &= ~Pin_chk;
        }
        else
        {
          Buffer[Lm2] = 1;
          PORTB |= Pin_chk;
        }

        Serial.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to PC
        Serial.flush(); // wait until write complete (no need for txDelay on this serial)

        PORTD &= ~Pin_frm; // Successful msg cycle, reset any error lights
        PORTB &= ~Pin_buf;
      }
    }
    else
    {
      Serial1.read(); // discard invalid byte
      PORTD |= Pin_frm; // warn of invalid byte(s), turn on frm LED
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
