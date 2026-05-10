// Serial Communication Test for ESP32-C3 (Heartbeat)

unsigned long lastBeat = 0;
int beatCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32-C3 Heartbeat ===");
}

void loop() {
  if (millis() - lastBeat >= 1000) {
    lastBeat = millis();
    beatCount++;
    Serial.print("Heartbeat #");
    Serial.print(beatCount);
    Serial.print(" | Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println("s");
  }
}
