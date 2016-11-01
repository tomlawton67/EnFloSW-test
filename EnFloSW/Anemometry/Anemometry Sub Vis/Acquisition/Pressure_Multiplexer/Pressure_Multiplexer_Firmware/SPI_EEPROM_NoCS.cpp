#include "SPI_EEPROM_NoCS.h"


void clsSPI_EEPROM_NoCS::Initialise(const byte PageSize_Bytes, const byte num_Pages)
{

//  Pin_CS = CS_Pin;
//  pinMode(Pin_CS, OUTPUT);
//  digitalWriteFast(Pin_CS, HIGH);

  PageSize = PageSize_Bytes;
  nPages = num_Pages;
  TotalSize = (word)PageSize * (word)nPages;

  SPI.begin();
}


byte clsSPI_EEPROM_NoCS::READ_StatusReg()
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000101);
  const byte bVal = SPI.transfer(NOP);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return bVal;
}


byte clsSPI_EEPROM_NoCS::READ_byte(const byte addr)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(addr);
  const byte bVal = SPI.transfer(NOP);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return bVal;
}


word clsSPI_EEPROM_NoCS::READ_word(const byte addr)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(addr);
  word wVal = static_cast<word>(SPI.transfer(NOP)) << 8;
  wVal |= static_cast<word>(SPI.transfer(NOP));

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return wVal;
}


unsigned long clsSPI_EEPROM_NoCS::READ_uLong(const byte addr)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(addr);
  unsigned long ulVal = static_cast<unsigned long>(SPI.transfer(NOP)) << 24;
  ulVal |= static_cast<unsigned long>(SPI.transfer(NOP)) << 16;
  ulVal |= static_cast<unsigned long>(SPI.transfer(NOP)) << 8;
  ulVal |= static_cast<unsigned long>(SPI.transfer(NOP));

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  return ulVal;
}


float clsSPI_EEPROM_NoCS::READ_float(const byte addr)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(addr);
  byte fVal_b[4];
  fVal_b[0] = SPI.transfer(NOP);
  fVal_b[1] = SPI.transfer(NOP);
  fVal_b[2] = SPI.transfer(NOP);
  fVal_b[3] = SPI.transfer(NOP);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

  float fVal;
  memcpy(&fVal, &fVal_b[0], 4);

  return fVal;
}


void clsSPI_EEPROM_NoCS::READ_page(const byte addr, byte* PageOut)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(addr);
  for (byte i = 0; i < PageSize; i++)
  {
    PageOut[i] = SPI.transfer(NOP);
  }

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


void clsSPI_EEPROM_NoCS::READ_all(byte* AllOut)
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000011);
  SPI.transfer(0x00);
  for (word i = 0; i < TotalSize; i++)
  {
    AllOut[i] = SPI.transfer(NOP);
  }

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


void clsSPI_EEPROM_NoCS::WRITE_StatusReg(const byte bVal)
{
//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000001);
  SPI.transfer(bVal);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


void clsSPI_EEPROM_NoCS::WRITE_byte(const byte addr, const byte bVal)
{
//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000010);
  SPI.transfer(addr);
  SPI.transfer(bVal);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


void clsSPI_EEPROM_NoCS::WRITE_word(const byte addr, const word wVal)
{
//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000010);
  SPI.transfer(addr);
  SPI.transfer(static_cast<byte>((wVal & 0b1111111100000000) >> 8));
  SPI.transfer(static_cast<byte>(wVal & 0b0000000011111111));

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


void clsSPI_EEPROM_NoCS::WRITE_uLong(const byte addr, const unsigned long ulVal)
{
//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000010);
  SPI.transfer(addr);
  SPI.transfer(static_cast<byte>((ulVal & 0b11111111000000000000000000000000) >> 24));
  SPI.transfer(static_cast<byte>((ulVal & 0b00000000111111110000000000000000) >> 16));
  SPI.transfer(static_cast<byte>((ulVal & 0b00000000000000001111111100000000) >> 8));
  SPI.transfer(static_cast<byte>( ulVal & 0b00000000000000000000000011111111));

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


void clsSPI_EEPROM_NoCS::WRITE_float(const byte addr, const float fVal)
{
  byte bVals[4];
  memcpy(&bVals[0], &fVal, 4);

//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000010);
  SPI.transfer(addr);
  SPI.transfer(bVals[0]);
  SPI.transfer(bVals[1]);
  SPI.transfer(bVals[2]);
  SPI.transfer(bVals[3]);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


void clsSPI_EEPROM_NoCS::WRITE_page(const byte addr, const byte* bVals)
{
//  Write_Enable();

  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000010);
  SPI.transfer(addr);
  for (byte i = 0; i < PageSize; i++)
  {
    SPI.transfer(bVals[i]);
  }

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();

//  while (CheckWriteStatus())
//  {
//    ;
//  }
//
//  Write_Disable();
}


//void clsSPI_EEPROM_NoCS::WRITE_all(const byte* bVals)
//{
//  byte addr = 0x00;
//
//  for (byte i = 0; i < nPages; i++)
//  {
//    WRITE_page(addr, &bVals[addr]);
//    addr += PageSize;
//  }
//}


//void clsSPI_EEPROM_NoCS::ERASE_all(const byte bEraseVal)
//{
//  byte addr = 0x00;
//  byte bVals[PageSize];
//
//  memset (&bVals[0], bEraseVal, PageSize);
//  for (byte i = 0; i < nPages; i++)
//  {
//    WRITE_page(addr, bVals);
//    addr += PageSize;
//  }
//}


boolean clsSPI_EEPROM_NoCS::CheckWriteStatus()
{
  return (READ_StatusReg() & 0b00000001);
}


void clsSPI_EEPROM_NoCS::Write_Enable()
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000110);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


void clsSPI_EEPROM_NoCS::Write_Disable()
{
  SPI.beginTransaction(spiEEPROM);

//  digitalWriteFast(Pin_CS, LOW);
  Delay_100ns();

  SPI.transfer(0b00000100);

  Delay_200ns();
//  digitalWriteFast(Pin_CS, HIGH);

  SPI.endTransaction();
}


inline void clsSPI_EEPROM_NoCS::Delay_100ns()
{
  __asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 97.2ns delay at 144MHz
}


inline void clsSPI_EEPROM_NoCS::Delay_200ns()
{
  __asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 97.2ns delay at 144MHz
  __asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"::); // 97.2ns delay at 144MHz
}


