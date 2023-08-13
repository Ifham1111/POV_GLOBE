#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

const char* ssid = "G3";
const char* password = "12345678910";
const char* esp32BIPAddress = "192.168.43.225";  // Replace with ESP32 B's IP address

AsyncWebServer server(80);

String message = "";  // Variable to store the user input message

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
                  "<input type='text' name='message'>"
                  "<input type='submit' value='Submit'>"
                  "</form>";
    request->send(200, "text/html", html);
  });

  server.on("/sendmessage", HTTP_POST, [](AsyncWebServerRequest* request){
    if (request->hasParam("message", true)) {
      message = request->getParam("message", true)->value();
      Serial.print("Received message: ");
      Serial.println(message);

      // Pass the message to ESP32 B
      sendToESP32B(message);
    }

    request->send(200, "text/plain", "Message received");
  });

  server.begin();
}

void loop() {
  // Other code, if any
}

void sendToESP32B(String message) {
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
