#include "Shift_OPT.h"


void clsShiftReg_OPT::Initialise(const byte Pol, const byte numBits)
{
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


void clsShiftReg_OPT::ShiftBit(const byte Bit)
{
  digitalWriteFast(Pin_Data, Bit);
  digitalWriteFast(Pin_Clock, HIGH);
  __asm__ __volatile__("nop\n\t""nop\n\t""nop\n\t"::); // 2 nops needed at 144MHz (13.8ns)
  digitalWriteFast(Pin_Clock, LOW);
}


void clsShiftReg_OPT::ShiftBit_Multiple(const byte Bit, const byte n)
{
  for (byte i = 0; i < n; i++)
  {
    ShiftBit(Bit);
  }
}


void clsShiftReg_OPT::Clear()
{
  ShiftBit_Multiple(Polarity_, nBits);
}


void clsShiftReg_OPT::Fill()
{
  ShiftBit_Multiple(Polarity, nBits);
}


void clsShiftReg_OPT::BeginSequence()
{
  Clear();
  ShiftBit(Polarity);
}


void clsShiftReg_OPT::ClearThenSetSoloBit(const byte Pos) // 80ns transient, 280ns stationary (at 144MHz)
{
  Clear();
  ShiftBit(Polarity);
  ShiftBit_Multiple(Polarity_, Pos);
}


void clsShiftReg_OPT::AdvanceBit()
{
  ShiftBit(Polarity_);
}


void clsShiftReg_OPT::SetPolarity(const byte Pol)
{
  Polarity = Pol;
  Polarity_ = !Pol;
}


//void clsShiftReg_OPT::Reset()
//{
//  digitalWriteFast(Pin_Data, LOW);
//  digitalWriteFast(Pin_Clock, LOW);
//
//  digitalWriteFast(Pin_Reset, LOW);
//  __asm__ __volatile__("nop\n\t"::); // needed at 144MHz (6.9ns) [only for OPT version]
//  digitalWriteFast(Pin_Reset, HIGH);
//}
