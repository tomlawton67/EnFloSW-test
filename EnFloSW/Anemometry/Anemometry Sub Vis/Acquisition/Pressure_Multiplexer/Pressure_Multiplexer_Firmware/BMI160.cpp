#include "BMI160.h"


void clsBMI160::Initialise(const byte pin)
{
  SetupPins(pin);
  SPI.begin();
  Reset();
}


void clsBMI160::SetupPins(const byte pin)
{
  Pin_CS = pin;

  pinMode(Pin_CS, OUTPUT);

  digitalWriteFast(Pin_CS, HIGH);
}


void clsBMI160::GetSample()
{
  // check drdy bits and update data register
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x9B); // read from STATUS register (0x1B | 0x80)
  byte RegByte = SPI.transfer(NOP);
  digitalWriteFast(Pin_CS, HIGH);

  // GYRO ########################################################
  if ((RegByte & 0b01000000) > 0) // check drdy_gyr bit
  {
    // collect gyro data.
    GetSample_Gyro();
    Buffer[12] = 0b00000000;
  }
  else
  {
    // set gyro stale bit in buffer
    Buffer[12] = 0b00000001;
  }

  // ACCEL ########################################################
  if ((RegByte & 0b10000000) > 0) // check drdy_acc bit
  {
    // collect accel data.
    GetSample_Accel();
    Buffer[12] |= 0b00000000;
  }
  else
  {
    // set accel stale bit in buffer
    Buffer[12] |= 0b00000010;
  }
  SPI.endTransaction();
}


inline void clsBMI160::GetSample_Gyro()
{
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x8C); // read from DATA register (0x0C | 0x80)
  Buffer[1] = SPI.transfer(NOP); // gyr_x_LSB
  Buffer[0] = SPI.transfer(NOP); // gyr_x_MSB
  Buffer[3] = SPI.transfer(NOP); // gyr_y_LSB
  Buffer[2] = SPI.transfer(NOP); // gyr_y_MSB
  Buffer[5] = SPI.transfer(NOP); // gyr_z_LSB
  Buffer[4] = SPI.transfer(NOP); // gyr_z_MSB
  digitalWriteFast(Pin_CS, HIGH);
}


inline void clsBMI160::GetSample_Accel()
{
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x92); // read from DATA register (0x12 | 0x80)
  Buffer[7] = SPI.transfer(NOP); // acc_x_LSB
  Buffer[6] = SPI.transfer(NOP); // acc_x_MSB
  Buffer[9] = SPI.transfer(NOP); // acc_y_LSB
  Buffer[8] = SPI.transfer(NOP); // acc_y_MSB
  Buffer[11] = SPI.transfer(NOP); // acc_z_LSB
  Buffer[10] = SPI.transfer(NOP); // acc_z_MSB
  digitalWriteFast(Pin_CS, HIGH);
}


void clsBMI160::Get_Temperature()
{
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0xA0); // Read TEMPERATURE_0 register (0x20 | 0x80)
  Buffer[14] = static_cast<word>(SPI.transfer(NOP)); // LSB
  Buffer[13] = static_cast<word>(SPI.transfer(NOP)); // MSB
  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


void clsBMI160::Configure()
{
  Configure_Acc();
  Configure_Gyr();
}


void clsBMI160::Configure_Acc()
{
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x40); // Write to ACC_CONF register (0x40 | 0x00)
  SPI.transfer(ACC_CONF);
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x41); // Write to ACC_RANGE register (0x41 | 0x00)
  SPI.transfer(ACC_RANGE);
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  delay(100); //100ms delay for settling
}


void clsBMI160::Configure_Gyr()
{
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x42); // Write to GYR_CONF register (0x42 | 0x00)
  SPI.transfer(GYR_CONF);
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x43); // Write to GYR_RANGE register (0x43 | 0x00)
  SPI.transfer(GYR_RANGE);
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  delay(100); //100ms delay for settling
}


void clsBMI160::Power(const byte pwr)
{
  SPI.beginTransaction(spiBMI160);

  // Accel
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x7E); // Write to CMD register (0x7E | 0x00)
  SPI.transfer((pwr > 0) ? 0b00010001 : 0b00010000);
  digitalWriteFast(Pin_CS, HIGH);

  delay(100); // longer delay in case interface is in suspend mode

  // Gyro
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x7E); // Write to CMD register (0x7E | 0x00)
  SPI.transfer((pwr > 0) ? 0b00010101 : 0b00010100);
  digitalWriteFast(Pin_CS, HIGH);

  delay(100); // longer delay in case interface is in suspend mode

  SPI.endTransaction();
}


void clsBMI160::Reset()
{
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz
  digitalWriteFast(Pin_CS, HIGH);
  // rising edge of CS ensures chip is in SPI mode
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz

  // issue soft reset cmd
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x7E); // Write to CMD register 0x7E (0x7E | 0x00)
  SPI.transfer(0xB6); // soft reset
  digitalWriteFast(Pin_CS, HIGH);

  delay(100);

  digitalWriteFast(Pin_CS, LOW);
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz
  digitalWriteFast(Pin_CS, HIGH);
  // rising edge of CS ensures chip is in SPI mode
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz

  SPI.endTransaction();

  delay(100);

  Set_Defaults();
  Power(1);
  Configure();
}


void clsBMI160::Set_Defaults()
{
  ACC_CONF = ACC_CONF_default;
  ACC_RANGE = ACC_RANGE_default;
  GYR_CONF = GYR_CONF_default;
  GYR_RANGE = GYR_RANGE_default;
}


byte clsBMI160::Check_ChipID()
{
  byte ID;

  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x80); // Read Chip_ID register
  ID = SPI.transfer(NOP); // Should return 0xD1 (209)
  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return (ID == 0xD1 ? 1 : 0);
}


void clsBMI160::Set_Range_and_Conf_Regs(const byte acc_range_Mode, const byte acc_conf_Mode, const byte gyr_range_Mode, const byte gyr_conf_Mode)
{
  Set_ACC_RANGE(acc_range_Mode);
  Set_ACC_CONF(acc_conf_Mode);
  Set_GYR_RANGE(gyr_range_Mode);
  Set_GYR_CONF(gyr_conf_Mode);
}


void clsBMI160::Set_ACC_RANGE(const byte Mode)
{
  switch (Mode)
  {
    case 0: // +-2g
      ACC_RANGE = 0b00000011;
      break;
    case 1: // +-4g
      ACC_RANGE = 0b00000101;
      break;
    case 2: // +-8g
      ACC_RANGE = 0b00001000;
      break;
    case 3: // +-16g
      ACC_RANGE = 0b00001100;
      break;
    default: // 0000, 0011 +-2g range
      ACC_RANGE = 0b00000011;
  }
  Configure_Acc();
}


void clsBMI160::Set_ACC_CONF(const byte Mode)
{
  switch (Mode)
  {
    case 0: // 3.125 Hz, OSR 4x
      ACC_CONF = 0b00000101;
      break;
    case 1: // 6.25 Hz, OSR 4x
      ACC_CONF = 0b00000110;
      break;
    case 2: // 12.5 Hz, OSR 4x
      ACC_CONF = 0b00000111;
      break;
    case 3: // 25 Hz, OSR 4x
      ACC_CONF = 0b00001000;
      break;
    case 4: // 50 Hz, OSR 4x
      ACC_CONF = 0b00001001;
      break;
    case 5: // 100 Hz, OSR 4x
      ACC_CONF = 0b00001010;
      break;
    case 6: // 200 Hz, OSR 4x
      ACC_CONF = 0b00001011;
      break;
    case 7: // 400 Hz, OSR 4x
      ACC_CONF = 0b00001100;
      break;
    case 8: // 800 Hz, OSR 2x
      ACC_CONF = 0b00011100;
      break;
    case 9: // 1600 Hz, OSR 1x
      ACC_CONF = 0b00101100;
      break;
    default: // 0 (no undersampling), 000 4x oversampling, 1010 ODR = 100Hz
      ACC_CONF = 0b00001010;
  }
  Configure_Acc();
}

void clsBMI160::Set_GYR_RANGE(const byte Mode)
{
  switch (Mode)
  {
    case 0: // +-125dps
      GYR_RANGE = 0b00000100;
      break;
    case 1: // +-250dps
      GYR_RANGE = 0b00000011;
      break;
    case 2: // +-500dps
      GYR_RANGE = 0b00000010;
      break;
    case 3: // +-1000dps
      GYR_RANGE = 0b00000001;
      break;
    case 4: // +2000dps
      GYR_RANGE = 0b00000000;
      break;
    default: // 00000, 100 +-125dps range
      GYR_RANGE = 0b00000100;
  }
  Configure_Gyr();
}


void clsBMI160::Set_GYR_CONF(const byte Mode)
{
  switch (Mode)
  {
    case 0: // 6.25 Hz, OSR 4x
      GYR_CONF = 0b00000110;
      break;
    case 1: // 12.5 Hz, OSR 4x
      GYR_CONF = 0b00000111;
      break;
    case 2: // 25 Hz, OSR 4x
      GYR_CONF = 0b00001000;
      break;
    case 3: // 50 Hz, OSR 4x
      GYR_CONF = 0b00001001;
      break;
    case 4: // 100 Hz, OSR 4x
      GYR_CONF = 0b00001010;
      break;
    case 5: // 200 Hz, OSR 4x
      GYR_CONF = 0b00001011;
      break;
    case 6: // 400 Hz, OSR 4x
      GYR_CONF = 0b00001100;
      break;
    case 7: // 800 Hz, OSR 4x
      GYR_CONF = 0b00001101;
      break;
    case 8: // 1600 Hz, OSR 2x
      GYR_CONF = 0b00011101;
      break;
    case 9: // 3200 Hz, OSR 1x
      GYR_CONF = 0b00101101;
      break;
    default: // 0 (no undersampling), 000 4x oversampling, 1010 ODR = 100Hz
      GYR_CONF = 0b00001010;
  }
  Configure_Gyr();
}


byte clsBMI160::Self_Test()
{
  byte Pass = 0; // Range 0...3. 0 = fail (both), 1 = pass accel fail gyro, 2 = fail accel pass gyro, 3 = pass (both)
  float ax1, ay1, az1, ax2, ay2, az2;
  byte RegByte;

  // ACCEL ON, +ve

  // Set configs to self-test values first
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x40); // Write to ACC_CONF register (0x40 | 0x00)
  SPI.transfer(0x2C); // 0 (no undersampling), 010 no oversampling, 1100 ODR = 1600Hz
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x6D); // Write to SELF_TEST register (0x6D | 0x00)
  SPI.transfer(0b00000101); // 000, 0 gyr_test off, 0 low amp, 1 +ve sign, 01 acc test enable
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  delay(100);

  // read values and keep for later
  SPI.beginTransaction(spiBMI160);
  GetSample_Accel();
  SPI.endTransaction();

  ax1 = WordToFloat(word(Buffer[6], Buffer[7]), 8.0);
  ay1 = WordToFloat(word(Buffer[8], Buffer[9]), 8.0);
  az1 = WordToFloat(word(Buffer[10], Buffer[11]), 8.0);


  // ACCEL ON, -ve

  // Set configs to self-test values first
  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x40); // Write to ACC_CONF register (0x40 | 0x00)
  SPI.transfer(0x2C); // 0 (no undersampling), 010 no oversampling, 1100 ODR = 1600Hz
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x6D); // Write to SELF_TEST register (0x6D | 0x00)
  SPI.transfer(0b00000001); // 000, 0 gyr_test off, 0 low amp, 0 -ve sign, 01 acc test enable
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  delay(100);

  // read values and check
  SPI.beginTransaction(spiBMI160);
  GetSample_Accel();
  SPI.endTransaction();

  ax2 = WordToFloat(word(Buffer[6], Buffer[7]), 8.0);
  ay2 = WordToFloat(word(Buffer[8], Buffer[9]), 8.0);
  az2 = WordToFloat(word(Buffer[10], Buffer[11]), 8.0);

  // check within range.  1 pass, 0 fail
  Pass = (abs(ax1 - ax2) > 2.0) && (abs(ay1 - ay2) > 2.0) && (abs(az1 - az2) > 2.0);


  // GYRO ON

  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x6D); // Write to SELF_TEST register (0x6D | 0x00)
  SPI.transfer(0b00010000); // 000, 1 gyr_test on, 0 low amp, 0 -ve sign, 00 acc test disable
  digitalWriteFast(Pin_CS, HIGH);

  delay(100);

  // read result from STATUS register
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x9B); // read from STATUS register (0x1B | 0x80)
  RegByte = SPI.transfer(NOP);
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  Pass |= RegByte & 0b00000010; // value of 2 is pass, 0 fail.

  // ALL OFF

  SPI.beginTransaction(spiBMI160);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x6D); // Write to SELF_TEST register (0x6D | 0x00)
  SPI.transfer(0x00); // 000, 0 gyr_test off, 0 low amp, 0 -ve sign, 00 acc test disable
  digitalWriteFast(Pin_CS, HIGH);

  delayMicroseconds(4);

  SPI.endTransaction();

  delay(100);
  Reset();

  return Pass;
}


float clsBMI160::WordToFloat(const word W, const float Range) // Convert two's complement to float
{
  float x;

  if ((W & 0x8000) == 0)
  {
    // positive, straight convert
    x = static_cast<float>(W) * Range * 3.05175781250E-5;
  }
  else
  {
    // negative, two's complement
    x = -static_cast<float>((W ^ 0xFFFF) + 1U) * Range * 3.05175781250E-5;
  }

  return x;
}


//byte clsBMI160::Read_Register_Byte(const byte Addr, byte MaskBits, byte R_ShiftBits)
//{
//  byte RegVal;
//
//  SPI.beginTransaction(spiBMI160);
//
//  digitalWriteFast(Pin_CS, LOW);
//  SPI.transfer(Addr | 0x80); // Read the register (this is the address as given in datasheet (with bit 7 = 0)
//  RegVal = (SPI.transfer(NOP) & MaskBits) >> R_ShiftBits; // mask and shift desired bit(s)
//  digitalWriteFast(Pin_CS, HIGH);
//
//  SPI.endTransaction();
//
//  return RegVal;
//}
//
//
//void clsBMI160::Write_Register_Byte(const byte Addr, const byte RegByte)
//{
//
//  SPI.beginTransaction(spiBMI160);
//
//  digitalWriteFast(Pin_CS, LOW);
//  SPI.transfer(Addr); // Write to this register (this is the address as given in datasheet (with bit 7 = 0)
//  SPI.transfer(RegByte); // write this byte to the register
//  digitalWriteFast(Pin_CS, HIGH);
//
//  SPI.endTransaction();
//}

