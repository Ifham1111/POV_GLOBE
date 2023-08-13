#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "G3";
const char* password = "12345678910";
LiquidCrystal_I2C lcd(0x27, 16, 2);

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 B");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    request->send(200, "text/plain", "Welcome to ESP32 B");
  });

  server.on("/displaymessage", HTTP_POST, [](AsyncWebServerRequest* request){
    if (request->hasParam("message", true)) {
      String message = request->getParam("message", true)->value();
      Serial.print("Received message: ");
      Serial.println(message);

      lcd.clear();
      //lcd.print("Message:");
      lcd.setCursor(0, 1);
      lcd.print(message);
    }

    request->send(200, "text/plain", "Message displayed");
  });

  server.begin();
}

void loop() {
  // Other code, if any
}
