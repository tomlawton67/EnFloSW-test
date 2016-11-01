#include <Arduino.h>
#include "Therm.h"


void clsTherm::Initialise(const byte Pin)
{
  Pin_Therm = Pin;
  pinMode(Pin_Therm, INPUT);
  analogReadResolution(16);
  analogReadAveraging(16);
  analogReference(DEFAULT); // range 0 - 3.3V
}


void clsTherm::Get_Temperature()
{
  wTemperature = analogRead(Pin_Therm);

  float ADCvolts = (static_cast<float>(wTemperature) * 1.5258789E-5) * Vs;

  Temperature = ADCvoltsToTemp(ADCvolts); // deg C
}


void clsTherm::Get_wTemperature()
{
  wTemperature = analogRead(Pin_Therm);
}


inline float clsTherm::ADCvoltsToTemp(const float V)
{
  float R1 = ((Vs / V) - 1.0) * R2;
  float T = (1.0 / ((invB * log(R1 / R10)) + invT0)) - 273.15; // deg C

  return T;
}
