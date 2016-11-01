// ######### SUPPLEMENTARY SPI_EEPROM FUNCTIONS FOR No_CS CASE ###########


byte SPI_EEPROM_READ_StatusReg(const byte ChipNum)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  const byte bVal = SPI_EEPROM.READ_StatusReg();
  ShiftReg_EEPROM.Clear();

  return bVal;
}


byte SPI_EEPROM_READ_byte(const byte ChipNum, const byte addr)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  const byte bVal = SPI_EEPROM.READ_byte(addr);
  ShiftReg_EEPROM.Clear();

  return bVal;
}


word SPI_EEPROM_READ_word(const byte ChipNum, const byte addr)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  const word wVal = SPI_EEPROM.READ_word(addr);
  ShiftReg_EEPROM.Clear();

  return wVal;
}


word SPI_EEPROM_READ_uLong(const byte ChipNum, const byte addr)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  const unsigned long ulVal = SPI_EEPROM.READ_uLong(addr);
  ShiftReg_EEPROM.Clear();

  return ulVal;
}


float SPI_EEPROM_READ_float(const byte ChipNum, const byte addr)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  const float fVal = SPI_EEPROM.READ_float(addr);
  ShiftReg_EEPROM.Clear();

  return fVal;
}


void SPI_EEPROM_READ_page(const byte ChipNum, const byte addr, byte* PageOut)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.READ_page(addr, PageOut);
  ShiftReg_EEPROM.Clear();
}


void SPI_EEPROM_READ_all(const byte ChipNum, byte* AllOut)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.READ_all(AllOut);
  ShiftReg_EEPROM.Clear();
}


void SPI_EEPROM_WRITE_StatusReg(const byte ChipNum, const byte bVal)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_StatusReg(bVal);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_Byte(const byte ChipNum, const byte addr, const byte bVal)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_byte(addr, bVal);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_word(const byte ChipNum, const byte addr, const word wVal)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_word(addr, wVal);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_uLong(const byte ChipNum, const byte addr, const unsigned long ulVal)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_uLong(addr, ulVal);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_float(const byte ChipNum, const byte addr, const float fVal)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_float(addr, fVal);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_page(const byte ChipNum, const byte addr, const byte* bVals)
{
  SPI_EEPROM_NoCS_Write_Enable(ChipNum);

  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.WRITE_page(addr, bVals);
  ShiftReg_EEPROM.Clear();

  SPI_EEPROM_NoCS_CheckWriteStatus(ChipNum);
  SPI_EEPROM_NoCS_Write_Disable(ChipNum);
}


void SPI_EEPROM_WRITE_all(const byte ChipNum, const byte* bVals)
{
  byte addr = 0x00;

  for (byte i = 0; i < SPI_EEPROM.nPages; i++)
  {
    SPI_EEPROM_WRITE_page(ChipNum, addr, &bVals[addr]);
    addr += SPI_EEPROM.PageSize;
  }
}


void SPI_EEPROM_ERASE_all(const byte ChipNum, const byte bEraseVal)
{
  byte addr = 0x00;
  byte bVals[SPI_EEPROM.PageSize];

  memset (&bVals[0], bEraseVal, SPI_EEPROM.PageSize);
  for (byte i = 0; i < SPI_EEPROM.nPages; i++)
  {
    SPI_EEPROM_WRITE_page(ChipNum, addr, bVals);
    addr += SPI_EEPROM.PageSize;
  }
}


void SPI_EEPROM_NoCS_CheckWriteStatus(const byte ChipNum)
{
  do
  {
    ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  } while (SPI_EEPROM.CheckWriteStatus());

  ShiftReg_EEPROM.Clear();
}


void SPI_EEPROM_NoCS_Write_Enable(const byte ChipNum)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.Write_Enable();
  ShiftReg_EEPROM.Clear();
}


void SPI_EEPROM_NoCS_Write_Disable(const byte ChipNum)
{
  ShiftReg_EEPROM.ClearThenSetSoloBit(ChipNum);
  SPI_EEPROM.Write_Disable();
  ShiftReg_EEPROM.Clear();
}


// ######### SUPPLEMENTARY SPI_EEPROM FUNCTIONS FOR No_CS CASE ###########
