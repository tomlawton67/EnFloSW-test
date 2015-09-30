#include <SPI.h>

byte Pin_ChA, Pin_ChB, Pin_CS_Vm, Pin_CS_RPM, Pin_LDAC_Vm, Pin_LDAC_RPM, Pin_Spkr;
word PPR;
byte PPRmult;
volatile boolean A, B, dir;
volatile unsigned long t_now, t_old, dt;
float dtheta, RPM, GearRatio;
float Vm, Pi, Ii, Di, KP, KI, KD, TI, TD, RPM_set, err, err_old, dti, inv_dti, RPM_tol;

float maxVal_RPM, minVal_RPM, inv_max_minDivBits_RPM;
float inv_max_minDivBits_Vm, maxVal_Vm, minVal_Vm;
word DACval_RPM, DACval_Vm;

char Buffer[44];
char PPR_bytes[2];
char RPM_set_bytes[4], maxVal_RPM_bytes[4], minVal_RPM_bytes[4], RPM_tol_bytes[4];
char maxVal_Vm_bytes[4], minVal_Vm_bytes[4], GearRatio_bytes[4];
char KP_bytes[4], TI_bytes[4], TD_bytes[4];


void setup()
{

  Serial.begin(115200);

  // configure pins
  Pin_ChA = 2;
  Pin_ChB = 3;
  Pin_CS_Vm = 10;
  Pin_CS_RPM = 9;
  Pin_LDAC_Vm = 8;
  Pin_LDAC_RPM = 7;
  Pin_Spkr = A5;

  // Encoder and gearing params
  PPRmult = 1;
  PPR = 256;
  GearRatio = 14.0 / 57.0; // reduction (shaft : motor)

  // set Vmotor max and min eng values for -10...10V range
  maxVal_Vm = 6.0; // Vmotor
  minVal_Vm = -6.0; // Vmotor

  // set RPM DAC max and min eng values for 0...10V range
  maxVal_RPM = 1000.0; //RPM 
  minVal_RPM = -1000.0; //RPM

  // PID values
  RPM_set = 0.0;
  RPM_tol = 0.0;
  KP = 0.009;
  TI = 0.5; // Integral time constant (s)
  TD = 0.0000; // Derivative time constant (O(1) prediction of err at time t + TD) (s)

  KI = 0.5 * KP / TI;
  KD = KP * TD;

  dtheta = ((PI + PI) / float(PPR * PPRmult)) * GearRatio * 30.0 / PI; // units: RPM*s
  inv_max_minDivBits_Vm = 65535.0 / (10 - -10); // 16-bits. Fixed scaling from 10 to -10 keeps Vm 1:1 as demanded
  inv_max_minDivBits_RPM = 65535.0 / (maxVal_RPM - minVal_RPM); // 16-bits


  pinMode(Pin_ChA, INPUT);
  pinMode(Pin_ChB, INPUT);
  pinMode(Pin_CS_Vm, OUTPUT);
  pinMode(Pin_CS_RPM, OUTPUT);
  pinMode(Pin_LDAC_Vm, OUTPUT);
  pinMode(Pin_LDAC_RPM, OUTPUT);
  pinMode(Pin_Spkr, OUTPUT);

  digitalWrite(Pin_CS_Vm, HIGH);
  digitalWrite(Pin_CS_RPM, HIGH);
  digitalWrite(Pin_LDAC_Vm, HIGH);
  digitalWrite(Pin_LDAC_RPM, HIGH);


  // SPI DAC comms
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);

  // Set Pin numbers to port bytes for speed in loop
  Pin_ChA = (1 << PIND2);
  Pin_ChB = (1 << PIND3);
  Pin_CS_Vm = (1 << PINB2);
  Pin_CS_RPM = (1 << PINB1);
  Pin_LDAC_Vm = (1 << PINB0);
  Pin_LDAC_RPM = (1 << PIND7);
  Pin_Spkr = (1 << PINC5);

  // initialise variables and interrupts
  A = 0;//(PIND & Pin_ChA) >> PIND2; // always 0 in X1 mode on falling edge
  B = (PIND & Pin_ChB) >> PIND3;
  RPM = 0.0;
  Ii = 0.0;
  err_old = 0.0;
  dt = 2 * 60000000 / maxVal_RPM; // some sensible initial dt
  t_now = micros();

  switch (PPRmult)
  {
  case 1:
    attachInterrupt(0, ChA_edgeX1, FALLING);
    attachInterrupt(1, ChB_edgeX1X2, CHANGE);
    break;
  case 2:
    attachInterrupt(0, ChA_edgeX2X4, CHANGE);
    attachInterrupt(1, ChB_edgeX1X2, CHANGE);
    break;
  case 4:
    attachInterrupt(0, ChA_edgeX2X4, CHANGE);
    attachInterrupt(1, ChB_edgeX4, CHANGE);
    break;
  }

}


void loop()
{

  // PID controller loop ################
  // use dt from encoder pulses, as that actually corresponds to the dt between RPM updates
  //if (dt > 0) // should never happen
  //{
  dti = 0.000001 * static_cast<float>(dt);
  inv_dti = 1.0 / dti;
  RPM = dir ? (dtheta * inv_dti) : (-dtheta * inv_dti);
  //}

  err = RPM_set - RPM;

  if (abs(err - err_old) > RPM_tol) // only alter PID and drive if RPM has changed (note this misses case of const dt with non-zero err)
  {
    // PID controller ####################
    Pi = KP * err;
    Ii += KI * (err + err_old) * dti;
    Di = KD * (err - err_old) * inv_dti;
    Vm = constrain(Pi + Ii + Di, minVal_Vm, maxVal_Vm);
    // ####################################

    // OUTPUT Vd TO Vd DAC ################
    // re-scale Vd to 0...65535
    DACval_Vm = static_cast<word>(constrain(static_cast<long>(floor(((Vm + 10.0) * inv_max_minDivBits_Vm) + 0.5)), 0, 65535));
    // write to DAC chip
    PORTB |= Pin_LDAC_Vm; //digitalWrite(Pin_LDAC_Vm, HIGH); 
    PORTB &= ~Pin_CS_Vm; //digitalWrite(Pin_CS_Vm, LOW);
    SPI.transfer(static_cast<byte>((DACval_Vm & 0b1111111100000000) >> 8)); // High byte
    SPI.transfer(static_cast<byte>(DACval_Vm & 0b0000000011111111)); // Low byte
    PORTB |= Pin_CS_Vm; //digitalWrite(Pin_CS_Vm, HIGH);
    PORTB &= ~Pin_LDAC_Vm; //digitalWrite(Pin_LDAC_Vm, LOW);
    // ####################################

    // OUTPUT RPM TO RPM DAC ##############
    // re-scale RPM to 0...65535
    DACval_RPM = static_cast<word>(constrain(static_cast<long>(floor(((RPM - minVal_RPM) * inv_max_minDivBits_RPM) + 0.5)), 0, 65535));
    // write to DAC chip
    PORTD |= Pin_LDAC_RPM; //digitalWrite(Pin_LDAC_RPM, HIGH);
    PORTB &= ~Pin_CS_RPM; //digitalWrite(Pin_CS_RPM, LOW);
    SPI.transfer(static_cast<byte>((DACval_RPM & 0b1111111100000000) >> 8)); // High byte
    SPI.transfer(static_cast<byte>(DACval_RPM & 0b0000000011111111)); // Low byte
    PORTB |= Pin_CS_RPM; //digitalWrite(Pin_CS_RPM, HIGH);
    PORTD &= ~Pin_LDAC_RPM; //digitalWrite(Pin_LDAC_RPM, LOW);
    // #####################################

    err_old = err;
   // PORTC ^= Pin_Spkr; // useful for audible loop load cue
  }
    //PORTC ^= Pin_Spkr; // useful for audible loop load cue
}


void ChA_edgeX1()
{
  t_old = t_now;
  t_now = micros();
  dt = t_now - t_old;

  dir = A ^ B; // 1 clockwise, 0 anti-clockwise

  PORTC ^= Pin_Spkr;
}

void ChA_edgeX2X4()
{
  t_old = t_now;
  t_now = micros();
  dt = t_now - t_old;

  A = (PIND & Pin_ChA) >> PIND2;
  dir = A ^ B; // 1 clockwise, 0 anti-clockwise

  PORTC ^= Pin_Spkr;
}

void ChB_edgeX1X2()
{
  B = (PIND & Pin_ChB) >> PIND3;
}

void ChB_edgeX4()
{
  t_old = t_now;
  t_now = micros();
  dt = t_now - t_old;

  B = (PIND & Pin_ChB) >> PIND3;

  PORTC ^= Pin_Spkr;
}


void ReInitialiseVars()
{

  KI = 0.5 * KP / TI;
  KD = KP * TD;

  dtheta = ((PI + PI) / float(PPR * PPRmult)) * GearRatio * 30.0 / PI;
  inv_max_minDivBits_RPM = 65535.0 / (maxVal_RPM - minVal_RPM); // 16-bits

  // initialise variables and interrupts
  //A = 0;//(PIND & Pin_ChA) >> PIND2; // always 0 in X1 mode on falling edge
  //B = (PIND & Pin_ChB) >> PIND3;
  //RPM = 0.0;
  Ii = 0.0;
  err_old = 0.0;
  dt = 2 * 60000000 / maxVal_RPM; // some sensible initial dt
  t_now = micros();

  switch (PPRmult)
  {
  case 1:
    A = 0;
    attachInterrupt(0, ChA_edgeX1, FALLING);
    attachInterrupt(1, ChB_edgeX1X2, CHANGE);
    break;
  case 2:
    attachInterrupt(0, ChA_edgeX2X4, CHANGE);
    attachInterrupt(1, ChB_edgeX1X2, CHANGE);
    break;
  case 4:
    attachInterrupt(0, ChA_edgeX2X4, CHANGE);
    attachInterrupt(1, ChB_edgeX4, CHANGE);
    break;
  }

}


void serialEvent()
{
  if (Serial.read() == '@') // beginning of instruction in correct place, read until termination char '#'
  {
    // already discarded '@'
    // read all into buffer, then parse values as appropriate
    Serial.readBytesUntil('#', Buffer, sizeof(Buffer)); // does NOT include '#'
    //Serial.write(reinterpret_cast<byte*>(&Buffer), sizeof(Buffer)); // echo for debug only

    // check if setting RPM only, or full config.
    switch(Buffer[0])
    {
    case 1: // RPM set only
      RPM_set_bytes[0] = Buffer[4];
      RPM_set_bytes[1] = Buffer[3];
      RPM_set_bytes[2] = Buffer[2];
      RPM_set_bytes[3] = Buffer[1];

      memcpy(&RPM_set, &RPM_set_bytes, 4);
      //Ii = 0.0;
      //err_old = 0.0;
      dt = 2 * 60000000 / maxVal_RPM; // some sensible initial dt
      t_now = micros();

      break;
    case 0: // Full config.
      RPM_set_bytes[0] = Buffer[4];
      RPM_set_bytes[1] = Buffer[3];
      RPM_set_bytes[2] = Buffer[2];
      RPM_set_bytes[3] = Buffer[1];

      PPR_bytes[0] = Buffer[6];
      PPR_bytes[1] = Buffer[5];
      PPRmult = Buffer[7];

      maxVal_RPM_bytes[0] = Buffer[11];
      maxVal_RPM_bytes[1] = Buffer[10];
      maxVal_RPM_bytes[2] = Buffer[9];
      maxVal_RPM_bytes[3] = Buffer[8];
      minVal_RPM_bytes[0] = Buffer[15];
      minVal_RPM_bytes[1] = Buffer[14];
      minVal_RPM_bytes[2] = Buffer[13];
      minVal_RPM_bytes[3] = Buffer[12];
      RPM_tol_bytes[0] = Buffer[19];
      RPM_tol_bytes[1] = Buffer[18];
      RPM_tol_bytes[2] = Buffer[17];
      RPM_tol_bytes[3] = Buffer[16];

      maxVal_Vm_bytes[0] = Buffer[23];
      maxVal_Vm_bytes[1] = Buffer[22];
      maxVal_Vm_bytes[2] = Buffer[21];
      maxVal_Vm_bytes[3] = Buffer[20];
      minVal_Vm_bytes[0] = Buffer[27];
      minVal_Vm_bytes[1] = Buffer[26];
      minVal_Vm_bytes[2] = Buffer[25];
      minVal_Vm_bytes[3] = Buffer[24];
      GearRatio_bytes[0] = Buffer[31];
      GearRatio_bytes[1] = Buffer[30];
      GearRatio_bytes[2] = Buffer[29];
      GearRatio_bytes[3] = Buffer[28];

      KP_bytes[0] = Buffer[35];
      KP_bytes[1] = Buffer[34];
      KP_bytes[2] = Buffer[33];
      KP_bytes[3] = Buffer[32];
      TI_bytes[0] = Buffer[39];
      TI_bytes[1] = Buffer[38];
      TI_bytes[2] = Buffer[37];
      TI_bytes[3] = Buffer[36];
      TD_bytes[0] = Buffer[43];
      TD_bytes[1] = Buffer[42];
      TD_bytes[2] = Buffer[41];
      TD_bytes[3] = Buffer[40];

      memcpy(&RPM_set, &RPM_set_bytes, 4);

      memcpy(&PPR, &PPR_bytes, 2);

      memcpy(&maxVal_RPM, &maxVal_RPM_bytes, 4);
      memcpy(&minVal_RPM, &minVal_RPM_bytes, 4);
      memcpy(&RPM_tol, &RPM_tol_bytes, 4);

      memcpy(&maxVal_Vm, &maxVal_Vm_bytes, 4);
      memcpy(&minVal_Vm, &minVal_Vm_bytes, 4);
      memcpy(&GearRatio, &GearRatio_bytes, 4);

      memcpy(&KP, &KP_bytes, 4);
      memcpy(&TI, &TI_bytes, 4);
      memcpy(&TD, &TD_bytes, 4);

      ReInitialiseVars();
      break; 
    }    

  }
  else
  {
    // discard entire buffer
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }
}




































