#ifndef BMI160_h
#define BMI160_h

#include <SPI.h>
#include <Arduino.h>

#define spiBMI160 SPISettings(5.0E6, MSBFIRST, SPI_MODE0) // 10.0E6 max according to timing spec


class clsBMI160
{
  private:
    const byte ACC_CONF_default = 0b00001010; // 0 (no undersampling), 000 4x oversampling, 1010 ODR = 100Hz
    const byte ACC_RANGE_default = 0b00000011; // 0000, 0011 +-2g range
    const byte GYR_CONF_default = 0b00001010; // 00, 00 4x oversampling, 1010 ODR = 100Hz
    const byte GYR_RANGE_default = 0b00000100; // 00000, 100 +-125dps range
    const byte NOP = 0x00;

    byte ACC_CONF = ACC_CONF_default;
    byte ACC_RANGE = ACC_RANGE_default;
    byte GYR_CONF = GYR_CONF_default;
    byte GYR_RANGE = GYR_RANGE_default;

    byte Pin_CS;

    void SetupPins(const byte pin);

    void Set_Defaults();

    void Configure();

    void Configure_Acc();

    void Configure_Gyr();

    inline void GetSample_Gyro();

    inline void GetSample_Accel();

    void Set_ACC_RANGE(const byte Mode);

    void Set_ACC_CONF(const byte Mode);

    void Set_GYR_RANGE(const byte Mode);

    void Set_GYR_CONF(const byte Mode);

    float WordToFloat(const word W, const float Range);


  public:
    static const byte L_Buffer = 15;

    byte Buffer[L_Buffer]; // 2 wx, 2 wy, 2 wz, 2 ax, 2 ay, 2 az, 1 stale w+a, 2 T. = 15

    void Initialise(const byte pin);

    void Power(const byte pwr);

    void GetSample();

    void Get_Temperature();

    void Reset();

    byte Check_ChipID();

    void Set_Range_and_Conf_Regs(const byte acc_range_Mode, const byte acc_conf_Mode, const byte gyr_range_Mode, const byte gyr_conf_Mode);

    byte Self_Test();

    //byte Read_Register_Byte(const byte Addr, byte MaskBits = 0xFF, byte R_ShiftBits = 0);

    //void Write_Register_Byte(const byte Addr, const byte RegByte);
};

#endif // BMI160_h
