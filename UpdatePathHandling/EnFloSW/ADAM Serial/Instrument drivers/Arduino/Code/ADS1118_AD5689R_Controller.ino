// Code for ADS1118 4Ch 16-bit ADC and AD5689R 2Ch 16-bit DAC, over RS-485
// Developed and tested by Paul Nathan 03/08/2015. Last modification 10/09/2015

#include <SPI.h>
#include <EEPROM.h>


//ADS1118 variables
const byte nChips_ADC = 2;
const byte nChans_ADC = 4;
const byte Pin_CS_ADC[nChips_ADC] = {2, 3};
const byte Pin_DRDY = MISO;
int CurrentChan, ADC_Chan;
word DataReg;
float ADCvolts[nChips_ADC][nChans_ADC]; //[chipNum][Chan]
const float FS = 6.144; //Full-scale volts
const float invRes = FS / 32768.0; // 2^-15 (not 16 because of two's complement output)
const word ConfigReg = 0b0000000101101011; // 0000 SS+chanbits, 000 FS+-6.144V, 1 Single-shot mode, 011 64Hz data rate, 0 ADC mode, 1 DRDY pull-up enabled, 01 valid data, 1
const word ChannelBits[nChans_ADC] = {0b1100000000000000, 0b1101000000000000, 0b1110000000000000, 0b1111000000000000}; // 1 SS, xxx single-ended AIN0...3
const byte ConfigMSB[nChans_ADC] = {static_cast<byte>(((ConfigReg | ChannelBits[0]) & 0b1111111100000000) >> 8),
                                    static_cast<byte>(((ConfigReg | ChannelBits[1]) & 0b1111111100000000) >> 8),
                                    static_cast<byte>(((ConfigReg | ChannelBits[2]) & 0b1111111100000000) >> 8),
                                    static_cast<byte>(((ConfigReg | ChannelBits[3]) & 0b1111111100000000) >> 8)
                                   };
const byte ConfigLSB[nChans_ADC] = {static_cast<byte>(ConfigReg & 0b0000000011111111),
                                    static_cast<byte>(ConfigReg & 0b0000000011111111),
                                    static_cast<byte>(ConfigReg & 0b0000000011111111),
                                    static_cast<byte>(ConfigReg & 0b0000000011111111)
                                   };
const byte TotalChannels_ADC = nChips_ADC * nChans_ADC;
const byte VirtualChannel_Chip_ADC[TotalChannels_ADC + 1] = { -1, 0, 0, 0, 0, 1, 1, 1, 1};
const byte VirtualChannel_Chan_ADC[TotalChannels_ADC + 1] = { -1, 3, 2, 1, 0, 3, 2, 1, 0};
//                                                             0, 1  2  3  4  5  6  7, 8


//AD5689R variables
const byte nChips_DAC = 4;
const byte nChans_DAC = 2;
const byte Pin_CS_DAC[nChips_DAC] = {7, 6, 5, 4};
const byte AddressBits[nChans_DAC + 1] = {0b00000001, 0b00001000, 0b00001001}; // third one is for both outputs together
const byte INPUTREG = 0b00010000;
word DACval;
float ConvFact = 65536.0 / 1.0; // not / 5.0 so that the input is normalised 0....1
const byte TotalChannels_DAC = nChips_DAC * nChans_DAC;
const byte VirtualChannel_Chip_DAC[TotalChannels_DAC + 1] = { -1, 0, 0, 1, 1, 2, 2, 3, 3};
const byte VirtualChannel_Chan_DAC[TotalChannels_DAC + 1] = { -1, 0, 1, 0, 1, 0, 1, 0, 1};
//                                                             0, 1  2  3  4  5  6  7, 8


const SPISettings ADS1118(2000000, MSBFIRST, SPI_MODE1);
const SPISettings AD5689R(2000000, MSBFIRST, SPI_MODE1);


// RS-485 variables
byte PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const byte L = 12; // msg length
const byte Lm1 = L - 1;
const byte Lm2 = L - 2;
//unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long Baud = 476000;

const char Ident[4] = {'C', 'T', 'R', 'L'};

byte Chan;
bool canProceed_ADC, canProceed_DAC;

const unsigned long PollPeriod = 4000; //us
unsigned long Now, LastPoll;


void setup()
{
  SetupPins(); // must do this first to ensure CS_ are pulled high before starting SPI

  SPI.begin();

  // Configure all ADS1118s simultaneously
  ConfigureADS1118s();

  // Configure all AD5689Rs simultaneously
  ConfigureAD5689Rs();

  SerialSetup();
}


void SetupPins()
{
  // ADC
  pinMode(Pin_CS_ADC[0], OUTPUT);
  pinMode(Pin_CS_ADC[1], OUTPUT);
  pinMode(Pin_DRDY, INPUT);
  digitalWrite(Pin_CS_ADC[0], HIGH);
  digitalWrite(Pin_CS_ADC[1], HIGH);

  // DAC
  pinMode(Pin_CS_DAC[0], OUTPUT);
  pinMode(Pin_CS_DAC[1], OUTPUT);
  pinMode(Pin_CS_DAC[2], OUTPUT);
  pinMode(Pin_CS_DAC[3], OUTPUT);
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);
}


void loop()
{
  // Poll DRDY_ state. This chip requires CS low beforehand
  Now = micros();
  if ((Now - LastPoll) > PollPeriod)
  {
    LastPoll = Now;

    SPI.beginTransaction(ADS1118);
    digitalWrite(Pin_CS_ADC[0], LOW);
    __asm__("nop\n\t""nop\n\t""nop\n\t"); // 188ns delay at 16MHz (100ns minimum required)
    if (digitalRead(Pin_DRDY) == LOW)
    {
      DRDY_low();
    }
    else
    {
      digitalWrite(Pin_CS_ADC[0], HIGH);
      SPI.endTransaction();
    }
  }

  SerialLoop();
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


void DRDY_low()
{
  // Note that CS for chip 0 is already low at this stage, and SPI transaction is ready

  CurrentChan = ADC_Chan;
  ++ADC_Chan %= nChans_ADC;

  DataReg  =  static_cast<word>(SPI.transfer(ConfigMSB[ADC_Chan])) << 8;
  DataReg |=  static_cast<word>(SPI.transfer(ConfigLSB[ADC_Chan]));

  digitalWrite(Pin_CS_ADC[0], HIGH);
  SPI.endTransaction();

  // Convert two's complement to float
  if ((DataReg & 0b1000000000000000) == 0)
  {
    // positive, straight convert
    ADCvolts[0][CurrentChan] = static_cast<float>(DataReg) * invRes;
  }
  else
  {
    // negative, two's complement
    ADCvolts[0][CurrentChan] = -static_cast<float>((DataReg ^ 0b1111111111111111) + 1U) * invRes;
  }

  // REPEAT THE ABOVE FOR REMAINING CHIPS ##################################################################################################

  SPI.beginTransaction(ADS1118);
  digitalWrite(Pin_CS_ADC[1], LOW);
  __asm__("nop\n\t""nop\n\t""nop\n\t"); // 188ns delay at 16MHz (100ns minimum required)
  if (digitalRead(Pin_DRDY) == LOW)
  {
    DataReg  =  static_cast<word>(SPI.transfer(ConfigMSB[ADC_Chan])) << 8;
    DataReg |=  static_cast<word>(SPI.transfer(ConfigLSB[ADC_Chan]));

    digitalWrite(Pin_CS_ADC[1], HIGH);
    SPI.endTransaction();

    // Convert two's complement to float
    if ((DataReg & 0b1000000000000000) == 0)
    {
      // positive, straight convert
      ADCvolts[1][CurrentChan] = static_cast<float>(DataReg) * invRes;
    }
    else
    {
      // negative, two's complement
      ADCvolts[1][CurrentChan] = -static_cast<float>((DataReg ^ 0b1111111111111111) + 1U) * invRes;
    }
  }
  else
  {
    digitalWrite(Pin_CS_ADC[1], HIGH);
    SPI.endTransaction();
  }
}


void DAC_Output(int DAC_Chip, int DAC_Chan, float Value)
{
  DACval = static_cast<word>(constrain(static_cast<long>(floor((ConvFact * Value) + 0.5)), 0, 65535));

  SPI.beginTransaction(AD5689R);

  digitalWrite(Pin_CS_DAC[DAC_Chip], LOW);
  SPI.transfer(INPUTREG | AddressBits[DAC_Chan]);
  SPI.transfer(static_cast<byte>((DACval & 0b1111111100000000) >> 8));
  SPI.transfer(static_cast<byte>(DACval & 0b0000000011111111));
  digitalWrite(Pin_CS_DAC[DAC_Chip], HIGH);

  SPI.endTransaction();
}


void DAC_Output_ALL(float Value)
{
  for (int i = 0; i < nChips_DAC; ++i)
  {
    DAC_Output(i, 2, Value); // DAC_Chan 2 is special case of "set both channels together"
  }
}


void ConfigureADS1118s()
{
  // Set channel variable to 0
  ADC_Chan = 0;

  SPI.beginTransaction(ADS1118);
  digitalWrite(Pin_CS_ADC[0], LOW);
  digitalWrite(Pin_CS_ADC[1], LOW);

  SPI.transfer(ConfigMSB[ADC_Chan]);
  SPI.transfer(ConfigLSB[ADC_Chan]);

  digitalWrite(Pin_CS_ADC[0], HIGH);
  digitalWrite(Pin_CS_ADC[1], HIGH);
  SPI.endTransaction();
}


void ConfigureAD5689Rs()
{
  ResetAD5689Rs();

  SPI.beginTransaction(AD5689R);

  digitalWrite(Pin_CS_DAC[0], LOW);
  digitalWrite(Pin_CS_DAC[1], LOW);
  digitalWrite(Pin_CS_DAC[2], LOW);
  digitalWrite(Pin_CS_DAC[3], LOW);
  SPI.transfer(0b10000000); // 1000 Daisy chain cmd
  SPI.transfer(0b00000000); // don't care bits
  SPI.transfer(0b00000000); // 0(LSB) standalone mode
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);

  digitalWrite(Pin_CS_DAC[0], LOW);
  digitalWrite(Pin_CS_DAC[1], LOW);
  digitalWrite(Pin_CS_DAC[2], LOW);
  digitalWrite(Pin_CS_DAC[3], LOW);
  SPI.transfer(0b01000000); // 0100 power up/down DAC cmd
  SPI.transfer(0b00000000); // don't-care bits
  SPI.transfer(0b00111100); // 00 DACB normal operation, 1111, 00 DACA normal operation
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);

  digitalWrite(Pin_CS_DAC[0], LOW);
  digitalWrite(Pin_CS_DAC[1], LOW);
  digitalWrite(Pin_CS_DAC[2], LOW);
  digitalWrite(Pin_CS_DAC[3], LOW);
  SPI.transfer(0b01010000); // LDAC_ mask register
  SPI.transfer(0b00000000); // don't-care bits
  SPI.transfer(0b00000000); // LDAC_ normal operation
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);

  digitalWrite(Pin_CS_DAC[0], LOW);
  digitalWrite(Pin_CS_DAC[1], LOW);
  digitalWrite(Pin_CS_DAC[2], LOW);
  digitalWrite(Pin_CS_DAC[3], LOW);
  SPI.transfer(0b01110000); // Internal reference cmd
  SPI.transfer(0b00000000); // don't-care bits
  SPI.transfer(0b00000000); // 0(LSB) int. ref. on
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);

  SPI.endTransaction();
}


void ResetAD5689Rs()
{
  SPI.beginTransaction(AD5689R);

  digitalWrite(Pin_CS_DAC[0], LOW);
  digitalWrite(Pin_CS_DAC[1], LOW);
  digitalWrite(Pin_CS_DAC[2], LOW);
  digitalWrite(Pin_CS_DAC[3], LOW);
  SPI.transfer(0b01100000); // Software reset cmd
  SPI.transfer(0b00000000); // don't-care bits
  SPI.transfer(0b00000000); // don't-care bits
  digitalWrite(Pin_CS_DAC[0], HIGH);
  digitalWrite(Pin_CS_DAC[1], HIGH);
  digitalWrite(Pin_CS_DAC[2], HIGH);
  digitalWrite(Pin_CS_DAC[3], HIGH);

  SPI.endTransaction();
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
          // Only proceed is specified channel is within range of device
          Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
          canProceed_ADC = (Chan > 0) && (Chan <= TotalChannels_ADC);
          canProceed_DAC = (Chan >= 0) && (Chan <= TotalChannels_DAC);
          switch (Buffer[3])
          {
            case 'A':
              // Analogue
              // check whether read or write
              switch (Buffer[4])
              {
                case 'R':
                  // Read from ADS1118 ##################
                  if (canProceed_ADC)
                  {
                    ValFloatToBytes(ADCvolts[VirtualChannel_Chip_ADC[Chan]][VirtualChannel_Chan_ADC[Chan]]); // direct to comms Buffer[6...9]
                  }
                  else
                  {
                    ErrOut();
                  }
                  // ########################
                  break;
                case 'W':
                  // Write to AD5689R ##################
                  if (canProceed_DAC)
                  {
                    if (Chan != 0) DAC_Output(VirtualChannel_Chip_DAC[Chan], VirtualChannel_Chan_DAC[Chan], ValBytesToFloat()); else DAC_Output_ALL(ValBytesToFloat());
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
                case 'R':
                  // Read
                  // ########################
                  if (Chan == 0) GetConfig(); else ErrOut();
                  // ########################
                  break;
                case 'W':
                  // Write
                  // ########################
                  if (Chan == 255 || Chan == 0) SetConfig(); else ErrOut();
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
  }
}


void GetConfig()
{
  const float val = static_cast<float>(EEPROM.read(Chan));

  ValFloatToBytes(val);
}


void GetIdent()  // direct to comms buffer
{
  memcpy(&Buffer[6], &Ident, 4);
}


inline byte CalcChecksum()
{
  byte sum = 0;
  for (byte i = 0; i < Lm2; i++)
  {
    sum += static_cast<byte>(Buffer[i]);
  }
  return sum;
}


inline void ErrOut()
{
  ValFloatToBytes(-9999);
}


float ValBytesToFloat() // direct from comms buffer
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


void ValFloatToBytes(float val) // direct to comms buffer
{
  const char *val_bytes = reinterpret_cast<char*>(&val);
  Buffer[6] = val_bytes[3];
  Buffer[7] = val_bytes[2];
  Buffer[8] = val_bytes[1];
  Buffer[9] = val_bytes[0];
}

