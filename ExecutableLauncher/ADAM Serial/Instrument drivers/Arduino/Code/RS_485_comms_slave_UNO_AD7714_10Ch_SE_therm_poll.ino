// Developed by Paul Nathan, last mod. 05/07/2013

#include <SPI.h>

// Thermistor variables
const float invB = 1.0 / 4334.0;
const float R2 = 100000.0;
const float R10 = 100000.0;
const float invT0 = 1.0 / 298.15;
const float Vs = 2.509; // hard-wired to Vref
float R1;


// AD7714 variables
const int pinCS[2] = {
  10, 9};

byte chan[2];
unsigned long DataReg, lastAutoCalib_ms, now_ms;//, GainReg, OffsetReg;
byte ModeReg, GainBits, FilterHighBits, FilterHighReg, FilterLowReg;
byte DRDYbit;
float Value[2][5], Val;
char *ValBytes;

const int nChans = 5; // Hard-coded (SE = 5, DIFF = 3)
const byte ChannelBitsLUT_DIFF[3] = {
  0b00000100, 0b00000101, 0b00000110};
const byte ChannelBitsLUT_SE[5] = {
  0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000110};

byte ChannelBits[5]; // for easy, globally selected SE or DIFF bits. Copy either of the above into this array

const unsigned long AutoCalib_ms = 3600000; // reset, config, auto-calib every hour
const float Vref = 2.499 - 0.0; // Manually input Vref (specific to each board!)


// RS-485 variables
int pinTX_ENABLE, L, Lm1, Lm2;
unsigned int txDelay;
char buffer[12], tmp;

const char thisAddress = 1;
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
  digitalWrite(pinCS[0], LOW);
  SPI.transfer(0b00001000 | ChannelBits[chan[0]]);
  DRDYbit = SPI.transfer(0xFF) & 0b10000000;
  if (DRDYbit == 0) DRDY_fall(0);
  digitalWrite(pinCS[0], HIGH);

  // CHIP 1
  digitalWrite(pinCS[1], LOW);
  SPI.transfer(0b00001000 | ChannelBits[chan[1]]);
  DRDYbit = SPI.transfer(0xFF) & 0b10000000;
  if (DRDYbit == 0) DRDY_fall(1);
  digitalWrite(pinCS[1], HIGH);

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

// rx/tx msg syntax is 12 bytes: '@', src., dest., A/D, R/W, pin, val, val ,val, val, rxCHK, txCHK

void SerialSetup()
{
  pinTX_ENABLE = 4;
  L = 12; // msg length

  pinMode(pinTX_ENABLE, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A3, OUTPUT);

  digitalWrite(A5, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A3, LOW);
  digitalWrite (pinTX_ENABLE, LOW);  // Disable transmit mode

  Lm1 = L - 1;
  Lm2 = L - 2;

  txDelay = (8 * 1000000) / baud; //us

  Serial.begin(baud); // send/receive messages to/from RS-485 network
}

void SerialLoop()
{
  // Check if message waiting in RS-485 transducer buffer
  // if so, construct variable buffer, implement request, and send output back over RS-485 to master
  // first check for buffer overrun, if so, turn on warning LED (pinA0)
  if (Serial.available() > 63) digitalWrite(A5, HIGH); 
  while (Serial.available() >= L) // only do something if data is waiting from RS-485 network
  {
    if (Serial.peek() == '@') // only consider valid message start, otherwise discard
    {
      //memset(buffer, 0x00, L); // initialise buffer  
      Serial.readBytes(buffer, L); // read msg into buffer

      // only interested in messages destined for this address
      if (buffer[2] == thisAddress)
      {
        // only worth proceeding if checksum test passes, otherwise ignore instruction and return bad txChk byte
        if (buffer[Lm2] == static_cast<char>(CalcChecksum()))
        {
          // For AD7714 only care if Analogue Read is requested
          switch (buffer[3])
          {
          case 'A':
            // Analogue
            // check whether read or write
            {
              switch (buffer[4])
              {
              case 'R':
                // Read
                ReadFromAD7714();
                VoltstoTemp();
                ValBytes = reinterpret_cast<char*>(&Val);
                buffer[6] = ValBytes[3];
                buffer[7] = ValBytes[2];
                buffer[8] = ValBytes[1];
                buffer[9] = ValBytes[0];
                break;
              }
            }
            break;
          }
          // now switch source and destination
          tmp = buffer[1];
          buffer[1] = buffer[2];
          buffer[2] = tmp;

          // to get here, txChk was okay, send back good txChk byte
          buffer[Lm1] = 0;
          // calculate checksum and place on end of buffer
          buffer[Lm2] = static_cast<char>(CalcChecksum());

          // Finally write updated buffer (now with requested value) to RS-485 network to be read by master
          digitalWrite (pinTX_ENABLE, HIGH);  // Enable transmit mode on RS-485 transceiver

          Serial.write(reinterpret_cast<byte*>(&buffer), L); // write buffer to RS-485 DI
          Serial.flush(); // wait until write complete
          delayMicroseconds(txDelay); // don't miss out last byte!

          digitalWrite (pinTX_ENABLE, LOW);  // Disable transmit mode on RS-485 transceiver
          digitalWrite(A5, LOW); // Successful msg cycle, reset any error lights
          digitalWrite(A4, LOW);
          digitalWrite(A3, LOW);
        }
        else
        {
          // warn of bad checksum
          // switch source and destination
          tmp = buffer[1];
          buffer[1] = buffer[2];
          buffer[2] = tmp;

          // to get here, txChk was bad, send back baf txChk byte
          buffer[Lm1] = 1;
          // calculate checksum and place on CHKtmp byte
          buffer[Lm2] = static_cast<char>(CalcChecksum());

          // Finally write updated buffer (now with requested value) to RS-485 network to be read by master
          digitalWrite (pinTX_ENABLE, HIGH);  // Enable transmit mode on RS-485 transceiver

          Serial.write(reinterpret_cast<byte*>(&buffer), L); // write buffer to RS-485 DI
          Serial.flush(); // wait until write complete
          delayMicroseconds(txDelay); // don't miss out last byte!

          digitalWrite (pinTX_ENABLE, LOW);  // Disable transmit mode on RS-485 transceiver
          digitalWrite(A4, HIGH);
        }
      }
    }
    else
    {
      Serial.read(); // discard invalid byte
      digitalWrite(A3, HIGH); // warn of invalid byte(s)
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


void  AD7714Setup()
{
  // configure pins
  pinMode(pinCS[0], OUTPUT);
  pinMode(pinCS[1], OUTPUT);

  digitalWrite(pinCS[0], HIGH);
  digitalWrite(pinCS[1], HIGH);

  // configure SPI mode
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4) ;

  // begin SPI comms.
  SPI.begin();

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

  chan[0] = 0;
  chan[1] = 0;

  lastAutoCalib_ms = millis();
}


void VoltstoTemp()
{
  // At this point, Val is the thermistor voltage. Use it then overwrite with temp.  
  R1 = ((Vs / Val) - 1.0) * R2;
  Val = (1.0 / ((invB * log(R1 / R10)) + invT0)) - 273.15; // degC
}


void ReadFromAD7714()
{
  switch (buffer[5])
  {
  case 0:
    // Reset ADC. Stop sampling, drive to default with >=32 DIN highs, configure and start sampling again
    // Chip 0
    ConfigureAD7714(0);
    // Chip 1
    ConfigureAD7714(1);
    break;
  case 1:
    Val = Value[0][0];
    break;
  case 2:
    Val = Value[0][1];
    break;
  case 3:
    Val = Value[0][2];
    break;
  case 4:
    Val = Value[0][3];
    break;
  case 5:
    Val = Value[0][4];
    break;
  case 6:
    Val = Value[1][0];
    break;
  case 7:
    Val = Value[1][1];
    break;
  case 8:
    Val = Value[1][2];
    break;
  case 9:
    Val = Value[1][3];
    break;
  case 10:
    Val = Value[1][4];
    break;
  default: 
    Val = 0.0;
  }

}


void DRDY_fall(int ChipNum)
{     
  SPI.transfer(0b01011000 | ChannelBits[chan[ChipNum]]); // set to read data register next
  DataReg = static_cast<unsigned long>(SPI.transfer(0xFF)) << 16; // read high byte and shift +16
  DataReg |= static_cast<unsigned long>(SPI.transfer(0xFF)) << 8; // read mid byte and shift +8 and biwise OR
  DataReg |= static_cast<unsigned long>(SPI.transfer(0xFF)); // read low byte and bitwise OR

  Value[ChipNum][chan[ChipNum]] = Vref * (float(DataReg) / 16777215.0); // raw voltage // Unipolar 24-bit mode only

  chan[ChipNum] += 1;
  chan[ChipNum] %= nChans;  // HARD CODED FOR SINGLE-ENDED ######

  // prepare for next channel
  SPI.transfer(0b00001000 | ChannelBits[chan[ChipNum]]);
  SPI.transfer(0xFF);
}


void ConfigureAD7714(int ChipNum)
{
  // Reset ADC
  digitalWrite(pinCS[ChipNum], LOW);

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

  digitalWrite(pinCS[ChipNum], HIGH);
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

  // Poll DRDY bit in comms register
  do
  {
    SPI.transfer(0b00001000 | ChanBits);
    DRDYbit = (SPI.transfer(0xFF) & 0b10000000); 
  } 
  while (DRDYbit == 0b10000000);
  // to get here DRDY must have fallen, discard value and move on to next calib channel 
  SPI.transfer(0b01011000 | ChanBits); // set to read data register next
  SPI.transfer(0xFF); // read high byte and shift +16
  SPI.transfer(0xFF); // read mid byte and shift +8 and biwise OR
  SPI.transfer(0xFF); // read low byte and bitwise OR
}





























