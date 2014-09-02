// Code for 16 channel 16-bit DAC over RS-485
// Developed and tested by Paul Nathan. Last modification 19/06/2014

#include <SPI.h>
#include <EEPROM.h>


// AD5754R variables
const byte NOP = 0b00011000; // followed by two more 8-bit writes of "don't care" values
const byte CLEAR = 0b00011100; // followed by two more 8-bit writes of "don't care" values
//const byte LOAD = 0b00011101; // followed by two more 8-bit writes of "don't care" values
const byte CTRL = 0b00011001; // followed by specific config bits, (first 12 are "don't care")
const byte PWR = 0b00010000; // followed by specific config bits, (first 5 are "don't care")
const byte OPRANGE = 0b00001000;  // OR with ChannelBits and then follow by specific config bits, (first 13 are "don't care")
const byte DACregWRITE = 0b00000000;  // OR with ChannelBits and then follow by two more 8-bit writes of DATA

const int nChips = 4; // number of DAC chips
const int nChannelsPerChip = 4; // one DAC chip
const byte ChannelBits[nChannelsPerChip + 1] = {0b00000100, 0b00000000, 0b00000001, 0b00000010, 0b00000011}; // first value is AllChannels, then DAC1...4

const int TotalChannels = nChannelsPerChip * nChips;
int Pin_CS[nChips] = {A3, A2, A1, A0};

float DAC_Vout[TotalChannels + 1]; // store voltage demand for query

int Chan;
bool canProceed;

// LUTs for mapping channel number to chip number and individual chip channel number. Hard coded for 4x4 config. (+1 size for 1 based indexing of channels)
//                                      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16
const int ChipNum[TotalChannels + 1] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
const int ChanNum[TotalChannels + 1] = {0, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
//unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long baud = 476000;

const char Ident[4] = {'D', 'A', 'C', '_'};


void setup()
{
  AD5754RSetup();

  SerialSetup();
}


void loop()
{
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

  // Set Pin numbers to port bytes for direct register manipulation
  PinTx_EN = (1 << PINC7);
  Pin_frm = (1 << PIND6);
  Pin_chk = (1 << PINB7);
  Pin_buf = (1 << PINB6);

  //txDelay = (8 * 1000000) / baud; //us

  Serial1.begin(baud); // send/receive messages to/from RS-485 network. MUST USE SERIAL1 ON MICRO!! Not Serial (which is USB)
}


void AD5754RSetup()
{
  // Setup pins
  for (int i = 0; i < nChips; i++)
  {
    pinMode(Pin_CS[i], OUTPUT);
    digitalWrite(Pin_CS[i], HIGH);
  }

  // SPI comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  // Set Pin numbers to port bytes for direct register manipulation
  Pin_CS[0] = (1 << PINF4);
  Pin_CS[1] = (1 << PINF5);
  Pin_CS[2] = (1 << PINF6);
  Pin_CS[3] = (1 << PINF7);

  // Configure AD5754R. All chips done inside subroutine
  ConfigureAD5754R(); // sets output range to values last stored in EEPROM
}


void ConfigureAD5754R()
{
  // Configure all N Chips
  for (int i = 0; i < nChips; i++)
  {
    // Configure all n DAC channels

    // first configure the control register
    PORTF &= ~Pin_CS[i]; // Low
    SPI.transfer(CTRL);
    SPI.transfer(0x00);
    SPI.transfer(0b00000101); // 0000, 0 TSD disabled, 1 CLAMP enabled, 0 CLR to 0V, 1 SDOUT disabled
    PORTF |= Pin_CS[i]; // High

    // configure output range register, do each channel
    for (int j = 1; j <= nChannelsPerChip; j++) // 1...nChan rather than 0...(nChan-1) because 0 is reserved for "all channels"
    {
      ConfigureAD5754R_Vmax(i, j);
    }

    // configure power register
    PORTF &= ~Pin_CS[i]; // Low
    SPI.transfer(PWR);
    SPI.transfer(0b00000000); // 00000, {0 0 0} = {OCD OCC OCB}(read only bits)
    SPI.transfer(0b00011111); // {0 0 0} = {OCA 0 TSD}(read only bits), {1 1 1 1 1} = {PUref, PUD, PUC, PUB, PUA} all enabled
    PORTF |= Pin_CS[i]; // High

    // finally set all 4 channels to CLR code (i.e. reset to 0V)
    PORTF &= ~Pin_CS[i]; // Low
    SPI.transfer(CLEAR);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    PORTF |= Pin_CS[i]; // High
  }
}


void ConfigureAD5754R_Vmax(int Chipnum, int Chan_individual)
{
  byte VmaxByte; // 00000, 0 0 0 = +5V (Gain of 2 on 2.5V Vref). For +10V set 0 0 1
  const int Channel = Chipnum * nChannelsPerChip + Chan_individual;

  // read Channel VmaxSwitch from EEPROM and set channel config. as appropriate
  if (EEPROM.read(Channel) == 0)
    VmaxByte = 0b00000000;
  else
    VmaxByte = 0b00000001;

  PORTF &= ~Pin_CS[Chipnum]; // Low
  SPI.transfer(OPRANGE | ChannelBits[Chan_individual]);
  SPI.transfer(0x00);
  SPI.transfer(VmaxByte);
  PORTF |= Pin_CS[Chipnum]; // High
}


inline void SerialLoop()
{
  // Check if message waiting in RS-485 transducer buffer
  // if so, construct variable buffer, implement request, and send output back over RS-485 to master
  // first check for buffer overrun, if so, turn on warning LED 'buf'
  if (Serial1.available() > 63) PORTB |= Pin_buf;
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
          // For AD5754R analogue write to set DAC values, digital write to set relays
          // Only proceed is specified channel is within range of device
          Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
          canProceed = (Chan >= 0) && (Chan <= TotalChannels);
          switch (Buffer[3])
          {
            case 'A':
              // Analogue
              // check whether read or write
              switch (Buffer[4])
              {
                case 'W':
                  // Write
                  // ########################
                  if (canProceed)
                  {
                    if (Chan != 0) DACwriteVal(); else DACwriteVal_ALL();
                  }
                  else ErrOut();
                  // ########################
                  break;
                case 'R':
                  // Read
                  // ########################
                  if (canProceed) DACreadVal(); else ErrOut();
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
                  if (canProceed || Chan == 255) SetConfig(); else ErrOut();
                  // ########################
                  break;
                case 'R':
                  // Read
                  // ########################
                  if (canProceed) GetConfig(); else ErrOut();
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


inline void DACwriteVal()
{
  // Using global channel number, convert value from raw bytes to float, convert to binary word and output
  const float val = ValBytesToFloat();
  const word DACval = static_cast<word>(constrain(static_cast<long>(floor((val * 65535) + 0.5)), 0, 65535));

  PORTF &= ~Pin_CS[ChipNum[Chan]]; // Low
  SPI.transfer(DACregWRITE | ChannelBits[ChanNum[Chan]]);
  SPI.transfer(static_cast<byte>((DACval & 0b1111111100000000) >> 8)); // High byte
  SPI.transfer(static_cast<byte>(DACval & 0b0000000011111111)); // Low byte
  PORTF |= Pin_CS[ChipNum[Chan]]; // High

  // Update channel values for query
  DAC_Vout[Chan] = val;
}


inline void DACwriteVal_ALL()
{
  // Loop through all chips, convert value from raw bytes to float, convert to binary word and output
  int i;
  const float val = ValBytesToFloat();
  const word DACval = static_cast<word>(constrain(static_cast<long>(floor((val * 65535) + 0.5)), 0, 65535));

  for (i = 0; i < nChips; i++)
  {
    PORTF &= ~Pin_CS[i]; // Low
    SPI.transfer(DACregWRITE | ChannelBits[0]);
    SPI.transfer(static_cast<byte>((DACval & 0b1111111100000000) >> 8)); // High byte
    SPI.transfer(static_cast<byte>(DACval & 0b0000000011111111)); // Low byte
    PORTF |= Pin_CS[i]; // High
  }

  // Update channel values for query
  for (i = 1; i <= TotalChannels; i++)
  {
    DAC_Vout[i] = val;
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

  // Chan 0 sets DAC Vmax for all channels together
  // Chan 1...N sets DAC Vmax for that channel alone
  // in all of the above cases val=1 sets Vmax=10V, val=0 sets Vmax=5V

  if (Chan == 0) // all channels the same (over all chips)
  {
    for (int i = 1; i <= TotalChannels; i++)
    {
      EEPROM.write(i, val);
      ConfigureAD5754R_Vmax(ChipNum[i], ChanNum[i]);
    }
  }
  else
  {
    EEPROM.write(Chan, val);
    ConfigureAD5754R_Vmax(ChipNum[Chan], ChanNum[Chan]);
  }
}


void GetConfig()
{
  const float val = static_cast<float>(EEPROM.read(Chan));

  ValFloatToBytes(val);
}


inline void DACreadVal()
{
  float mult;

  (EEPROM.read(Chan) == 0) ? (mult = 5.0) : (mult = 10.0);
  ValFloatToBytes(DAC_Vout[Chan] * mult);
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
