// Simple toggle

// Pin connected to resistor/transistor
#define relayPin 13

void setup() {
  // Set mode of GPIO pin
  pinMode(relayPin, OUTPUT);
}

void loop() {
  // Toggle switch constantly with a 1000ms interval
  digitalWrite(relayPin, HIGH);
  delay(1000);
  digitalWrite(relayPin, LOW);
  delay(1000);
}
