// e1_led - 1Hz blink
#define LED 8  // Built-in LED on ESP32-C3 Super Mini

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
