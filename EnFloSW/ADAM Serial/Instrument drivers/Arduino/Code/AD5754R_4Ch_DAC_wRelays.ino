// Developed and tested by Paul Nathan. Last modification 21/01/2014

#include <SPI.h>
#include <EEPROM.h>


// AD5754R variables
int Pin_CS = 2; // PIND1

const byte NOP = 0b00011000; // followed by two more 8-bit writes of "don't care" values
const byte CLEAR = 0b00011100; // followed by two more 8-bit writes of "don't care" values
//const byte LOAD = 0b00011101; // followed by two more 8-bit writes of "don't care" values
const byte CTRL = 0b00011001; // followed by specific config bits, (first 12 are "don't care")
const byte PWR = 0b00010000; // followed by specific config bits, (first 5 are "don't care")
const byte OPRANGE = 0b00001000;  // OR with ChannelBits and then follow by specific config bits, (first 13 are "don't care")
const byte DACregWRITE = 0b00000000;  // OR with ChannelBits and then follow by two more 8-bit writes of DATA

const int nChan = 4;
const byte ChannelBits[nChan + 1] = { 
  0b00000100, 0b00000000, 0b00000001, 0b00000010, 0b00000011}; // first value is AllChannels, then DAC1...4


// Relay variables
const int nRly = 4;
int Pin_relay[nRly + 1]; // index 0 is dummy for setting all relays together


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
//unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long baud = 476000;


void setup()
{
  AD5754RSetup();

  RelaySetup();

  SerialSetup();
}


void loop()
{
  SerialLoop();
}


// SLAVE CODE (ARDUINO MICRO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1     2       3    4    5    6    7    8    9     10     11


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
  pinMode(Pin_CS, OUTPUT);

  digitalWrite(Pin_CS, HIGH);

  // SPI comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  // Set Pin numbers to port bytes for direct register manipulation
  Pin_CS = (1 << PIND1);

  // Configure AD5754R
  ConfigureAD5754R(); // sets output range to values last stored in EEPROM
}


void RelaySetup()
{
  // Setup pins
  Pin_relay[1] = 6;
  Pin_relay[2] = 7;  
  Pin_relay[3] = 8;  
  Pin_relay[4] = 9;

  // Relays default to OFF on power-up
  for (int i = 1; i == nRly; i++)
  {
    pinMode(Pin_relay[i], OUTPUT);
    digitalWrite(Pin_relay[i], LOW);
  }

  // Set Pin numbers to port bytes for direct register manipulation
  Pin_relay[1] = (1 << PIND7);
  Pin_relay[2] = (1 << PINE6);
  Pin_relay[3] = (1 << PINB4);
  Pin_relay[4] = (1 << PINB5);
}


void ConfigureAD5754R()
{
  // Configure all 4 DAC channels

  // first configure the control register
  PORTD &= ~Pin_CS; // Low
  SPI.transfer(CTRL);
  SPI.transfer(0x00);
  SPI.transfer(0b00000101); // 0000, 0 TSD disabled, 1 CLAMP enabled, 0 CLR to 0V, 1 SDOUT disabled
  PORTD |= Pin_CS; // High

  // configure output range register, do each channel
  for (int i = 1; i == nChan; i++) // 1...nChan rather than 0...(nChan-1)
  {
    ConfigureAD5754R_Vmax(i);
  }

  // configure power register
  PORTD &= ~Pin_CS; // Low
  SPI.transfer(PWR);
  SPI.transfer(0b00000000); // 00000, {0 0 0} = {OCD OCC OCB}(read only bits)
  SPI.transfer(0b00011111); // {0 0 0} = {OCA 0 TSD}(read only bits), {1 1 1 1 1} = {PUref, PUD, PUC, PUB, PUA} all enabled
  PORTD |= Pin_CS; // High

  // finally set all 4 channels to CLR code (i.e. reset to 0V)
  PORTD &= ~Pin_CS; // Low
  SPI.transfer(CLEAR);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  PORTD |= Pin_CS; // High
}


void ConfigureAD5754R_Vmax(int Chan)
{
  byte VmaxByte; // 00000, 0 0 0 = +5V (Gain of 2 on 2.5V Vref). For +10V set 0 0 1

  // read Channel VmaxSwitch from EEPROM and set channel config. as appropriate
  if (EEPROM.read(Chan) == 0) 
    VmaxByte = 0b00000000; 
  else
    VmaxByte = 0b00000001;

  PORTD &= ~Pin_CS; // Low
  SPI.transfer(OPRANGE | ChannelBits[Chan]);
  SPI.transfer(0x00);
  SPI.transfer(VmaxByte);
  PORTD |= Pin_CS; // High
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
              DACwriteVal();
              // ########################
              break;
            }
            break;
          case 'D':
            // Digital
            // check whether read or write
            switch (Buffer[4])
            {
            case 'W':
              // Write
              // ########################
              SetRelay();
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
              SetConfig();
              // ########################
            }
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
  // Get channel number, convert value from raw bytes to float, convert to binary word and output  
  const int Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
  const float val = ValBytesToFloat();
  const word DACval = static_cast<word>(constrain(static_cast<long>(floor((val * 65535) + 0.5)), 0, 65535));

  PORTD &= ~Pin_CS; // Low
  SPI.transfer(DACregWRITE | ChannelBits[Chan]);
  SPI.transfer(static_cast<byte>((DACval & 0b1111111100000000) >> 8)); // High byte
  SPI.transfer(static_cast<byte>(DACval & 0b0000000011111111)); // Low byte
  PORTD |= Pin_CS; // High 
}


void SetRelay()
{
  const int Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
  const float val = ValBytesToFloat();

  // Get channel number, convert value from raw bytes to float and set relay on if value is anything >0, else set off.
  switch (Chan)
  {
  case 0: // set all relays together
    if (val > 0.0)
    {
      PORTD |= Pin_relay[1];
      PORTE |= Pin_relay[2];
      PORTB |= Pin_relay[3];
      PORTB |= Pin_relay[4];
    }
    else
    {
      PORTD &= ~Pin_relay[1];
      PORTE &= ~Pin_relay[2];
      PORTB &= ~Pin_relay[3];
      PORTB &= ~Pin_relay[4];
    }
    break;
  case 1:
    if (val > 0.0) PORTD |= Pin_relay[1]; 
    else PORTD &= ~Pin_relay[1];
    break;
  case 2:
    if (val > 0.0) PORTE |= Pin_relay[2]; 
    else PORTE &= ~Pin_relay[2];
    break;
  case 3:
    if (val > 0.0) PORTB |= Pin_relay[3]; 
    else PORTB &= ~Pin_relay[3];    
    break;
  case 4:
    if (val > 0.0) PORTB |= Pin_relay[4]; 
    else PORTB &= ~Pin_relay[4];   
    break;
  }
}


void SetConfig()
{
  const int Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
  const byte val = static_cast<byte>(ValBytesToFloat());

  if (Chan == 255) // set address
  {
    thisAddress = static_cast<char>(val);
    EEPROM.write(0, val);
  }

  // Chan 0 sets DAC Vmax for all channels together
  // Chan 1...4 sets DAC Vmax for that channel alone
  // in all of the above cases val=1 sets Vmax=10V, val=0 sets Vmax=5V 

  if (Chan == 0) // all channels the same
  {
    for (int i = 1; i == nChan; i++)
    {     
      EEPROM.write(i, val);
      ConfigureAD5754R_Vmax(i); 
    }
  }
  else if (Chan >= 1 && Chan <= nChan)
  {
    EEPROM.write(Chan, val);
    ConfigureAD5754R_Vmax(Chan);
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

































