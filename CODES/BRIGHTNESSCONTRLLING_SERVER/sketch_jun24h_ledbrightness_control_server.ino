#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

const char* ssid = "G3";
const char* password = "12345678910";
const char* esp32BIPAddress = "192.168.43.225";  // Replace with ESP32 B's IP address

AsyncWebServer server(80);

int brightness = 0;  // Variable to store the LED brightness

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    String html = "<form method='post' action='/sendmessage'>"
                  "<input type='range' min='0' max='255' name='brightness'>"
                  "<input type='submit' value='Set Brightness'>"
                  "</form>";
    request->send(200, "text/html", html);
  });

  server.on("/sendmessage", HTTP_POST, [](AsyncWebServerRequest* request){
    if (request->hasParam("brightness", true)) {
      brightness = request->getParam("brightness", true)->value().toInt();
      Serial.print("Received brightness: ");
      Serial.println(brightness);

      // Pass the brightness to ESP32 B
      sendToESP32B(brightness);
    }

    request->send(200, "text/plain", "Brightness received");
  });

  server.begin();
}

void loop() {
  // Other code, if any
}

void sendToESP32B(int brightness) {
  // Code to send brightness value to ESP32 B
  // You can use HTTP POST or any other communication method of your choice

  // Example using HTTP POST request
  HTTPClient http;
  http.begin("http://" + String(esp32BIPAddress) + "/setbrightness");  // Replace with the appropriate URL of ESP32 B
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "brightness=" + String(brightness);
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode == 200) {
    Serial.println("Brightness sent to ESP32 B");
  } else {
    Serial.print("Failed to send brightness. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
