#include "Shift.h"


void clsShiftReg::Initialise(const byte DataPin, const byte ClockPin, const byte Pol, const byte numBits) // const byte ResetPin
{
  Pin_Data = DataPin;
  Pin_Clock = ClockPin;
  //Pin_Reset = ResetPin;

  pinMode(Pin_Data, OUTPUT);
  pinMode(Pin_Clock, OUTPUT);
  //pinMode(Pin_Reset, OUTPUT);

  digitalWriteFast(Pin_Data, LOW);
  digitalWriteFast(Pin_Clock, LOW);
  //digitalWriteFast(Pin_Reset, HIGH);

  SetPolarity(Pol);
  nBits = numBits;

  //Reset();
  Clear();
}


void clsShiftReg::ShiftBit(const byte Bit)
{
  digitalWriteFast(Pin_Data, Bit);
  digitalWriteFast(Pin_Clock, HIGH);
  __asm__ __volatile__("nop\n\t""nop\n\t"::); // 2 nops needed at 144MHz (13.8ns)  [OPT version only, but add here anyway]
  digitalWriteFast(Pin_Clock, LOW);
}


void clsShiftReg::ShiftBit_Multiple(const byte Bit, const byte n)
{
  for (byte i = 0; i < n; i++)
  {
    ShiftBit(Bit);
  }
}


void clsShiftReg::Clear()
{
  ShiftBit_Multiple(Polarity_, nBits);
}


void clsShiftReg::Fill()
{
  ShiftBit_Multiple(Polarity, nBits);
}


void clsShiftReg::BeginSequence()
{
  Clear();
  ShiftBit(Polarity);
}


void clsShiftReg::ClearThenSetSoloBit(const byte Pos) // 200ns transient, 600ns stationary (at 144MHz)
{
  Clear();
  ShiftBit(Polarity);
  ShiftBit_Multiple(Polarity_, Pos);
}


void clsShiftReg::AdvanceBit()
{
  ShiftBit(Polarity_);
}


void clsShiftReg::SetPolarity(const byte Pol)
{
  Polarity = Pol;
  Polarity_ = !Pol;
}


//void clsShiftReg::Reset()
//{
//  digitalWriteFast(Pin_Data, LOW);
//  digitalWriteFast(Pin_Clock, LOW);
//
//  digitalWriteFast(Pin_Reset, LOW);
//  //  __asm__ __volatile__("nop\n\t"::); // needed at 144MHz (6.9ns) [only for OPT version]
//  digitalWriteFast(Pin_Reset, HIGH);
//}
