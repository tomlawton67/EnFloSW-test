#ifndef Therm_h
#define Therm_h


class clsTherm
{
  private:
    const float invB = 1.0 / 4250.0; // 25 - 50 degC
    const float R2 = 100000.0;
    const float R10 = 100000.0;
    const float invT0 = 1.0 / 298.15;
    const float Vs = 3.30; // Supply for thermistor circuit
    byte Pin_Therm;

    inline float ADCvoltsToTemp(const float V);


  public:
    float Temperature = 25.0; // deg C
    word wTemperature = 0x1900;

    void Initialise(const byte Pin);

    void Get_Temperature();

    void Get_wTemperature();
};

#endif // Therm_h
