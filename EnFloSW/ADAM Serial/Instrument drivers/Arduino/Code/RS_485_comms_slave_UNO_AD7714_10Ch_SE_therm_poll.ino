// Developed by Paul Nathan, last mod. 04/02/2014

#include <SPI.h>
#include <EEPROM.h>

// Thermistor variables
const float invB = 1.0 / 4334.0;
const float R2 = 100000.0;
const float R10 = 100000.0;
const float invT0 = 1.0 / 298.15;
const float Vs = 2.509; // hard-wired to Vref
float R1;


// AD7714 variables
int Pin_CS[2] = {10, 9};

unsigned long DataReg, lastAutoCalib_ms, now_ms;//, GainReg, OffsetReg;
float ADCvolts[2][5], Volts, Temperature;
char *TemperatureBytes;
int chan[2];
byte ModeReg, GainBits, FilterHighBits, FilterHighReg, FilterLowReg;
byte DRDYbit;

const int nChans = 5; // Hard-coded (SE = 5, DIFF = 3)
const byte ChannelBitsLUT_DIFF[3] = {0b00000100, 0b00000101, 0b00000110};
const byte ChannelBitsLUT_SE[5] = {0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000110};

byte ChannelBits[5]; // for easy, globally selected SE or DIFF bits. Copy either of the above into this array

unsigned long AutoCalib_ms = 3600000UL * max(1UL, static_cast<unsigned long>(EEPROM.read(1))); // reset, config, auto-calib every x milliseconds
const float Vref = 2.499; // Manually input Vref (specific to each board!)


// RS-485 variables
int PinTx_EN, Pin_frm, Pin_chk, Pin_buf;
const int L = 12; // msg length
const int Lm1 = L - 1;
const int Lm2 = L - 2;
unsigned int txDelay;
char Buffer[L], tmp;

char thisAddress = static_cast<char>(EEPROM.read(0));
const long baud = 476000;


void setup()
{
  AD7714Setup();

  SerialSetup();
}


void loop()
{
  // Check DRDY bit in comms register. If fallen then grab sample
  // Do not poll here as this would hold up serial comms!

  // CHIP 0
  PORTB &= ~Pin_CS[0]; // LOW
  SPI.transfer(0b00001000 | ChannelBits[chan[0]]);
  DRDYbit = SPI.transfer(0xFF) & 0b10000000;
  if (DRDYbit == 0) DRDY_fall(0);
  PORTB |= Pin_CS[0]; // HIGH

  // CHIP 1
  PORTB &= ~Pin_CS[1]; // LOW
  SPI.transfer(0b00001000 | ChannelBits[chan[1]]);
  DRDYbit = SPI.transfer(0xFF) & 0b10000000;
  if (DRDYbit == 0) DRDY_fall(1);
  PORTB |= Pin_CS[1]; // HIGH

  SerialLoop();

  // Check reset, config, auto-calib time
  now_ms = millis();
  if ((now_ms - lastAutoCalib_ms) > AutoCalib_ms)
  {
    lastAutoCalib_ms = now_ms;
    // Chip 0
    ConfigureAD7714(0);
    // Chip 1
    ConfigureAD7714(1);
  }
}


// SLAVE CODE (ARDUINO UNO ONLY!!)

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D/C, R/W, pin, val, val ,val, val, rxCHK, txCHK
//                                0    1     2       3    4    5    6    7    8    9     10     11


void SerialSetup()
{
  // Configure pins
  PinTx_EN = 4;
  Pin_frm = A5;
  Pin_chk = A4;
  Pin_buf = A3;

  pinMode(PinTx_EN, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A3, OUTPUT);

  digitalWrite(Pin_frm, LOW);
  digitalWrite(Pin_chk, LOW);
  digitalWrite(Pin_buf, LOW);
  digitalWrite(PinTx_EN, LOW);  // Disable transmit mode

  // Set Pin numbers to port bytes for speed
  PinTx_EN = (1 << PIND4);
  Pin_frm = (1 << PINC5);
  Pin_chk = (1 << PINC4);
  Pin_buf = (1 << PINC3);

  txDelay = (8 * 1000000) / baud; //us

  Serial.begin(baud); // send/receive messages to/from RS-485 network
}


void AD7714Setup()
{
  // configure pins
  pinMode(Pin_CS[0], OUTPUT);
  pinMode(Pin_CS[1], OUTPUT);

  digitalWrite(Pin_CS[0], HIGH);
  digitalWrite(Pin_CS[1], HIGH);

  // SPI comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4) ;

  // Set Pin numbers to port bytes for speed
  Pin_CS[0] = (1 << PINB2);
  Pin_CS[1] = (1 << PINB1);

  // configure AD7714 HARDCODED HERE, check bi/uni-polar, gain, boost, sampling rate etc ##################
  FilterHighBits = 0b00000001; // 50Hz
  FilterLowReg = 0b10000000; // 50Hz
  FilterHighReg = 0b11000000 | FilterHighBits; // Unipolar, 24-bit, Boost off (G=1)
  GainBits = 0b00000000; // 1

  ModeReg = 0b00100000 | GainBits; // self-calib then retun to normal, FSYNC off

  memcpy(&ChannelBits, &ChannelBitsLUT_SE, sizeof(ChannelBitsLUT_SE)); // Here choose SE or DIFF channelbits via copy

  ConfigureAD7714(0);
  ConfigureAD7714(1); // both chips configured identically in this device's case
  // #######################################################################################################

  lastAutoCalib_ms = millis();
}


void DRDY_fall(int ChipNum)
{
  SPI.transfer(0b01011000 | ChannelBits[chan[ChipNum]]); // set to read data register next
  DataReg = static_cast<unsigned long>(SPI.transfer(0xFF)) << 16; // read high byte and shift +16
  DataReg |= static_cast<unsigned long>(SPI.transfer(0xFF)) << 8; // read mid byte and shift +8 and biwise OR
  DataReg |= static_cast<unsigned long>(SPI.transfer(0xFF)); // read low byte and bitwise OR

  ADCvolts[ChipNum][chan[ChipNum]] = Vref * (float(DataReg) / 16777215.0); // raw voltage // Unipolar 24-bit mode only

  chan[ChipNum] += 1;
  chan[ChipNum] %= nChans;  // HARD CODED FOR SINGLE-ENDED ######

  // prepare for next channel
  SPI.transfer(0b00001000 | ChannelBits[chan[ChipNum]]);
  SPI.transfer(0xFF);
}


void ConfigureAD7714(int ChipNum)
{
  PORTB &= ~Pin_CS[ChipNum]; // LOW

  // Reset ADC. Stop sampling, drive to default with >=32 DIN highs, configure and start sampling again
  for (int i = 0; i < 32; i++)
  {
    SPI.transfer(0xFF);
  }

  // Configure channels
  for (int i = 0; i < nChans; i++)
  {
    // prepare for next channel
    SPI.transfer(0b00001000 | ChannelBits[i]);
    SPI.transfer(0xFF);
    ConfigureAD7714channel(ChannelBits[i]);

    //    if (ChipNum == 0 && i == 0)
    //    {
    //    // Read Gain Register
    //    SPI.transfer(0b01111000 | ChannelBits[i]);
    //    GainReg = static_cast<unsigned long>(SPI.transfer(0xFF)) << 16; // read high byte and shift +16
    //    GainReg |= static_cast<unsigned long>(SPI.transfer(0xFF)) << 8; // read mid byte and shift +8 and biwise OR
    //    GainReg |= static_cast<unsigned long>(SPI.transfer(0xFF)); // read low byte and bitwise OR
    //
    //    // Read Offset Register
    //    SPI.transfer(0b01101000 | ChannelBits[i]);
    //    OffsetReg = static_cast<unsigned long>(SPI.transfer(0xFF)) << 16; // read high byte and shift +16
    //    OffsetReg |= static_cast<unsigned long>(SPI.transfer(0xFF)) << 8; // read mid byte and shift +8 and biwise OR
    //    OffsetReg |= static_cast<unsigned long>(SPI.transfer(0xFF)); // read low byte and bitwise OR
    //    }

  }

  //// prepare for channel 0
  //chan[ChipNum] = 0;
  //SPI.transfer(0b00001000 | ChannelBits[0]);
  //SPI.transfer(0xFF);

  PORTB |= Pin_CS[ChipNum]; // HIGH
  //######################################
}


void ConfigureAD7714channel(byte ChanBits)
{
  // write to comms register to setup a write to the Filter registers (high then low), then to the mode register

  // Filter High register config
  SPI.transfer(0b00100000 | ChanBits);
  SPI.transfer(FilterHighReg);

  // Filter Low register config
  SPI.transfer(0b00110000 | ChanBits);
  SPI.transfer(FilterLowReg);

  // Mode register config
  SPI.transfer(0b00010000 | ChanBits);
  SPI.transfer(ModeReg);

  // The following prevents serial loop from running which causes lag.
  // unfortunately with this chip it is necessary to ensure a proper calibration!

  delay(120); // 6 / 50Hz * 1000 (ms) until calib. done, then poll DRDY (should go low after another 3/ 50Hz)

  // Poll DRDY bit in comms register
  do
  {
    SPI.transfer(0b00001000 | ChanBits);
    DRDYbit = (SPI.transfer(0xFF) & 0b10000000);
    delay(1);
  }
  while (DRDYbit == 0b10000000);
  // to get here DRDY must have fallen, discard value and move on to next calib channel
  //SPI.transfer(0b01011000 | ChanBits); // set to read data register next
  //SPI.transfer(0xFF); // read high byte and shift +16
  //SPI.transfer(0xFF); // read mid byte and shift +8 and biwise OR
  //SPI.transfer(0xFF); // read low byte and bitwise OR
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
  // first check for buffer overrun, if so, turn on warning LED (pinA0)
  if (Serial.available() > 63) PORTC |= Pin_buf;
  while (Serial.available() >= L) // only do something if data is waiting from RS-485 network
  {
    if (Serial.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(buffer, 0x00, L); // initialise buffer
      Serial.readBytes(Buffer, L); // read msg into buffer

      // only interested in messages destined for this address
      if (Buffer[2] == thisAddress)
      {
        // only worth proceeding if checksum test passes, otherwise ignore instruction and return bad txChk byte
        if (Buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          // For AD7714 only care if Analogue Read is requested
          switch (Buffer[3])
          {
            case 'A':
              // Analogue
              // check whether read or write
              switch (Buffer[4])
              {
                case 'R':
                  // Read ###############
                  ReadFromAD7714();
                  Temperature = VoltstoTemp(Volts);
                  TemperatureBytes = reinterpret_cast<char*>(&Temperature);
                  Buffer[6] = TemperatureBytes[3];
                  Buffer[7] = TemperatureBytes[2];
                  Buffer[8] = TemperatureBytes[1];
                  Buffer[9] = TemperatureBytes[0];
                  // #####################
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

          PORTD &= ~PinTx_EN;  // Disable transmit mode on RS-485 transceiver (Tx_EN = 0)
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
  const int Chan = static_cast<int>(static_cast<byte>(Buffer[5]));
  const byte val = static_cast<byte>(ValBytesToFloat());

  switch (Chan)
  {
    case 255:
      // set address
      thisAddress = static_cast<char>(val);
      EEPROM.write(0, val);
      break;
    case 1:
      // change self-calibration interval (val in hrs)
      AutoCalib_ms = 3600000 * max(1UL, static_cast<unsigned long>(val));
      EEPROM.write(1, val);
      break;
    case 0:
      // perform self-calibration
      ConfigureAD7714(0); // Chip 0
      ConfigureAD7714(1); // Chip 1
      break;
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


void ReadFromAD7714()
{
  switch (static_cast<int>(static_cast<byte>(Buffer[5])))
  {
    case 1:
      Volts = ADCvolts[0][0];
      break;
    case 2:
      Volts = ADCvolts[0][1];
      break;
    case 3:
      Volts = ADCvolts[0][2];
      break;
    case 4:
      Volts = ADCvolts[0][3];
      break;
    case 5:
      Volts = ADCvolts[0][4];
      break;
    case 6:
      Volts = ADCvolts[1][0];
      break;
    case 7:
      Volts = ADCvolts[1][1];
      break;
    case 8:
      Volts = ADCvolts[1][2];
      break;
    case 9:
      Volts = ADCvolts[1][3];
      break;
    case 10:
      Volts = ADCvolts[1][4];
      break;
    default:
      Volts = 0.0;
  }
}
