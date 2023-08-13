#include <WiFi.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESPAsyncWebServer.h>
//#include <Adafruit_NeoPixel.h>

const char* ssid = "G3";
const char* password = "12345678910";

LiquidCrystal_I2C lcd(0x27, 16, 2);
// const int ledPin = 5;  // Pin connected to the LED strip
// const int numLEDs = 10;  // Number of LEDs in the strip

// Adafruit_NeoPixel strip(numLEDs, ledPin, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

   // Initialize LCD display
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 B");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // strip.begin();
  // strip.show();  // Initialize all LEDs to off

  server.on("/setbrightness", HTTP_POST, [](AsyncWebServerRequest* request){
    if (request->hasParam("brightness", true)) {
      int brightness = request->getParam("brightness", true)->value().toInt();
      Serial.print("Received brightness: ");
      Serial.println(brightness);

      // // Set LED strip brightness
      // strip.setBrightness(brightness);
      // strip.show();  // Update the strip with the new brightness
      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print(brightness);
    }

    request->send(200, "text/plain", "Brightness set");
  });

  server.begin();
}

void loop() {
  // Other code, if any
}
