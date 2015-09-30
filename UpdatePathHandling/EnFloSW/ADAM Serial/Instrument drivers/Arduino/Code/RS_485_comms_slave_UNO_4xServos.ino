// Code for 4x miniature servos over RS-485
// Developed and tested by Paul Nathan. Last modification 19/06/2014

#include <EEPROM.h>
#include <Servo.h>

// Servo variables
const int nServos = 4;

Servo SERVO[nServos]; //hard-coded for Nservos
float PWM_val;
char PWM_bytes[4];

const int PinServo[nServos] = {8, 9, 10, 11};
const int PWM_min[nServos] = {900, 900, 900, 900};
const int PWM_max[nServos] = {2000, 2000, 2000, 2000};
int PWM_range[nServos];

int Chan;
bool canProceed;


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long baud = 476000;

const char Ident[4] = {'S', 'R', 'V', 'O'};


void setup()
{
  ServosSetup();

  SerialSetup();
}


void loop()
{
  SerialLoop();
}


// SLAVE CODE (ARDUINO UNO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C/I, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1      2       3      4    5    6    7    8    9     10     11


void SerialSetup()
{
  // Configure pins
  PinTx_EN = 4;
  Pin_frm = A3;
  Pin_chk = A4;
  Pin_buf = A5;

  pinMode(PinTx_EN, OUTPUT);
  pinMode(Pin_frm, OUTPUT);
  pinMode(Pin_chk, OUTPUT);
  pinMode(Pin_buf, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(13, LOW);
  digitalWrite(Pin_frm, LOW);
  digitalWrite(Pin_chk, LOW);
  digitalWrite(Pin_buf, LOW);
  digitalWrite(PinTx_EN, LOW);  // Disable transmit mode

  // Set Pin numbers to port bytes for speed
  PinTx_EN = (1 << PIND4);
  Pin_frm = (1 << PINC3);
  Pin_chk = (1 << PINC4);
  Pin_buf = (1 << PINC5);

  txDelay = (8 * 1000000) / baud; //us

  Serial.begin(baud); // send/receive messages to/from RS-485 network
}


void ServosSetup()
{
  for (int i = 0; i < nServos; i++)
  {
    PWM_range[i] = PWM_max[i] - PWM_min[i];
    SERVO[i].attach(PinServo[i], PWM_min[i], PWM_max[i]);
  }
}


inline void SerialLoop()
{
  // Check if message waiting in RS-485 transducer buffer
  // if so, construct variable buffer, implement request, and send output back over RS-485 to master
  // first check for buffer overrun, if so, turn on warning LED (pin13)
  if (Serial.available() > 63) PORTC |= Pin_buf;
  while (Serial.available() >= L) // only do something if data is waiting from RS-485 network
  {
    if (Serial.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(Buffer, 0x00, L); // initialise buffer
      Serial.readBytes(Buffer, L); // read msg into buffer

      // only interested in messages destined for this address
      if (Buffer[2] == thisAddress)
      {
        // only worth proceeding if checksum test passes, otherwise ignore instruction and return bad txChk byte
        if (Buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          // For Servos only care if Analogue Write is requested
          Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
          canProceed = (Chan >= 0) && (Chan < nServos);
          switch (Buffer[3])
          {
            case 'A':
              // Analogue
              // check whether read or write
                switch (Buffer[4])
                {
                  case 'W':
                    // Write
                    // ########################################## Start of Servo-specific code ###################################################################
                    if (canProceed)
                    {
                      PWM_bytes[0] = Buffer[9];
                      PWM_bytes[1] = Buffer[8];
                      PWM_bytes[2] = Buffer[7];
                      PWM_bytes[3] = Buffer[6];
                      memcpy(&PWM_val, &PWM_bytes, sizeof(PWM_val));
                      SERVO[Chan].writeMicroseconds(int((constrain(PWM_val, 0.0, 1.0) * PWM_range[Chan]) + PWM_min[Chan])); // Chan represent servo num
                    }
                    // ########################################## End of Servo-specific code #####################################################################
                    break;
              }
              break;
           case 'C':
              // Config. mode
              // check whether read or write
              switch (Buffer[4])
              {
                case 'W':
                  // Write
                  // ########################
                 SetConfig();
                  // ########################
                  break;
                case 'R':
                  // Read
                  // ########################
                 GetConfig();
                  // ########################
                  break;
              }
              break;
            case 'I':
              // Ident. mode
              // return device ident 4 bytes
              // ########################
              GetIdent();
              // ########################
              break;
          }
          // now switch source and destination
          tmp = Buffer[1];
          Buffer[1] = Buffer[2];
          Buffer[2] = tmp;

          // to get here, txChk was okay, send back good txChk byte
          Buffer[Lm1] = 0;
          // calculate checksum and place on end of buffer
          Buffer[Lm2] = static_cast<char>(CalcChecksum());

          // Finally write updated buffer (now with requested value) to RS-485 network to be read by master
          PORTD |= PinTx_EN;  // Enable transmit mode on RS-485 transceiver (Tx_EN = 1)

          Serial.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485 DI
          Serial.flush(); // wait until write complete
          delayMicroseconds(txDelay); // don't miss out last byte!

          PORTD &= ~PinTx_EN; // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
          PORTC &= ~Pin_frm; // Successful msg cycle, reset any error lights
          PORTC &= ~Pin_chk;
          PORTC &= ~Pin_buf;
        }
        else
        {
          // warn of bad checksum
          // switch source and destination
          tmp = Buffer[1];
          Buffer[1] = Buffer[2];
          Buffer[2] = tmp;

          // to get here, txChk was bad, send back baf txChk byte
          Buffer[Lm1] = 1;
          // calculate checksum and place on CHKtmp byte
          Buffer[Lm2] = static_cast<char>(CalcChecksum());

          // Finally write updated buffer (now with requested value) to RS-485 network to be read by master
          PORTD |= PinTx_EN;  // Enable transmit mode on RS-485 transceiver (Tx_EN = 1)

          Serial.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485 DI
          Serial.flush(); // wait until write complete
          delayMicroseconds(txDelay); // don't miss out last byte!

          PORTD &= ~PinTx_EN; // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
          PORTC |= Pin_chk; // turn on bad checksum LED
        }
      }
    }
    else
    {
      Serial.read(); // discard invalid byte
      PORTC |= Pin_frm; // warn of invalid byte(s), turn on frm LED
    }
  }
}


void SetConfig()
{
  const byte val = static_cast<byte>(ValBytesToFloat());

  if (Chan == 255) // set address
  {
    thisAddress = static_cast<char>(val);
    EEPROM.write(0, val);
  }
}


void GetConfig()
{
  const float val = static_cast<float>(EEPROM.read(Chan));

  ValFloatToBytes(val);
}


inline void GetIdent()  // direct to comms buffer
{
  memcpy(&Buffer[6], &Ident, 4);
}


inline byte CalcChecksum()
{
  byte sum = 0;
  for (int i = 0; i < Lm2; i++)
  {
    sum += static_cast<byte>(Buffer[i]);
  }
  return (sum);
}


inline float ValBytesToFloat() // direct from comms buffer
{
  char val_bytes[4];
  float val;

  val_bytes[0] = Buffer[9];
  val_bytes[1] = Buffer[8];
  val_bytes[2] = Buffer[7];
  val_bytes[3] = Buffer[6];
  memcpy(&val, &val_bytes, 4);

  return val;
}


inline void ValFloatToBytes(float val) // direct to comms buffer
{
  const char *val_bytes = reinterpret_cast<char*>(&val);
  Buffer[6] = val_bytes[3];
  Buffer[7] = val_bytes[2];
  Buffer[8] = val_bytes[1];
  Buffer[9] = val_bytes[0];
}
