// Define motor control pins
#define MOTOR_ENA 13
#define MOTOR_IN1 27
#define MOTOR_IN2 26

int motorSpeed = 128;

void handleRootPage(AsyncWebServerRequest *request) {
  String html = "<html>";
  html+="<head>";
  html+="<title>POV GLOBE LED DISPLAY</title>";
  html+="<style>";
  html+="body {font-family: Verdana, Geneva, Tahoma, sans-serif;text-align: center;padding: 20px;background-image: linear-gradient(to right, rgba(255,0,0,0), rgb(48, 14, 17));margin-top: 100px;margin-bottom: 100px;}";
  html+="h1 {color: #060606;font-size: 50px;text-align: center;font-weight: 400;font-style: italic;text-shadow: 2px 2px 2px #000000;}";
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
  html+="<label for='speed'>Adjust motor speed:</label><br>";
  html+="<input type='range' id='speed' name='speed' min='0' max='255' value='" + String(motorSpeed) + "'><br><br>";
  html+="<script>";
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

void setup() {

  server.on("/speed", HTTP_POST, handleSpeedUpdate);

  // Initialize motor control pins
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}
