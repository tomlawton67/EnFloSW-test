#ifndef Press_NoCS_h
#define Press_NoCS_h

#include <SPI.h>
#include <Arduino.h>

#define spiPress SPISettings(3.0E6, MSBFIRST, SPI_MODE0) // 0.8E6 max according to timing spec(!)


class clsPress_NoCS
{
  private:
    const byte NOP = 0x00;
    //    byte Pin_CS;


  public:
    static const byte L_Buffer_Fast = 2;
    static const byte L_Buffer_Full = 4;
    
    byte Buffer_Fast[L_Buffer_Fast], Buffer_Full[L_Buffer_Full];

    void Initialise();

    void GetSample_Full();

    void GetSample_Fast();

    void Delay_2500ns();

    float CalcTemperature();

    byte ExtractStatusBits_fromFull();

    byte ExtractStatusBits_fromFast();
};

#endif // Press_NoCS_h
