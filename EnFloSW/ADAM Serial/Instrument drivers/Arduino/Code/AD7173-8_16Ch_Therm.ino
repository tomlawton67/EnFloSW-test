// Code for AD7173-8 16Ch 24-bit thermistor ADC over RS-485
// Developed and tested by Paul Nathan. Last modification 27/06/2015

#include <SPI.h>
#include <EEPROM.h>


// Thermistor variables
const float invB = 1.0 / 4303.0; // 25 - 80 degC
const float R2 = 100000.0;
const float R10 = 100000.0;
const float invT0 = 1.0 / 298.15;
const float Vs = 2.50; // hard-wired to Vref
float R1;


// AD7173-8 variables
int Pin_CS = A5;

const byte NOP = 0x00;
const float Vref = 2.50;

const int TotalChannels = 16;

const unsigned long PollPeriod = 1500; //us
unsigned long Now, LastPoll;

byte STATUSREG;
unsigned long DATAREG;

int Chan, ADCchannel;
float ADCvolts[16], Volts, Temperature;

unsigned long LastAutoCalib_ms, Now_ms;
unsigned long AutoCalib_ms = 60000UL * static_cast<unsigned long>(EEPROM.read(1)); // self-calib every x minutes

bool canProceed;


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
//unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long Baud = 476000;

const char Ident[4] = {'A', 'D', 'C', 't'};


void setup()
{
  AD7173Setup();

  SerialSetup();
}


void loop()
{
  // Poll RDY bit in status register. If it has gone low, get channel data
  // chip configured in continuous conversion mode, auto-cycles through channels in ascending order
  Now = micros();
  if ((Now - LastPoll) > PollPeriod)
  {
    LastPoll = Now;

    // check RDY bit in status register
    PORTF &= ~Pin_CS; // Low
    SPI.transfer(0b01000000); // STATUS REGISTER, Read
    STATUSREG = SPI.transfer(NOP);
    PORTF |= Pin_CS; // High

    if ((STATUSREG & 0b10000000) == 0)
    {
      RDY_low();
    }
  }

  SerialLoop();

  // Check self-calib time
  if (AutoCalib_ms > 0)
  {
    Now_ms = millis();
    if ((Now_ms - LastAutoCalib_ms) > AutoCalib_ms)
    {
      LastAutoCalib_ms = Now_ms;
      CalibrateAD7173();
    }
  }
}


// SLAVE CODE (ARDUINO MICRO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C/I, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1      2       3      4    5    6    7    8    9     10     11


void SerialSetup()
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

  // Set Pin numbers to port bytes for speed
  PinTx_EN = (1 << PINC7);
  Pin_frm = (1 << PIND6);
  Pin_chk = (1 << PINB7);
  Pin_buf = (1 << PINB6);

  //txDelay = (8 * 1000000) / baud; //us

  Serial1.begin(Baud); // send/receive messages to/from RS-485 network. MUST USE SERIAL1 ON MICRO!! Not Serial (which is USB)
}


void AD7173Setup()
{
  // Setup pins
  pinMode(Pin_CS, OUTPUT);

  digitalWrite(Pin_CS, HIGH);

  // SPI comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  // Set Pin numbers to port bytes for speed
  Pin_CS = (1 << PINF0);

  // Configure AD7173-8
  RESET();
  ConfigureAD7173();
  CalibrateAD7173();

  LastAutoCalib_ms = millis();
}


inline void RDY_low()
{
  // new data available, read DATA register
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b01000100); // DATA REGISTER, Read
  DATAREG = static_cast<unsigned long>(SPI.transfer(NOP)) << 16;
  DATAREG |= static_cast<unsigned long>(SPI.transfer(NOP)) << 8;
  DATAREG |= static_cast<unsigned long>(SPI.transfer(NOP));
  PORTF |= Pin_CS; // High

  ADCchannel = STATUSREG & 0b00001111;
  ADCvolts[ADCchannel] = Vref * (static_cast<float>(DATAREG) / 16777216.0); // Uni-polar 24-bit binary to volts
}


void ConfigureAD7173()
{
  // Setup configuration 0
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00100000); // SETUP CONFIGURATION REGISTER 0, Write
  SPI.transfer(0b00001111); // 000, 0 Unipolar, 11 ref buf enabled, 11 Ain buf enabled
  SPI.transfer(0b01100000); // 0 burnout current off, 1 buf chop max freq, 10 internal 2.5V ref, 0000
  PORTF |= Pin_CS; // High

  // Configure 16 Single ended inputs, with REF- as the negative reference (0V)
  for (byte i = 0; i < 16; i++)
  {
    PORTF &= ~Pin_CS; // Low
    SPI.transfer(0b00010000 | i ); // CHANNEL i REGISTER, Write
    SPI.transfer(0b10000000 | ((i & 0b00011000) >> 3)); // 1 Ch enabled, 000 use setup config 0, 00, xx AINi
    SPI.transfer(((i & 0b00000111) << 5) | 0b00010110); // xxx AINi, 10110 REF-
    PORTF |= Pin_CS; // High
  }

  // Filter configuration 0
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00101000); // FILTER CONFIGURATION REGISTER 0, Write
  SPI.transfer(0b00000000); // 0 SINC3_MAP0 off, 000, 0 ENHFILTEN0 off, 000 ENHFILT0 none
  SPI.transfer(0b00001101); // 0, 00 Sinc5 + sinc1, 01101 200 Hz data rate <<<<<<<<<<<<<<<<<<<<<<<<<<<<< DATA RATE SET HERE!
  PORTF |= Pin_CS; // High

  // ADC mode register
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00000001); // ADC MODE REGISTER, Write
  SPI.transfer(0b10100000); // 1 internal ref on, 0, 1 Single cycle settling, 00, 000 no delay after channel switch
  SPI.transfer(0b00000000); // 0, 000 continuous conversion mode, 00 internal clock, 00
  PORTF |= Pin_CS; // High

  // Interface mode register
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00000010); // INTERFACE MODE REGISTER, Write
  SPI.transfer(0b00000101); // 000, 0 ALT_SYNC disabled, 0 IOSTRENGTH disabled, 1 HIDE_DELAY_ false, 0, 1 DOUT_RESET enabled
  SPI.transfer(0b00000000); // 0 CONTREAD disabled, 0 DATA_STAT disabled, 0 REG_CHECK disabled, 0, 00 CRC disabled, 0, 0 24-bit data
  PORTF |= Pin_CS; // High

  // GPIO configuration register
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00000010); // GPIO CONFIGURATION REGISTER, Write
  SPI.transfer(0b00000000); // 0, 0 PDSW disabled, 0 GPO2,3 disabled, 0 MUX_IO disabled, 0 SYNC disabled, 00 ERROR disabled, 0 ERR_DAT low
  SPI.transfer(0b00000000); // GPIO pin settings, not needed
  PORTF |= Pin_CS; // High
}


void CalibrateAD7173()
{

  // turn off all channels except 0
  for (byte i = 1; i < 16; i++)
  {
    PORTF &= ~Pin_CS; // Low
    SPI.transfer(0b00010000 | i ); // CHANNEL i REGISTER, Write
    SPI.transfer(0b00000000 | ((i & 0b00011000) >> 3)); // 0 Ch disabled, 000 use setup config 0, 00, xx AINi
    SPI.transfer(((i & 0b00000111) << 5) | 0b00010110); // xxx AINi, 10110 REF-
    PORTF |= Pin_CS; // High
  }

  // reduce data rate to 1.25Hz value for best calibration
  // Filter configuration 0
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00101000); // FILTER CONFIGURATION REGISTER 0, Write
  SPI.transfer(0b00000000); // 0 SINC3_MAP0 off, 000, 0 ENHFILTEN0 off, 000 ENHFILT0 none
  SPI.transfer(0b00010110); // 0, 00 Sinc5 + sinc1, 10110 1.25 Hz data rate
  PORTF |= Pin_CS; // High

  // BEGIN INTERNAL OFFSET CALIBRATION
  // ADC mode register
  PORTF &= ~Pin_CS; // Low
  SPI.transfer(0b00000001); // ADC MODE REGISTER, Write
  SPI.transfer(0b10100000); // 1 internal ref on, 0, 1 Single cycle settling, 00, 000 no delay after channel switch
  SPI.transfer(0b01000000); // 0, 100 internal offset calibration, 00 internal clock, 00
  PORTF |= Pin_CS; // High

  // Poll STATUS register for RDY_ bit going low
  do
  {
    delayMicroseconds(PollPeriod);

    PORTF &= ~Pin_CS; // Low
    SPI.transfer(0b01000000); // STATUS REGISTER, Read
    STATUSREG = SPI.transfer(NOP);
    PORTF |= Pin_CS; // High

    SerialLoop(); // don't hold up entire network!

  } while ((STATUSREG & 0b10000000) != 0);

  // AD7173-8 is now in standby mode, run full config to resume
  ConfigureAD7173();
}


void RESET()
{
  for (int i = 0; i < 8; i++)
  {
    SPI.transfer(0xFF);
  }
  delayMicroseconds(1000);
}


inline float VoltstoTemp(float& V)
{
  R1 = ((Vs / V) - 1.0) * R2;
  return ((1.0 / ((invB * log(R1 / R10)) + invT0)) - 273.15); // degC
}


void SerialLoop()
{
  // Check if message waiting in RS-485 transducer buffer
  // if so, construct variable buffer, implement request, and send output back over RS-485 to master
  // first check for buffer overrun, if so, turn on warning LED 'buf'
  if (Serial1.available() > 63) PORTD |= Pin_buf;
  while (Serial1.available() >= L) // only do something if data is waiting from RS-485 network
  {
    if (Serial1.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(Buffer, 0x00, L); // initialise buffer
      Serial1.readBytes(Buffer, L); // read msg into buffer

      // only interested in messages destined for this address
      if (Buffer[2] == thisAddress)
      {
        // only worth proceeding if checksum test passes, otherwise ignore instruction and return bad txChk byte
        if (Buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          // For AD7173 only care if Analogue Read is requested
          // Only proceed is specified channel is within range of device
          Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
          canProceed = (Chan > 0) && (Chan <= TotalChannels);
          switch (Buffer[3])
          {
            case 'A':
              // Analogue
              // check whether read or write
              switch (Buffer[4])
              {
                case 'R':
                  // Read ##################
                  if (canProceed)
                  {
                    ReadFromAD7173();
                    Temperature = VoltstoTemp(Volts);
                    ValFloatToBytes(Temperature); // direct to comms Buffer[6...9]
                  }
                  else 
                  {
                    ErrOut();
                  }
                  // ########################
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
                  if (canProceed || Chan == 255 || Chan == 0) SetConfig(); else ErrOut();
                  // ########################
                  break;
                case 'R':
                  // Read
                  // ########################
                  if (canProceed || Chan == 0) GetConfig(); else ErrOut();
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
          PORTC |= PinTx_EN;  // Enable transmit mode on RS-485 transceiver (Tx_EN = 1)

          Serial1.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485 DI
          Serial1.flush(); // wait until write complete
          //delayMicroseconds(txDelay); // don't miss out last byte! // appears to be un-necessary on Micro

          PORTC &= ~PinTx_EN; // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
          PORTD &= ~Pin_frm; // Successful msg cycle, reset any error lights
          PORTB &= ~Pin_chk;
          PORTB &= ~Pin_buf;
        }
        else
        {
          // warn of bad checksum
          // switch source and destination
          tmp = Buffer[1];
          Buffer[1] = Buffer[2];
          Buffer[2] = tmp;

          // to get here, txChk was bad, send back bad txChk byte
          Buffer[Lm1] = 1;
          // calculate checksum and place on CHKtmp byte
          Buffer[Lm2] = static_cast<char>(CalcChecksum());

          // Finally write updated buffer (now with requested value) to RS-485 network to be read by master
          PORTC |= PinTx_EN;  // Enable transmit mode on RS-485 transceiver (Tx_EN = 1)

          Serial1.write(reinterpret_cast<byte*>(&Buffer), L); // write buffer to RS-485 DI
          Serial1.flush(); // wait until write complete
          //delayMicroseconds(txDelay); // don't miss out last byte! // appears to be un-necessary on Micro

          PORTC &= ~PinTx_EN;  // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
          PORTB |= Pin_chk; // turn on bad checksum LED
        }
      }
    }
    else
    {
      Serial1.read(); // discard invalid byte
      PORTD |= Pin_frm; // warn of invalid byte(s), turn on frm LED
    }
  }

}


void SetConfig()
{
  const byte val = static_cast<byte>(ValBytesToFloat());

  switch (Chan)
  {
    case 255:
      // set address
      thisAddress = static_cast<char>(val);
      EEPROM.write(0, val);
      break;
    case 1:
      // change self-calibration interval (val in minutes)
      AutoCalib_ms = 60000UL * static_cast<unsigned long>(val);
      EEPROM.write(1, val);
      break;
    case 0:
      // perform self-calibration
      CalibrateAD7173();
      break;
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
  return sum;
}


inline void ErrOut()
{
  ValFloatToBytes(-9999);
}


inline float ValBytesToFloat()
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


void ReadFromAD7173() // Map physical channels to software channels
{
  switch (Chan)
  {
    case 1:
      Volts = ADCvolts[3];
      break;
    case 2:
      Volts = ADCvolts[2];
      break;
    case 3:
      Volts = ADCvolts[1];
      break;
    case 4:
      Volts = ADCvolts[0];
      break;
    case 5:
      Volts = ADCvolts[15];
      break;
    case 6:
      Volts = ADCvolts[14];
      break;
    case 7:
      Volts = ADCvolts[13];
      break;
    case 8:
      Volts = ADCvolts[12];
      break;
    case 9:
      Volts = ADCvolts[11];
      break;
    case 10:
      Volts = ADCvolts[10];
      break;
    case 11:
      Volts = ADCvolts[9];
      break;
    case 12:
      Volts = ADCvolts[8];
      break;
    case 13:
      Volts = ADCvolts[7];
      break;
    case 14:
      Volts = ADCvolts[6];
      break;
    case 15:
      Volts = ADCvolts[5];
      break;
    case 16:
      Volts = ADCvolts[4];
      break;
    default:
      Volts = 0.0;
  }
}



















