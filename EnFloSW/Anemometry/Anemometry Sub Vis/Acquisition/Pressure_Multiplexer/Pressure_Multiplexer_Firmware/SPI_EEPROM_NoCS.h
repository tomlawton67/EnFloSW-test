#ifndef SPI_EEPROM_NoCS_h
#define SPI_EEPROM_NoCS_h

#include <SPI.h>
#include <Arduino.h>

#define spiEEPROM SPISettings(5.0E6, MSBFIRST, SPI_MODE0) // 5.0E6 max according to timing spec

// Use this modified SPI_EEPROM class when using the 25xx010A in combination with a CS line that originates from a de-mux or shift register
// !!IMPORTANT!! When calling ANY write function, it is necessary to perform the following steps:
// 0. Set CS low
// 1. Call "Write_Enable()" 
// 2. Cycle CS
// 3. Call the desired WRITE_xxxx function
// 4. Cycle CS
// 5. Execute the "CheckWriteStatus()" loop (with cycle CS inside the loop)
// 6. Cycle CS
// 7. Call "Write_Disable()"
// 8. Set CS high

// Note that "WRITE_all()" and "ERASE_all()" are no longer available and must be implemented manually using the original function but with CS cycle in the loop

class clsSPI_EEPROM_NoCS
{
  private:
    const byte NOP = 0x00;
//    byte Pin_CS;

  public:
    byte PageSize;
    byte nPages;
    word TotalSize;
    
    void Initialise(const byte PageSize_Bytes, const byte nPages);

    byte READ_StatusReg();

    byte READ_byte(const byte addr);

    word READ_word(const byte addr);

    unsigned long READ_uLong(const byte addr);

    float READ_float(const byte addr);

    void READ_page(const byte addr, byte* PageOut);

    void READ_all(byte* AllOut);

    void WRITE_StatusReg(const byte bVal);

    void WRITE_byte(const byte addr, const byte bVal);

    void WRITE_word(const byte addr, const word wVal);

    void WRITE_uLong(const byte addr, const unsigned long ulVal);

    void WRITE_float(const byte addr, const float fVal);

    void WRITE_page(const byte addr, const byte* bVals);

//    void WRITE_all(const byte* bVals);

//    void ERASE_all(const byte bEraseVal);

    boolean CheckWriteStatus();

    void Write_Enable();

    void Write_Disable();

    void Delay_100ns();

    void Delay_200ns();
};


#endif // SPI_EEPROM_NoCS_h
