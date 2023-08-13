#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <ESP32Servo.h>

const char* ssid = "G3";
const char* password = "12345678910";
const char* esp32BIPAddress = "192.168.43.123";  // Replace with ESP32 B's IP address

String brightnessLevel = "128";

String previousBrightnessLevel = brightnessLevel;

int motorSpeed = 128;

int previousMotorSpeed  = motorSpeed;

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

// Define motor control pins
#define MOTOR_ENA 13
#define MOTOR_IN1 27
#define MOTOR_IN2 26

Servo motor;

AsyncWebServer server(80);

String message = "";  // Variable to store the user input message



void handleRootPage(AsyncWebServerRequest *request) {
  String html = "<html>";
  html+="<head>";
  html+="<title>POV GLOBE LED DISPLAY</title>";
  html+="<style>";
  html+="body {font-family: Verdana, Geneva, Tahoma, sans-serif;text-align: center;padding: 20px;background-image: linear-gradient(to right, rgba(255,0,0,0), rgb(48, 14, 17));margin-top: 100px;margin-bottom: 100px;}";
  html+="h1 {color: #060606;font-size: 50px;text-align: center;font-weight: 400;font-style: italic;text-shadow: 2px 2px 2px #000000;}";
  html+="form {margin-bottom: 20px;}";
  html+="label {font-weight: bold;}";
  html+="input[type='text'] {width: 300px;padding: 5px;margin-bottom: 10px;}";
  html+=".button {display: inline-block;border-radius: 4px;background-color: #251216;border: none;color: #FFFFFF;text-align: center;font-size: 18px;padding: 10px;width: 100px;transition: all 0.5s;cursor: pointer;margin: 5px;}";
  html+=".button span {cursor: pointer;display: inline-block;position: relative;transition: 0.5s;}";
  html+=".button span:after {content: '\00bb';position: absolute;opacity: 0;top: 0;right: -20px;transition: 0.5s;}";
  html+=".button:hover span {padding-right: 25px;}";
  html+=".button:hover span:after {opacity: 1;right: 0;}";
  html+="input[type='range'] {width: 300px;}";
  html+="input[type='range']::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 20px;height: 20px;background-color: #131516;border-radius: 50%;cursor: pointer;}";
  html+="input[type='range']::-webkit-slider-thumb:hover {background-color: #070808;}";
  html+="input[type='range']::-moz-range-thumb {width: 20px;height: 20px;background-color: #1d0d15;border-radius: 50%;cursor: pointer;}";
  html+="input[type='range']::-moz-range-thumb:hover {background-color: #150912;}";
  html+="input[type='range']::-webkit-slider-runnable-track {width: 100%height: 5px;background-color: #0a0909;}";
  html+="input[type='range']:focus {outline: none;}";
  html+="input[type='range']:focus::-webkit-slider-runnable-track {background-color: #181414;}";
  html+="</style>";
  html+="</head>";
  html+="<body>";
  html+="<h1>POV GLOBE LED DISPLAY</h1>";
  html+="<form method='POST' action='/sendmessage'>";
  html+="<label for='message'>Enter a message:</label><br>";
  html+="<input type='text' id='message' name='message'><br><br>";
  html+="<button class='button' style='vertical-align:middle' type='submit' value='Submit'><span>Submit </span></button>";
  html+="</form>";
  html+="<br><br>";
  html+="<label for='brightness'>Adjust LED brightness:</label><br>";
  html+="<input type='range' id='brightness' name='brightness' min='0' max='255' value='" + String(brightnessLevel) + "'><br><br>";
  html+="<label for='speed'>Adjust motor speed:</label><br>";
  html+="<input type='range' id='speed' name='speed' min='0' max='255' value='" + String(motorSpeed) + "'><br><br>";
  html+="<script>";
  html+="document.getElementById('brightness').addEventListener('input', function() {";
  html+="var xhttp = new XMLHttpRequest();";
  html+="xhttp.onreadystatechange = function() {";
  html+="if (this.readyState == 4 && this.status == 200) {";
  html+="console.log('Brightness updated');";
  html+="}";
  html+="};";
  html+="xhttp.open('POST', '/sendbrightness', true);";
  html+="xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  html+="xhttp.send('brightness=' + this.value);";
  html+="});";
  html+="document.getElementById('speed').addEventListener('input', function() {";
  html+="var xhttp = new XMLHttpRequest();";
  html+="xhttp.onreadystatechange = function() {";
  html+="if (this.readyState == 4 && this.status == 200) {";
  html+="console.log('Speed updated');";
  html+="}";
  html+="};";
  html+="xhttp.open('POST', '/speed', true);";
  html+="xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  html+=" xhttp.send('speed=' + this.value);";
  html+=" });";
  html+=" </script>";
  html+="</body>";
  html+="</html>";

  request->send(200, "text/html", html);
}

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


/////user message to esp32 b////***

// void handleFormSubmit(AsyncWebServerRequest *request) {
//   if (request->hasParam("message", true)) {
//     userInput = request->getParam("message", true)->value();
//     Serial.println("Received message: " + userInput);
//   }

//   request->redirect("/");
// }


void handleFormSubmit(AsyncWebServerRequest* request) {
  if (request->hasParam("message", true)) {
    message = request->getParam("message", true)->value();
    Serial.print("Received message: ");
    Serial.println(message);

    // Pass the message to ESP32 B
    sendMessageToESP32B(message);
  }

  request->redirect("/");
}

void sendMessageToESP32B(String message) {
  WiFiClient client;

  if (client.connect(esp32BIPAddress, 80)) {
    Serial.println("Sending message to ESP32 B...");

    // Create the HTTP POST request
    String url = "/displaymessage";
    String data = "message=" + message;

    // Send the request
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + esp32BIPAddress + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Content-Length: " + data.length() + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 data);

    Serial.println("Message sent to ESP32 B");
  } else {
    Serial.println("Failed to connect to ESP32 B");
  }
}

/////led brigthness controlling to esp32 b////***

// void handleBrightnessUpdate(AsyncWebServerRequest *request) {
//   if (request->hasParam("brightness", true)) {
//     brightnessLevel = request->getParam("brightness", true)->value().toInt();
//     FastLED.setBrightness(brightnessLevel);
//     Serial.println("Brightness level updated: " + String(brightnessLevel));
//   }

//   request->redirect("/");
// }

// void handleBrightnessUpdate(AsyncWebServerRequest* request) {
//   if (request->hasParam("brightness", true)) {
//     brightnessLevel = request->getParam("brightness", true)->value().toInt();
//     Serial.println("Brightness level updated: " + String(brightnessLevel));

//     // Pass the message to ESP32 B
//     sendBrightnessToESP32B(brightnessLevel);
//   }

//   request->redirect("/");
// }

// void sendBrightnessToESP32B(String brightnessLevel) {
//   WiFiClient client;

//   if (client.connect(esp32BIPAddress, 80)) {
//     Serial.println("Sending message to ESP32 B...");

//     // Create the HTTP POST request
//     String url = "/displaybrightnessLevel";
//     String data = "brightnessLevel=" + brightnessLevel;

//     // Send the request
//     client.print(String("POST ") + url + " HTTP/1.1\r\n" +
//                  "Host: " + esp32BIPAddress + "\r\n" +
//                  "Content-Type: application/x-www-form-urlencoded\r\n" +
//                  "Content-Length: " + data.length() + "\r\n" +
//                  "Connection: close\r\n\r\n" +
//                  data);

//     Serial.println("Message sent to ESP32 B");
//   } else {
//     Serial.println("Failed to connect to ESP32 B");
//   }
// }


////motor speed control in esp32 a////***

void handleSpeedUpdate(AsyncWebServerRequest *request) {
  if (request->hasParam("speed", true)) {
    motorSpeed = request->getParam("speed", true)->value().toInt();
    int motorSpeedMapped = map(motorSpeed, 0, 100, 0, 255);  // Map the motor speed to a range suitable for PWM
    analogWrite(MOTOR_ENA, motorSpeedMapped);  // Set the motor speed using PWM
    if (motorSpeed > 0) {
      digitalWrite(MOTOR_IN1, HIGH);  // Set motor direction forward
      digitalWrite(MOTOR_IN2, LOW);
    } else {
      digitalWrite(MOTOR_IN1, LOW);  // Set motor direction reverse
      digitalWrite(MOTOR_IN2, HIGH);
    }
    //motor.setSpeed(motorSpeed);
    Serial.println("Motor speed updated: " + String(motorSpeed));
  }

  request->redirect("/");
}

/////////////////////////////////end of logic////////////////********

void setup() {
  // Serial monitor initialization
  Serial.begin(115200);

   // Initialize LCD display
  lcd.init();
  // Display initial message on LCD
  lcd.setBacklight(HIGH);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  //ipAddress = WiFi.localIP();

  // Set up web server
  server.on("/", HTTP_GET, handleRootPage);
  server.on("/sendmessage", HTTP_POST, handleFormSubmit);
  //server.on("/sendbrightness", HTTP_POST, handleBrightnessUpdate);
  server.on("/speed", HTTP_POST, handleSpeedUpdate);
  server.begin();

  // Initialize motor control pins
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(4), Pulse_Event, RISING);
  delay(1000);

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

}

void Bgt(){
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Brightness : ");
  lcd.setCursor(0, 1);
  lcd.print(brightnessLevel);
  delay(700);

}

void TextLcd(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Input : ");
  lcd.setCursor(0, 1);
  lcd.print(message);
  delay(700);

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

