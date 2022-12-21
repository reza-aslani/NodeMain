//#Pin set name#
int DimmerPin = PA0;
int CurrentPin = PA2;
int VoltagePin = PA1;
int LightPin = PB9;

//# Value of Parameters #
int dimmer = 100;
int light = 0;

void setup_real_values()
{
 //mySerial.println("--------------- Start ");
  // Serial.begin(9600);
  //  Set Pin Mode
  pinMode(PA0, OUTPUT);
  pinMode(PA1, INPUT);
  pinMode(PA2, INPUT);
  pinMode(PB9, OUTPUT);
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13,HIGH);
  SetLightDimmer(5);
  SetLightPower(false);
  delay(3000);
  SetLightPower(true);
  digitalWrite(PC13,LOW);
}

// void loop()
// {
//     Serial.print("voltage:");
//     Serial.print(ReadVoltage());
//     Serial.print(", Current:");
//     Serial.println(ReadCurrent());
//     SetLightDimmer(dimmer);
// }

void SetLightDimmer(int val)
{
  dimmer = val;
  if (dimmer > 100)
  {
    dimmer = 100;
  }

  analogWrite(DimmerPin, map(dimmer, 0, 100, 0, 255));
}

int GetLightDimmer()
{
  return dimmer;
}

void SetLightPower(bool val)
{
  if (val == true)
  {
    digitalWrite(LightPin, HIGH);
  }
  else if (val == false)
  {
    digitalWrite(LightPin, LOW);
  }
}

static String readValues_Real()
{
  // return "{\"d\":\"95\",\"v\":\"210\",\"c\":\"1.2\",\"p\":\"0.1\"}";

  return "{\"v\":" + String(ReadVoltage()) + ",\"c\":" + String(ReadCurrent()) + ",\"p\":" + String(0) + "}";
}

int ReadVoltage()
{
  int sensorValue = 0;
  // for (int i = 0; i < 100; i++)
  {
    sensorValue = (int)(analogRead(VoltagePin) / 0.9312033898305085);
    delay(1);
  }
  int voltage = sensorValue -10;
  return voltage;
}

float ReadCurrent()
{
  float sensorValue = 0;
  for (int i = 0; i < 100; i++)
  {
    sensorValue += (float)(analogRead(CurrentPin) * 0.01220703125);
    delay(1);
  }
  float current = sensorValue / 100;
  return current;
}

// void serialEvent()
// {
//     while (Serial.available())
//     {
//         dimmer += 10;
//         if (dimmer == 100)
//             dimmer = 0;
//         break;
//     }
// }
