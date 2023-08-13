#include <LiquidCrystal_I2C.h>

// Initialize LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte PulsesPerRevolution = 2;
const unsigned long ZeroTimeout = 100000;
const byte numReadings = 2;

volatile unsigned long LastTimeWeMeasured;
volatile unsigned long PeriodBetweenPulses = ZeroTimeout + 1000;
volatile unsigned long PeriodAverage = ZeroTimeout + 1000;
unsigned long FrequencyRaw;
unsigned long FrequencyReal;
unsigned long RPM;
unsigned int PulseCounter = 1;
unsigned long PeriodSum;

unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;
unsigned long CurrentMicros = micros();
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;
unsigned long readings[numReadings];
unsigned long readIndex;  
unsigned long total; 
unsigned long average;

void Pulse_Event() {
  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros();
  if (PulseCounter >= AmountOfReadings)  {
    PeriodAverage = PeriodSum / AmountOfReadings;
    PulseCounter = 1;
    PeriodSum = PeriodBetweenPulses;

    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  } else {
    PulseCounter++;
    PeriodSum = PeriodSum + PeriodBetweenPulses;
  }
}

void Displayrpm(){
    LastTimeCycleMeasure = LastTimeWeMeasured;
  CurrentMicros = micros();
  if (CurrentMicros < LastTimeCycleMeasure) 
  {
    LastTimeCycleMeasure = CurrentMicros;
  }
  FrequencyRaw = 10000000000 / PeriodAverage;
  if (PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra) {
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;
  } else {
    ZeroDebouncingExtra = 0;
  }
  FrequencyReal = FrequencyRaw / 10000;

  RPM = FrequencyRaw / PulsesPerRevolution * 60;
  RPM = RPM / 10000;
  total = total - readings[readIndex];
  readings[readIndex] = RPM;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;

}



void setup() {

  // Serial monitor initialization
  Serial.begin(115200);

  lcd.init();
  // Display initial message on LCD
  lcd.setBacklight(HIGH);
}


void WifiLcd(){
  lcd.clear();

  // Display WiFi status
  if (WiFi.status() == WL_CONNECTED) {
      lcd.setCursor(0, 0);
      lcd.print("Connected TO WIFI");
      Serial.print("Connected TO WIFI");
  } else {
      lcd.setCursor(0, 0);
      lcd.print("Not connected");
      Serial.print("Not connected");

  }
  delay(700);

}

void Rpm(){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("RPM : ");
  lcd.setCursor(0, 1);
  lcd.print(RPM);
  delay(700);
  Serial.print(RPM);
  
}

void Bgt(){
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Brightness : ");
  lcd.setCursor(0, 1);
  lcd.print(brightnessLevel);
  delay(700);
  Serial.print(brightnessLevel);

}

void TextLcd(){
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Input : ");
  lcd.setCursor(0, 1);
  lcd.print(message);
  delay(700);
  Serial.print(message);

}

void loop() {

  WifiLcd();

  Rpm();

  Bgt();

  if (message != "") 
  {
    TextLcd();
  }
}
