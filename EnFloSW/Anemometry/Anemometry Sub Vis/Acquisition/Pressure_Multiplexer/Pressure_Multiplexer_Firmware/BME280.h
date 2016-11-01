#ifndef BME280_h // double-inclusion guard
#define BME280_h

#include <SPI.h>
#include <Arduino.h>

#define spiBME280 SPISettings(5.0E6, MSBFIRST, SPI_MODE0) // 10.0E6 max according to timing spec

// Custom data types for compensation parameters for BME280 defined such that compensation code in datasheet can be using verbatim
using BME280_S32_t = long signed int; // N.B. int is 32-bit on ARM (like long)
using BME280_U32_t = long unsigned int;
using BME280_S64_t = long long signed int;


class clsBME280
{
  private:
    const byte NOP = 0x00;
    const byte CTRL_HUM = 0b00000101; // 00000, 101 16x oversampling humidity
    const byte CTRL_MEAS_ON = 0b10110111; // 101 16x oversampling temperature, 101 16x oversampling pressure, 11 normal mode
    const byte CTRL_MEAS_OFF = 0b10110100; // 101 16x oversampling temperature, 101 16x oversampling pressure, 00 sleep mode
    const byte CONFIG = 0b00001100; // 000 0.5ms inactive duration between samples, 011 IIR coeff 8, 0, 0 disable 3-wire SPI

    // Compensation parameters for BME280 with custom typedefs defined such that compensation code in datasheet can be using verbatim
    BME280_S32_t t_fine;
    unsigned short dig_T1, dig_P1;
    signed short dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
    unsigned char dig_H1, dig_H3;
    signed char dig_H6;

    byte Pin_CS;

    static const byte L_Buffer = 8;

    byte Buffer[L_Buffer]; // 3 press, 3 temp, 2 hum. = 8

    void SetupPins(const byte pin);

    void Configure();

    void LoadCompensationParams();

    inline void GetRawSample();

    BME280_S32_t Compensate_T_int32(BME280_S32_t adc_T);

    BME280_U32_t Compensate_P_int64(BME280_S32_t adc_P);

    BME280_U32_t Compensate_H_int32(BME280_S32_t adc_H);


  public:
    float Temperature = 25.0; // deg C
    float Pressure = 101325; // Pa
    float Humidity = 30.0; // RH (%)

    void Get_TPH();

    void Initialise(const byte pin);

    void Power(const byte pwr);

    void Reset();

    byte Check_ChipID();

//    byte Read_Register_Byte(const byte Addr, byte MaskBits = 0xFF, byte R_ShiftBits = 0);
//
//    void Write_Register_Byte(const byte Addr, const byte RegByte);
};

#endif // BME280_h
