#include "stm32yyxx_ll_adc.h"

/* Values available in datasheet */
#define CALX_TEMP 25
#if defined(STM32F1xx)
#define V25 1430
#define AVG_SLOPE 4300
#define VREFINT 1200
#elif defined(STM32F2xx) || defined(STM32F4xx)
#define V25 760
#define AVG_SLOPE 2500
#define VREFINT 1210
#endif

/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096

//#define LED_BUILTIN 13

// // the setup routine runs once when you press reset:
// void setup()
// {
//   // initialize serial communication at 9600 bits per second:
//   pinMode(LED_BUILTIN, OUTPUT);
//   digitalWrite(LED_BUILTIN, 1);

//   Serial.begin(115200);
//   // delay(1000);
//   // Serial.println(F("Starting... "));
//   // delay(1000);
//   // digitalWrite(LED_BUILTIN, HIGH);

//   // while (!Serial);
//   analogReadResolution(12);
// }

static int32_t readVref()
{
  return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
}

static int32_t readTempSensor(int32_t VRef)
{
  return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
}

static int32_t readVoltage(int32_t VRef, uint32_t pin)
{
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(VRef, analogRead(pin), LL_ADC_RESOLUTION));
}

static String readValues_Demo()
{
  //return "{\"d\":\"95\",\"v\":\"210\",\"c\":\"1.2\",\"p\":\"0.1\"}";

  int32_t VRef = readVref();
  return "{\"v\":" + String((int)VRef) 
  + ",\"c\":" + String((int)readTempSensor(VRef)) 
  + ",\"p\":" + String((int)readVoltage(VRef, A0)) 
  + "}";
}

// // The loop routine runs over and over again forever:
// void loop()
// {
//   //int32_t VRef = readVref();
//   // Serial.print("VRef(mv)= " + VRef);
//   // Serial.printf("\t");
//   Serial.print(readValues() + "\r\n");
//   // Serial.printf("\tTemp= %i", readTempSensor(VRef));
//   // Serial.printf("\tA0(mv)= %i\r\n", readVoltage(VRef, A0));
//   delay(2000);
// }
