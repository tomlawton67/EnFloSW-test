#ifndef Shift_OPT_h
#define Shift_OPT_h

#include <Arduino.h>

#define Pin_Data 10
#define Pin_Clock 14

// This class is "tuned" specially for the Texas Instruments CD74AC164 8-bit serial-in/parallel-out shift register timings.
// Fastest performance when pins are declared as constants (or static const within a class).
// All timings already exceed minimum requirements of the IC, even at 144MHz.
// Alter pin numbers here as necessary.
class clsShiftReg_OPT
{
  private:
    //static const byte Pin_Reset = 2;


  public:
    byte nBits;
    byte Polarity;
    byte Polarity_;

    void Initialise(const byte pol, const byte numBits);

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

#endif // Shift_OPT_h
