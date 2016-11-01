#include "BME280.h"


void clsBME280::Initialise(const byte pin)
{
  SetupPins(pin);
  SPI.begin();
  Reset();
}


void clsBME280::SetupPins(const byte pin)
{
  Pin_CS = pin;

  pinMode(Pin_CS, OUTPUT);

  digitalWriteFast(Pin_CS, HIGH);
}


void clsBME280::Get_TPH()
{
  GetRawSample();

  BME280_S32_t T_raw, P_raw, H_raw;

  T_raw = ((BME280_S32_t)Buffer[3]) << 12;
  T_raw |= ((BME280_S32_t)Buffer[4]) << 4;
  T_raw |= ((BME280_S32_t)Buffer[5]) >> 4;

  P_raw = ((BME280_S32_t)Buffer[0]) << 12;
  P_raw |= ((BME280_S32_t)Buffer[1]) << 4;
  P_raw |= ((BME280_S32_t)Buffer[2]) >> 4;

  H_raw = ((BME280_S32_t)Buffer[6]) << 8;
  H_raw |= ((BME280_S32_t)Buffer[7]);

  Temperature = ((float)(Compensate_T_int32(T_raw))) * 0.01;       // deg C
  Pressure = ((float)(Compensate_P_int64(P_raw))) * 0.00390625;    // Pa
  Humidity = ((float)(Compensate_H_int32(H_raw))) * 0.0009765625;  // % Relative Humidity
}


inline void clsBME280::GetRawSample()
{
  SPI.beginTransaction(spiBME280);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0xF7); // start read from PRESS_MSB register 0xF7 (will auto-increment from here)
  Buffer[0] = SPI.transfer(NOP); // PRESS_MSB
  Buffer[1] = SPI.transfer(NOP); // PRESS_LSB
  Buffer[2] = SPI.transfer(NOP); // PRESS_XLSB
  Buffer[3] = SPI.transfer(NOP); // TEMP_MSB
  Buffer[4] = SPI.transfer(NOP); // TEMP_LSB
  Buffer[5] = SPI.transfer(NOP); // TEMP_XLSB
  Buffer[6] = SPI.transfer(NOP); // HUM_MSB
  Buffer[7] = SPI.transfer(NOP); // HUM_LSB
  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


void clsBME280::Reset()
{
  SPI.beginTransaction(spiBME280);

  digitalWriteFast(Pin_CS, LOW);
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz
  digitalWriteFast(Pin_CS, HIGH);
  // rising edge of CS ensures chip is in SPI mode
  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 144MHz

  // issure soft reset cmd
  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x60); // Write to RESET register 0xE0 (0xE0 & 0x7F)
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
  Configure();
  LoadCompensationParams();
  Power(1);
}


void clsBME280::Configure()
{
  SPI.beginTransaction(spiBME280);
  digitalWriteFast(Pin_CS, LOW);

  // Supports multiple writes ([reg addr + data] pairs)

  SPI.transfer(0x72); // Write to 0xF2 register (0xF2 & 0x7F)
  SPI.transfer(CTRL_HUM);

  SPI.transfer(0x74); // Write to 0xF4 register (0xF4 & 0x7F)
  SPI.transfer(CTRL_MEAS_OFF);

  SPI.transfer(0x75); // Write to 0xF5 register (0xF5 & 0x7F)
  SPI.transfer(CONFIG);

  digitalWriteFast(Pin_CS, HIGH);
  SPI.endTransaction();

  delay(100); //100ms delay for settling
}


void clsBME280::Power(const byte pwr)
{
  SPI.beginTransaction(spiBME280);
  digitalWriteFast(Pin_CS, LOW);

  SPI.transfer(0x74); // Write to 0xF4 register (0xF4 & 0x7F)
  SPI.transfer((pwr > 0) ? CTRL_MEAS_ON : CTRL_MEAS_OFF);

  digitalWriteFast(Pin_CS, HIGH);
  SPI.endTransaction();

  delay(100); //100ms delay for settling
}


void clsBME280::LoadCompensationParams()
{
  byte tmp_0xE5;

  SPI.beginTransaction(spiBME280);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0x88); // Read calibration data register 1, auto-increment
  // 0x88
  dig_T1 = (unsigned short)SPI.transfer(NOP);
  dig_T1 |= ((unsigned short)SPI.transfer(NOP)) << 8;
  // 0x8A
  dig_T2 = (signed short)SPI.transfer(NOP);
  dig_T2 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x8C
  dig_T3 = (signed short)SPI.transfer(NOP);
  dig_T3 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x8E
  dig_P1 = (unsigned short)SPI.transfer(NOP);
  dig_P1 |= ((unsigned short)SPI.transfer(NOP)) << 8;
  // 0x90
  dig_P2 = (signed short)SPI.transfer(NOP);
  dig_P2 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x92
  dig_P3 = (signed short)SPI.transfer(NOP);
  dig_P3 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x94
  dig_P4 = (signed short)SPI.transfer(NOP);
  dig_P4 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x96
  dig_P5 = (signed short)SPI.transfer(NOP);
  dig_P5 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x98
  dig_P6 = (signed short)SPI.transfer(NOP);
  dig_P6 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x9A
  dig_P7 = (signed short)SPI.transfer(NOP);
  dig_P7 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x9C
  dig_P8 = (signed short)SPI.transfer(NOP);
  dig_P8 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0x9E
  dig_P9 = (signed short)SPI.transfer(NOP);
  dig_P9 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0xA1
  dig_H1 = (unsigned char)SPI.transfer(NOP);
  digitalWriteFast(Pin_CS, HIGH);

  //__asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 41.6ns delay at 120MHz

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0xE1); // Read calibration data register 2, auto-increment
  // 0xE1
  dig_H2 = (signed short)SPI.transfer(NOP);
  dig_H2 |= ((signed short)SPI.transfer(NOP)) << 8;
  // 0xE3
  dig_H3 = (unsigned char)SPI.transfer(NOP);
  // 0xE4
  dig_H4 = ((signed short)SPI.transfer(NOP)) << 4;
  // 0xE5
  tmp_0xE5 = SPI.transfer(NOP);
  dig_H4 |= (signed short)(tmp_0xE5 & 0b00001111);
  dig_H5 = ((signed short)(tmp_0xE5 & 0b11110000)) >> 4;
  // 0xE6
  dig_H5 |= ((signed short)SPI.transfer(NOP)) << 4;
  // 0xE7
  dig_H6 = (signed char)SPI.transfer(NOP);
  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


byte clsBME280::Check_ChipID()
{
  byte ID;

  SPI.beginTransaction(spiBME280);

  digitalWriteFast(Pin_CS, LOW);
  SPI.transfer(0xD0); // Read ID register 0xD0
  ID = SPI.transfer(NOP); // Should return 0x60 (96)
  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return (ID == 0x60 ? 1 : 0);
}


// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t clsBME280::Compensate_T_int32(BME280_S32_t adc_T)
{
  BME280_S32_t var1, var2, T;

  var1 = ((((adc_T >> 3) - ((BME280_S32_t)dig_T1 << 1))) * ((BME280_S32_t)dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((BME280_S32_t)dig_T1)) * ((adc_T >> 4) - ((BME280_S32_t)dig_T1))) >> 12) *
          ((BME280_S32_t)dig_T3)) >> 14;
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;

  return T;
}


// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of "24674867" represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t clsBME280::Compensate_P_int64(BME280_S32_t adc_P)
{
  BME280_S64_t var1, var2, p;

  var1 = ((BME280_S64_t)t_fine) - 128000;
  var2 = var1 * var1 * (BME280_S64_t)dig_P6;
  var2 = var2 + ((var1 * (BME280_S64_t)dig_P5) << 17);
  var2 = var2 + (((BME280_S64_t)dig_P4) << 35);
  var1 = ((var1 * var1 * (BME280_S64_t)dig_P3) >> 8) + ((var1 * (BME280_S64_t)dig_P2) << 12);
  var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)dig_P1) >> 33;
  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((BME280_S64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7) << 4);

  return (BME280_U32_t)p;
}


// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of "47445" represents 47445/1024 = 46.333 %RH
BME280_U32_t clsBME280::Compensate_H_int32(BME280_S32_t adc_H)
{
  BME280_S32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)dig_H4) << 20) - (((BME280_S32_t)dig_H5) * v_x1_u32r)) +
                 ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)dig_H6)) >> 10) * (((v_x1_u32r *
                     ((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) *
                     ((BME280_S32_t)dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

  return (BME280_U32_t)(v_x1_u32r >> 12);
}


//byte clsBME280::Read_Register_Byte(const byte Addr, byte MaskBits, byte R_ShiftBits)
//{
//  byte RegVal;
//
//  SPI.beginTransaction(spiBME280);
//
//  digitalWriteFast(Pin_CS, LOW);
//  SPI.transfer(Addr); // Read this register (this is the address as given in datasheet (with bit 7 = 1)
//  RegVal = (SPI.transfer(NOP) & MaskBits) >> R_ShiftBits; // mask and shift desired bit(s)
//  digitalWriteFast(Pin_CS, HIGH);
//
//  SPI.endTransaction();
//
//  return RegVal;
//}
//
//
//void clsBME280::Write_Register_Byte(const byte Addr, const byte RegByte)
//{
//
//  SPI.beginTransaction(spiBME280);
//
//  digitalWriteFast(Pin_CS, LOW);
//  SPI.transfer(Addr & 0x7F); // Write to this register (this is the address as given in datasheet (with bit 7 = 1)
//  SPI.transfer(RegByte); // write this byte to the register
//  digitalWriteFast(Pin_CS, HIGH);
//
//  SPI.endTransaction();
//}
