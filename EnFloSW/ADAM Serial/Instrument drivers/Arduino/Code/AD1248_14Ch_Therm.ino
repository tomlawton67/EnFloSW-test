// Developed and tested by Paul Nathan. Last modification 11/12/2013

#include <SPI.h>
#include <EEPROM.h>


// Thermistor variables
const float invB = 1.0 / 4334.0;
const float R2 = 100000.0;
const float R10 = 100000.0;
const float invT0 = 1.0 / 298.15;
const float Vs = 2.048; // hard-wired to Vref
float R1;


// ADS1248 variables
int Pin_CS[2] = {
  10, 12};
int Pin_DRDY0 = 11;

const byte NOP = 0xFF;
const byte RDATA = 0x12;
const byte SDATAC = 0x16;
const byte WREG = 0x40;
const byte SELFOCAL = 0x62;
const byte SYNC = 0x04;
const byte RESET = 0x06;
//const byte RREG = 0x20;
//const byte RDATAC = 0x14;

unsigned long DataReg, lastAutoCalib_ms, now_ms;
float ADCvolts[2][7], Volts, Temperature;
char *TemperatureBytes;
int Chan;
int currentChan;
const int nChans = 7; // per chip, single-ended
const byte ChannelBits[7] = { 
  0b00001000, 0b00010000, 0b00011000, 0b00100000, 0b00101000, 0b00110000, 0b00111000}; // Single-ended with AIN0 as common ref

const unsigned long AutoCalib_ms = 3600000; // auto-calib offset every hour
const float Vref = 2.048; // Manually input Vref (nominal value from spec sheet appears to match voltmeter value over a range of chips!)


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
  ADS1248Setup();

  SerialSetup();
}


void loop()
{
  // Interrogate DRDY pin. If it has gone low, process sample
  // CHIP 0 as master DRDY after SYNC. Do all chips sequentially if DRDY0 goes low
  if ((PINB & Pin_DRDY0) >> PINB7 == 0) DRDY_low();

  SerialLoop();

  // Check last auto-offset calib time and do it if elapsed
  now_ms = millis();
  if ((now_ms - lastAutoCalib_ms) > AutoCalib_ms)
  {
    lastAutoCalib_ms = now_ms;
    // re-configure all chips together
    ConfigureADS1248s;
  }
}


// SLAVE CODE (ARDUINO MICRO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D, R/W, pin, val, val ,val, val, rxCHK, txCHK

void SerialSetup()
{
  // Configure pins
  PinTx_EN = 13;
  Pin_frm = 9;
  Pin_chk = 5;
  Pin_buf = 3;

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
  Pin_frm = (1 << PINB5);
  Pin_chk = (1 << PINC6);
  Pin_buf = (1 << PIND0);

  //txDelay = (8 * 1000000) / baud; //us

  Serial1.begin(baud); // send/receive messages to/from RS-485 network. MUST USE SERIAL1 ON MICRO!! Not Serial (which is USB)
}


void ADS1248Setup()
{
  // Setup pins
  pinMode(Pin_CS[0], OUTPUT);
  pinMode(Pin_CS[1], OUTPUT);
  pinMode(Pin_DRDY0, INPUT);

  digitalWrite(Pin_CS[0], HIGH);
  digitalWrite(Pin_CS[1], HIGH);

  // SPI comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  // Set Pin numbers to port bytes for speed
  Pin_CS[0] = (1 << PINB6);
  Pin_CS[1] = (1 << PIND6);
  Pin_DRDY0 = (1 << PINB7);

  // Configure all ADS1248s simultaneously
  ConfigureADS1248s();

  lastAutoCalib_ms = millis();
}


void DRDY_low()
{
  PORTB &= ~Pin_CS[0]; // LOW

  SPI.transfer(RDATA); // load conversion result to output register

  // Make use of full-duplex nature of SPI to switch to next channel in sequence while reading output register
  // cycle to next channel value, storing current one for Value array
  currentChan = Chan;
  ++Chan %= nChans;

  DataReg =  static_cast<unsigned long>(SPI.transfer(WREG | 0x00)) << 16; // read high byte and shift +16, also write to MUX0 reg
  DataReg |= static_cast<unsigned long>(SPI.transfer(0x00)) << 8; // read mid byte and shift +8 and biwise OR, also prepare 1 byte write
  DataReg |= static_cast<unsigned long>(SPI.transfer(ChannelBits[Chan])); // read low byte and bitwise OR, also write reg config
  SPI.transfer(NOP);

  PORTB |= Pin_CS[0]; // HIGH

  // Convert two's complement to straight binary and then to negated float
  DataReg = -(~DataReg + 1UL);
  ADCvolts[0][currentChan] = Vref * (float(DataReg) / 8388607.0); // (Vref / 1) scaling hard coded for Gain=1

  // REPEAT THE ABOVE FOR REMAINING CHIPS ##################################################################################################

  PORTD &= ~Pin_CS[1]; // LOW

  SPI.transfer(RDATA); 

  DataReg =  static_cast<unsigned long>(SPI.transfer(WREG | 0x00)) << 16;
  DataReg |= static_cast<unsigned long>(SPI.transfer(0x00)) << 8; 
  DataReg |= static_cast<unsigned long>(SPI.transfer(ChannelBits[Chan])); 
  SPI.transfer(NOP);

  PORTD |= Pin_CS[1]; // HIGH

  DataReg = -(~DataReg + 1UL);
  ADCvolts[1][currentChan] = Vref * (float(DataReg) / 8388607.0); 
}


void ConfigureADS1248s()
{
  PORTB &= ~Pin_CS[0]; // ALL LOW
  PORTD &= ~Pin_CS[1];

  // Stop continuous sampling (if necessary), only sample on demand and don't auto-replace output shift register with newest sample on DRDY low
  SPI.transfer(SDATAC);

  // Write desired config to registers
  // Skip register 0x00 MUX0, will select channel later on
  // write to registers 0x01 to 0x03, VBIAS, MUX1, SYS0
  SPI.transfer(WREG | 0x01);
  SPI.transfer(0x00 | 0x02); 
  // Register 0x01 VBIAS. No bias on any AIN, so set all to zero
  SPI.transfer(0b00000000);
  // Register 0x02 MUX1. Internal osc 0, Internal Vref always on 01, Onboard ref is used 10, sys monitor off 000
  SPI.transfer(0b00110000);
  // Register 0x03 SYS0. 0, Gain=1 000, Data rate=20Hz 0010
  SPI.transfer(0b00000010);
  // Skip registers 0x04 - 0x06 OFC coefficients to be set by self calib. later
  // Skip registers 0x07 - 0x09 FSC coefficients factory set!
  SPI.transfer(NOP);

  SPI.transfer(WREG | 0x0A);
  SPI.transfer(0x00 | 0x02); // write to the next 3 registers, IDAC0, IDAC1, GPIOCFG
  // Register 0x0A IDAC0. 0000, DRDY pin as Dout only 0, Current source magnitude zero 000
  SPI.transfer(0b00000000);
  // Register 0x0B IDAC1. Current source 1 disconnected 1100, current source 2 disconnected 1100
  SPI.transfer(0b11001100);
  // Register 0x0C GPIOCFG. Use all pins as analog input
  SPI.transfer(0b00000000);
  // Final Registers 0x0D and 0x0E can be skipped due to the above GPIO config. 
  SPI.transfer(NOP);

  // per-channel self offset calib
  for (int i = 0; i < nChans; i++)
  {
    // write to register 0x00 MUX0 to select channel to calib.
    SPI.transfer(WREG | 0x00);
    SPI.transfer(0x00 | 0x00);
    // Register 0x00 MUX0. Burnout current off 00, AIN+, AIN-. Preconfigured in ChannelBits variable for quick cycling
    SPI.transfer(ChannelBits[i]);
    SPI.transfer(NOP);

    // Now perform self offset calibration
    SPI.transfer(SELFOCAL);
    // waiting for DRDY to return low with first new valid sample naturally occurs in polling loop
    // so no need to hold it here and delay other comms.
  }

  // Reset channel variable so it corresponds with ADC MUX
  Chan = 0;
  // write to register 0x00 MUX0 to select channel
  SPI.transfer(WREG | 0x00);
  SPI.transfer(0x00 | 0x00);
  // Register 0x00 MUX0. Select channel "0"
  SPI.transfer(ChannelBits[0]);
  SPI.transfer(NOP);

  // synchronise ADC digital filters on all chips
  SPI.transfer(SYNC);
  SPI.transfer(SYNC);

  PORTB |= Pin_CS[0]; // ALL HIGH
  PORTD |= Pin_CS[1];
}


inline float VoltstoTemp(float& V)
{
  R1 = ((Vs / V) - 1.0) * R2;
  return ((1.0 / ((invB * log(R1 / R10)) + invT0)) - 273.15); // degC
}


inline void SerialLoop()
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
          // For ADS7148 only care if Analogue Read is requested
          switch (Buffer[3])
          {
          case 'A':
            // Analogue
            // check whether read or write
            switch (Buffer[4])
            {
            case 'R':
              // Read ##################
              ReadFromADS1248();
              Temperature = VoltstoTemp(Volts);
              TemperatureBytes = reinterpret_cast<char*>(&Temperature);
              Buffer[6] = TemperatureBytes[3];
              Buffer[7] = TemperatureBytes[2];
              Buffer[8] = TemperatureBytes[1];
              Buffer[9] = TemperatureBytes[0];
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
          PORTB &= ~Pin_frm; // Successful msg cycle, reset any error lights
          PORTC &= ~Pin_chk;
          PORTD &= ~Pin_buf;
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
          PORTC |= Pin_chk; // turn on bad checksum LED
        }
      }
    }
    else
    {
      Serial1.read(); // discard invalid byte
      PORTB |= Pin_frm; // warn of invalid byte(s), turn on frm LED
    }
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


void ReadFromADS1248() // Map physical channels to incremental 'software' channels
{
  switch (static_cast<int>(static_cast<byte>(Buffer[5])))
  {
  case 0: // special channel called to initiate reset and offset self-calibration
    // Reset ADCs, send RESET command and re-configure
    PORTB &= ~Pin_CS[0]; // ALL LOW
    PORTD &= ~Pin_CS[1];

    SPI.transfer(RESET);
    delay(1); // 0.6ms fixed reset time. Can use simple 'delay' function here as nothing else important should be happening anyway, and interrupts are not disabled so comms ok.

    PORTB |= Pin_CS[0]; // ALL HIGH
    PORTD |= Pin_CS[1];

    ConfigureADS1248s();
    break;
  case 1:
    Volts = ADCvolts[0][0];
    break;
  case 2:
    Volts = ADCvolts[0][3];
    break;
  case 3:
    Volts = ADCvolts[0][4];
    break;
  case 4:
    Volts = ADCvolts[0][2];
    break;
  case 5:
    Volts = ADCvolts[0][1];
    break;
  case 6:
    Volts = ADCvolts[0][6];
    break;
  case 7:
    Volts = ADCvolts[0][5];
    break;
  case 8:
    Volts = ADCvolts[1][4];
    break;
  case 9:
    Volts = ADCvolts[1][3];
    break;
  case 10:
    Volts = ADCvolts[1][0];
    break;
  case 11:
    Volts = ADCvolts[1][5];
    break;
  case 12:
    Volts = ADCvolts[1][6];
    break;
  case 13:
    Volts = ADCvolts[1][1];
    break;
  case 14:
    Volts = ADCvolts[1][2];
    break;
  default: 
    Volts = 0.0;
  }
}



















