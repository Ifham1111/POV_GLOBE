int messageLength = 0;
int currentPosition = 0;
int hallEffectPin = 14;  // Pin connected to the hall effect sensor

void setup() {

  pinMode(hallEffectPin, INPUT_PULLUP);  // Use INPUT_PULLUP to enable the internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(hallEffectPin), hallEffectISR, FALLING);  // Attach the interrupt

}

volatile bool sensorTriggered = false;  // Flag to indicate the hall effect sensor detection

void IRAM_ATTR hallEffectISR() {
  sensorTriggered = true;  // Set the flag to true when the hall effect sensor is triggered
}


void loop() {
if (message != "") 
  {

    if (sensorTriggered) 
    { 
      // Check if the hall effect sensor is triggered
      sensorTriggered = false;  // Reset the flag
      
      if (currentPosition >= messageLength) 
      {
      
        currentPosition = 0;  // Reset to the beginning of the message
        delay(800);  // Delay for 2 seconds before displaying the message again
  
      } 
      else if (currentPosition >= 0 && currentPosition < messageLength) 
      {
  
        char currentChar = message[currentPosition];
        textPrint(currentChar);  // Generate LED pattern for the current character
        delay(800);
        currentPosition++;
        //delay(100);  // Adjust the delay as needed to control the rotation speed
  
      } 
      else 
      {
  
        // Clear LEDs when the sensor is not active
        for (int i = 0; i < NUM_PIXELS; i++) 
        {
          leds[i] = CRGB::Black;  // Set all LEDs to off
        }
        FastLED.show();
  
      }
  
    }
  
  }
}
