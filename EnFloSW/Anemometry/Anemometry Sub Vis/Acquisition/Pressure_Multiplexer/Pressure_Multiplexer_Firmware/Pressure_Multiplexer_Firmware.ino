// Firmware for 64 channel HSC Pressure Multiplexer. Written by Paul Nathan, finalised 25/02/2016
// Set to 144MHz optimised, USB serial. Check "O3" opt. in boards.txt. Check 4ms delay in pins_teensy.c.

#pragma GCC optimize ("O3")

#include <EEPROM.h>
#include "BME280.h"
#include "BMI160.h"
#include "Press_NoCS.h"
#include "SPI_EEPROM_NoCS.h"
#include "Shift.h"
#include "Shift_OPT.h"
#include "Therm.h"


clsBME280 BME280;
clsBMI160 BMI160;
clsPress_NoCS* Pressure = NULL;
clsSPI_EEPROM_NoCS SPI_EEPROM;
clsShiftReg ShiftReg_EEPROM;
clsShiftReg ShiftReg_OkLED;
clsShiftReg_OPT ShiftReg_Pressure;
clsTherm Thermistor;

const byte EEPROM_PageSize = 16; // bytes
const byte EEPROM_nPages = 8;
const byte nSlots = 8;
const byte nSensorsPerSlot = 8;
const byte maxSensors = nSlots * nSensorsPerSlot;

byte nSlots_Used = 0;
byte ActiveSensors[maxSensors];
byte FaultySensors[maxSensors];
byte nFaults = 0;
byte maxSensorNum = 0;

const byte Pin_PWR = 0;
const byte Pin_StreamLED = 1;
const byte Pin_CS_BMI160 = 2;
const byte Pin_Sync_In = 3; // Falling edge trigger - input pullup, ISR
const byte Pin_Sync_Out = 4; // Falling edge upon start - output
const byte Pin_CS_BME280 = 9;
const byte Pin_ShiftReg_OkLED_Clock = 20;
const byte Pin_ShiftReg_OkLED_Data = 21;
const byte Pin_ShiftReg_EEPROM_Clock = 22;
const byte Pin_ShiftReg_EEPROM_Data = 23;
const byte Pin_Thermistor = A4;

elapsedMillis USB_Interval;
unsigned long USB_dt = 10; // ms

elapsedMillis StateData_Interval;
unsigned long StateData_dt = 1000; // ms

IntervalTimer StreamInterval;
float Stream_dt = 10000.0; // us

byte Mode = 0; // 0 Fast pressure stream, 1 Full pressure stream (P & T)

const byte L_usb = 6;
char USB_Buffer[L_usb]; // '@', Cmd, [x, x, x, x]

word L_data = 0;
byte L_data_fast = 0;
word L_data_full = 0;
char* DATA_Buffer = NULL; // '#', dt_warn, [x]? (Length depends on number of detected cards and Mode)

const byte L_mem = (EEPROM_PageSize * EEPROM_nPages) + 3;
char MEM_Buffer[L_mem]; // '%', slot, 'R'/'W', [x]EEPROM_TotalSize

const byte L_mem2 = 30;
const byte L_mem2m2 = 28;
char MEM2_Buffer[L_mem2]; // '~', 'R'/'W', [x]EEPROM(mcu)28 On Teensy EEPROM, for storing accelgyro constants

const byte L_state = 30;
char STATE_Buffer[L_state]; // '$', [2 Therm], [4 T, 4 P, 4 H], [2 wx, 2 wy, 2 wz, 2 ax, 2 ay, 2 az, 1 stale w+a, 2 T]

volatile boolean Pulse = false;
boolean Pulsing = false;

unsigned long t_start, t_end, dt; //us
const float dt_Margin = 0.05; // rel. error tolerance in dt
float dt_Margin_x_Stream_dt = dt_Margin * Stream_dt;

byte BME280_ok = 0;
byte BMI160_ok = 0;
byte Therm_ok = 0;
byte PowerState = 0;


void setup()
{
  // Configure pins that are not configured in classes
  Configure_Pins();

  Zero_All_ShiftRegs();

  // Initiliase buffer headers (except for DATA_Buffer, must be done after power-up sequence)
  USB_Buffer[0] = '@';
  MEM_Buffer[0] = '%';
  STATE_Buffer[0] = '$';

  // Configure sync in pin
  attachInterrupt(Pin_Sync_In, Trigger_Start, FALLING);

  Serial.begin(12160000);
}


FASTRUN void loop()
{
  while (true) // optimisation
  {

    if (Pulse)
    {
      Pulse = false;

      switch (Mode)
      {
        case 0:
          // Pressure stream (fast)
          PressureCycle_Fast();
          break;
        case 1:
          // Pressure & die temp. stream (full)
          PressureCycle_Full();
          break;
        default:
          ;
      }

      // warn if dt rel error is too high
      t_end = micros();
      dt = t_end - t_start;
      t_start = t_end;
      DATA_Buffer[1] = ((float)abs(dt - Stream_dt) > dt_Margin_x_Stream_dt) ? 1 : 0;

      Serial.write(DATA_Buffer, L_data);
    }

    if (PowerState)
    {
      if (StateData_Interval >= StateData_dt)
      {
        StateData_Interval = 0;
        Get_State_Data();

        if (Pulsing) Serial.write(STATE_Buffer, L_state); // only send out if streaming, else only collect data and send out on request cmd only.
      }
    }

    if (USB_Interval >= USB_dt)
    {
      USB_Interval = 0;
      USB_Check();
    }

  }
}


void Configure_Pins()
{
  pinMode(Pin_PWR, OUTPUT);
  digitalWriteFast(Pin_PWR, LOW);

  pinMode(Pin_StreamLED, OUTPUT);
  digitalWriteFast(Pin_StreamLED, LOW);

  pinMode(Pin_Sync_In, INPUT_PULLUP);

  pinMode(Pin_Sync_Out, OUTPUT);
  digitalWriteFast(Pin_Sync_Out, HIGH);


  // Set SPI pins here to ensure good power-down state upon first plugging in
  pinMode(12, INPUT); // MISO
  pinMode(11, OUTPUT); // MOSI
  digitalWriteFast(11, LOW);
  pinMode(13, OUTPUT); // SCLK
  digitalWriteFast(13, LOW);

  pinMode(Pin_CS_BME280, OUTPUT);
  digitalWriteFast(Pin_CS_BME280, HIGH);

  pinMode(Pin_CS_BMI160, OUTPUT);
  digitalWriteFast(Pin_CS_BMI160, HIGH);
}


void Zero_All_ShiftRegs()
{
  ShiftReg_Pressure.Initialise(1, maxSensors); // This is the optimised class, so no pin inputs here (hard-coded in class). If run with no power, this line only serves to set the pins to output mode and low
  ShiftReg_EEPROM.Initialise(Pin_ShiftReg_EEPROM_Data, Pin_ShiftReg_EEPROM_Clock, 1, nSlots);
  ShiftReg_OkLED.Initialise(Pin_ShiftReg_OkLED_Data, Pin_ShiftReg_OkLED_Clock, 1, nSlots);
}


void Initialise_Subsystems()
{
  // Before doing anything, it is essential to set high all CS inputs on all pressure transducers and EEPROMs
  ShiftReg_Pressure.Initialise(0, maxSensors); // This is the optimised class, so no pin inputs here (hard-coded in class). In this context, there should be power, so this will actually initialise the shift registers
  ShiftReg_EEPROM.Initialise(Pin_ShiftReg_EEPROM_Data, Pin_ShiftReg_EEPROM_Clock, 0, nSlots);
  ShiftReg_OkLED.Initialise(Pin_ShiftReg_OkLED_Data, Pin_ShiftReg_OkLED_Clock, 1, nSlots);

  SPI_EEPROM.Initialise(EEPROM_PageSize, EEPROM_nPages);

  BMI160.Initialise(Pin_CS_BMI160);
  BMI160.Set_Range_and_Conf_Regs(0, 1, 0, 0);
  BME280.Initialise(Pin_CS_BME280);
  Thermistor.Initialise(Pin_Thermistor);
}


void Power_ON() // ensure this is only entered if PowerState = 0 (to avoid memory leak with 'new' DATA_Buffer)
{
  digitalWriteFast(Pin_PWR, HIGH);

  delay(10); // 10ms power supply settling time (5ms required by pressure sensors)

  // Initialise shift registers, classes and on-board sensors
  Initialise_Subsystems();

  ShiftReg_OkLED.ShiftBit(1); // Prepare to use OkLEDs as progress indicator (already initialised within above subroutine)

  nSlots_Used = 0;
  maxSensorNum = 0;
  memset(&ActiveSensors[0], 0, maxSensors);

  // Loop through slots, check EEPROM is present (thereby card), then check EEPROM page values for presence of sensor (all zero implies absence). Mark as appropriate
  byte i, j;
  byte addr;
  byte PageBytes[EEPROM_PageSize];
  for (i = 0; i < nSlots; i++)
  {
    if (SPI_EEPROM_READ_StatusReg(i) == 0)
    {
      // Card i is present and EEPROM is okay, now check for sensors present (via EEPROM info)
      ++nSlots_Used;

      addr = 0;
      for (j = 0; j < nSensorsPerSlot; j++)
      {
        SPI_EEPROM_READ_page(i, addr, PageBytes);
        if (PageSum(PageBytes, EEPROM_PageSize) > 0)
        {
          // Sensor j on card i is (supposedly) present, mark in active list and increment
          maxSensorNum = (i * nSensorsPerSlot) + j;
          ActiveSensors[maxSensorNum] = 1;
          ++maxSensorNum; // convert to actual count rather than index
        }
        addr += EEPROM_PageSize; // increment EEPROM address by page size and continue loop
      }
    }
    delay(125); // Slow down to allow user to see!
    ShiftReg_OkLED.ShiftBit(0); // increment single OkLED to next slot to show progress
  }

  // Next, knowing how many sensors are present, configure the pressure sensors' shift registers and check for sensor response (zero in status bits is good) AND temperature returned in operating range (-20...+85)
  Set_maxSensorNum(maxSensorNum);

  float T;
  byte S;
  byte Pass = 1;
  byte FaultArray[nSlots]; // stores the fault flags, to be subsequently read in reverse for ShiftReg_OkLED
  byte currentSlot = 0;

  ShiftReg_OkLED.Clear(); // Clear OkLEDs to use again as progress indicator
  ShiftReg_OkLED.ShiftBit(1);
  memset(&FaultArray[0], 0, nSlots);
  nFaults = 0;
  memset(&FaultySensors[0], 0, maxSensors);
  for (i = 0; i < maxSensorNum; i++)
  {
    if (ActiveSensors[i])
    {
      Pressure[i].Initialise();

      // First ever sample is always marked as "stale". Get several samples and work off last one.
      for (byte k = 0; k < 4; k++)
      {
        ShiftReg_Pressure.ClearThenSetSoloBit(i);
        Pressure[i].GetSample_Full();
        ShiftReg_Pressure.Clear();
        delay(4); // wait long enough so second sample is not stale(!). >1ms is ok.
      }

      T = Pressure[i].CalcTemperature();
      S = Pressure[i].ExtractStatusBits_fromFull();
      if (S == 0) // Status ok
      {
        if ((T >= -20.0) && (T <= 85.0)) // temperature in operating range
        {
          // This sensor is okay
          ;
        }
        else
        {
          FaultySensors[i] = 1;
          ++nFaults;
          Pass = 0;
        }
      }
      else
      {
        FaultySensors[i] = 1;
        ++nFaults;
        Pass = 0;
      }
    }

    if ((((i + 1) % nSensorsPerSlot) == 0) || ((i + 1) == maxSensorNum))
    {
      // Next sensor is on next card up OR is the very last sensor, set the LED now depending on pass byte, then reset pass byte
      //delay(125); // no need for this, already slow enough
      ShiftReg_OkLED.ShiftBit(0); // moves single LED to next slot (to indicate progress of self-test)
      FaultArray[currentSlot] = Pass;
      ++currentSlot;
      Pass = 1;
    }

  }

  // Finally, set OkLEDs
  ShiftReg_OkLED.Clear();
  for (i = 0; i < nSlots; i++)
  {
    ShiftReg_OkLED.ShiftBit(FaultArray[nSlots - i - 1]);
  }

  PowerState = 1;
}


void Power_OFF() // ensure this is only entered if PowerState = 1 (to avoid deleting null dynamic arrays)
{
  // Power down BME280 and BMI160 first
  BME280.Power(0);
  BMI160.Power(0);

  digitalWriteFast(Pin_PWR, LOW);

  Zero_All_ShiftRegs();

  delete[] Pressure;
  Pressure = NULL;

  delete[] DATA_Buffer;
  DATA_Buffer = NULL;

  BME280_ok = 0;
  BMI160_ok = 0;
  Therm_ok = 0;
  PowerState = 0;
  PowerState = 0;
}


FASTRUN void PressureCycle_Fast()
{
  ShiftReg_Pressure.BeginSequence();
  for (byte i = 0; i < maxSensorNum; i++)
  {
    if (ActiveSensors[i])
    {
      Pressure[i].GetSample_Fast();
    }
    ShiftReg_Pressure.AdvanceBit();
  }

  //copy to data serial buffer
  byte c = 2;
  for (byte i = 0; i < maxSensorNum; i++)
  {
    if (ActiveSensors[i])
    {
      memcpy(&DATA_Buffer[c], &Pressure[i].Buffer_Fast[0], clsPress_NoCS::L_Buffer_Fast);
      c += clsPress_NoCS::L_Buffer_Fast;
    }
  }
}


FASTRUN void PressureCycle_Full()
{
  ShiftReg_Pressure.BeginSequence();
  for (byte i = 0; i < maxSensorNum; i++)
  {
    if (ActiveSensors[i])
    {
      Pressure[i].GetSample_Full();
    }
    ShiftReg_Pressure.AdvanceBit();
  }

  // copy to data serial buffer
  word c = 2; // can exceed 255 with full banks!
  for (byte i = 0; i < maxSensorNum; i++)
  {
    if (ActiveSensors[i])
    {
      memcpy(&DATA_Buffer[c], &Pressure[i].Buffer_Full[0], clsPress_NoCS::L_Buffer_Full);
      c += clsPress_NoCS::L_Buffer_Full;
    }
  }
}


FASTRUN void Get_State_Data()
{
  BME280.Get_TPH();
  BMI160.GetSample();
  BMI160.Get_Temperature();
  Thermistor.Get_wTemperature();

  // copy to data serial buffer (uses 1 + 2 + 4 + 4 + 4 + 15 = 30 bytes)
  //                                [hdr] [therm] [BME280] [BMI160]
  //                                '$', [2 Therm], [4 T, 4 P, 4 H], [2 wx, 2 wy, 2 wz, 2 ax, 2 ay, 2 az, 1 stale w+a, 2 T]
  memcpy(&STATE_Buffer[1], &Thermistor.wTemperature, 2);
  memcpy(&STATE_Buffer[3], &BME280.Temperature, 4);
  memcpy(&STATE_Buffer[7], &BME280.Pressure, 4);
  memcpy(&STATE_Buffer[11], &BME280.Humidity, 4);
  memcpy(&STATE_Buffer[15], &BMI160.Buffer[0], clsBMI160::L_Buffer);
}


FASTRUN void USB_Check()
{
  if (Serial.available())
  {
    if (Serial.peek() == '@') // check for standard cmd packet start
    {
      Serial.readBytes(USB_Buffer, L_usb); // read usb packet
      switch (USB_Buffer[1]) // check 'cmd' char
      {
        case 'P': // Power ON
          if (!PowerState) Power_ON();
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'p': // Power OFF
          if (PowerState) Power_OFF();
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'R': // Reset
          if (PowerState)
          {
            Power_OFF();
            delay(1000);
            Power_ON();
          }
          else
          {
            Power_ON();
          }
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'S': // Start streaming
          if (PowerState) StartPulsing();

          break;
        case 's': // Stop streaming
          StopPulsing();

          break;
        case 'T': // Set stream dt (us)
          memcpy(&Stream_dt, &USB_Buffer[2], 4);
          dt_Margin_x_Stream_dt = dt_Margin * Stream_dt;
          Serial.write(USB_Buffer, L_usb);

          break;
        case 't': // Set state data dt (ms)
          memcpy(&StateData_dt, &USB_Buffer[2], 4);
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'M': // Set data mode and then send back L_data
          SetMode(static_cast<byte>(USB_Buffer[2]));
          memcpy(&USB_Buffer[2], &L_data, 2);
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'O':
          OverrideSensorNum(static_cast<byte>(USB_Buffer[2]));
          memcpy(&USB_Buffer[2], &L_data, 2);
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'G': // Get status bits [PowerState, nSlots_Used, maxSensorNum, nFaults]
          USB_Buffer[2] = PowerState;
          USB_Buffer[3] = nSlots_Used;
          USB_Buffer[4] = maxSensorNum;
          USB_Buffer[5] = nFaults;
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'g': // Get sub-status bits [BME280_ok, BMI160_ok, Therm_ok, Mode]
          if (PowerState) Check_Subsystems();
          USB_Buffer[2] = BME280_ok;
          USB_Buffer[3] = BMI160_ok;
          USB_Buffer[4] = Therm_ok;
          USB_Buffer[5] = Mode;
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'F': // Check sensor[i] fault flag
          USB_Buffer[2] = FaultySensors[static_cast<byte>(USB_Buffer[2])];
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'A': // Check sensor[i] active flag
          USB_Buffer[2] = ActiveSensors[static_cast<byte>(USB_Buffer[2])];
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'C': // Configure AccelGyro (Acc. range (g), Acc. rate (Hz), Gyro range (dps), Gyro rate (Hz))
          BMI160.Set_Range_and_Conf_Regs(static_cast<byte>(USB_Buffer[2]), static_cast<byte>(USB_Buffer[3]), static_cast<byte>(USB_Buffer[4]), static_cast<byte>(USB_Buffer[5]));
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'Z': // Read Serial number
          Read_SerNum();
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'z': // Write Serial number
          Write_SerNum();
          Serial.write(USB_Buffer, L_usb);

          break;
        case 'D': // Write STATE_Buffer (most recent data)
          Serial.write(STATE_Buffer, L_state);

          break;
        default:
          ;
      }
    }
    else if (Serial.peek() == '%') // check for EEPROM cmd data packet start
    {
      Serial.readBytes(MEM_Buffer, L_mem); // read usb packet
      byte slotNum = static_cast<byte>(MEM_Buffer[1]); // read slot number
      switch (MEM_Buffer[2]) // check 'R/W' char
      {
        case 'R': // Read
          Read_EEPROM(slotNum);
          Serial.write(MEM_Buffer, L_mem);

          break;
        case 'W': // Write
          Write_EEPROM(slotNum);
          Serial.write(MEM_Buffer, L_mem);

          break;
        default:
          ;
      }
    }
    else if (Serial.peek() == '~') // check for MEM2 cmd data packet start
    {
      Serial.readBytes(MEM2_Buffer, L_mem2); // read usb packet
      switch (MEM2_Buffer[1]) // check 'R/W' char
      {
        case 'R': // Read
          Read_MEM2();
          Serial.write(MEM2_Buffer, L_mem);

          break;
        case 'W': // Write
          Write_MEM2();
          Serial.write(MEM2_Buffer, L_mem);

          break;
        default:
          ;
      }
    }
    else
    {
      // discard entire buffer
      while (Serial.available())
      {
        Serial.read();
      }
    }
  }
}


void Check_Subsystems()
{
  BME280_ok = BME280.Check_ChipID();
  BMI160_ok = (BMI160.Check_ChipID() + BMI160.Self_Test()) == 4;
  Thermistor.Get_Temperature();
  Therm_ok = ((Thermistor.Temperature >= -40.0) && (Thermistor.Temperature <= 125.0)) ? 1 : 0; // thermistor in operating range
}


void SetMode(const byte newMode)
{
  Mode = newMode;
  switch (newMode)
  {
    case 0:
      L_data = L_data_fast;
      break;
    case 1:
      L_data = L_data_full;
      break;
    default:
      ;
  }
  delete[] DATA_Buffer;
  DATA_Buffer = NULL;
  DATA_Buffer = new char[L_data];
  DATA_Buffer[0] = '#';
}


void Set_maxSensorNum(const byte newVal)
{
  maxSensorNum = newVal;

  ShiftReg_Pressure.Initialise(0, maxSensorNum); // This is the optimised class, so no pin inputs here (hard-coded in class)

  delete[] Pressure;
  Pressure = NULL;
  Pressure = new clsPress_NoCS[maxSensorNum];

  L_data_full = maxSensorNum * clsPress_NoCS::L_Buffer_Full + 2;
  L_data_fast = maxSensorNum * clsPress_NoCS::L_Buffer_Fast + 2;
  SetMode(Mode); // to set L_data and initialise DATA_Buffer
}


void OverrideSensorNum(const byte Num)
{
  Set_maxSensorNum(Num);
  nSlots_Used = max(1, Num >> 3); // floor(Num / 8)
  memset(&ActiveSensors[0], 0, maxSensors); // clear first
  memset(&ActiveSensors[0], 1, Num); // flag as active up to Num
}


void StartPulsing()
{
  StreamInterval.priority(1); // Top priority for sample clock (after systick at 0)
  StreamInterval.begin(Tick, Stream_dt);

  digitalWriteFast(Pin_Sync_Out, LOW);
  digitalWriteFast(Pin_StreamLED, HIGH);
  Pulsing = true;

  t_start = micros();
}


void StopPulsing()
{
  StreamInterval.end();
  digitalWriteFast(Pin_Sync_Out, HIGH);
  digitalWriteFast(Pin_StreamLED, LOW);
  Pulsing = false;
}


void Trigger_Start()
{
  if (PowerState) StartPulsing();
}


void Tick()
{
  Pulse = true;
}


void Read_EEPROM(const byte slotNum)
{
  byte tmp[SPI_EEPROM.TotalSize];

  SPI_EEPROM_READ_all(slotNum, tmp);

  memcpy(&MEM_Buffer[3], &tmp[0], SPI_EEPROM.TotalSize);
}


void Write_EEPROM(const byte slotNum)
{
  byte tmp[SPI_EEPROM.TotalSize];

  memcpy(&tmp[0], &MEM_Buffer[3], SPI_EEPROM.TotalSize);

  SPI_EEPROM_WRITE_all(slotNum, tmp);
}


void Read_MEM2()
{
  for (byte i = 2; i < L_mem2; i++)
  {
    MEM2_Buffer[i] = EEPROM.read(i);
  }
}


void Write_MEM2()
{
  for (byte i = 2; i < L_mem2; i++)
  {
    EEPROM.write(i, MEM2_Buffer[i]);
  }
}


void Read_SerNum()
{
  USB_Buffer[2] = EEPROM.read(0);
  USB_Buffer[3] = EEPROM.read(1);
}


void Write_SerNum()
{
  EEPROM.write(0, USB_Buffer[2]);
  EEPROM.write(1, USB_Buffer[3]);
}


word PageSum(byte PageBytes[], byte Len)
{
  word sum = 0;
  for (byte i = 0; i < Len; i++)
  {
    sum += PageBytes[i];
  }
  return sum;
}

