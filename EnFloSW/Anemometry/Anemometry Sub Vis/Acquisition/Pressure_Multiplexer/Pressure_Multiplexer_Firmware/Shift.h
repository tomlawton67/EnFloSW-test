#ifndef Shift_h
#define Shift_h

#include <Arduino.h>


// This class is the more generic variant for the Texas Instruments CD74AC164 8-bit serial-in/parallel-out shift register/
// Pins are user-defined on initialisation, at the (small) expense of absolute max. performance
class clsShiftReg
{
  private:
    byte Pin_Data;
    byte Pin_Clock;
    //byte Pin_Reset;


  public:
    byte nBits;
    byte Polarity;
    byte Polarity_;

    void Initialise(const byte DataPin, const byte ClockPin, const byte pol, const byte numBits); // const byte ResetPin

    //void Reset();

    void ShiftBit(const byte Bit);

    void ShiftBit_Multiple(const byte Bit, const byte n);

    void Clear();

    void Fill();

    void BeginSequence();

    void ClearThenSetSoloBit(const byte Pos);

    void AdvanceBit();

    void SetPolarity(const byte Pol);
};

#endif // Shift_h
