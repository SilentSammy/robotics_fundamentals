// Buzzer Test for ESP32-C3

#define BUZZER 3

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(BUZZER, OUTPUT);
  
  Serial.println("=== Buzzer Test ===");
  Serial.println("Playing tones...\n");
  
  // Test 1: Single beep
  Serial.println("1. Single beep (1kHz, 200ms)");
  tone(BUZZER, 1000, 200);
  delay(500);
  
  // Test 2: Two short beeps
  Serial.println("2. Two short beeps");
  tone(BUZZER, 1500, 100);
  delay(200);
  tone(BUZZER, 1500, 100);
  delay(500);
  
  // Test 3: Ascending tones
  Serial.println("3. Ascending tones");
  for (int freq = 500; freq <= 2000; freq += 500) {
    tone(BUZZER, freq, 150);
    delay(200);
  }
  delay(500);
  
  // Test 4: Simple melody (startup sound)
  Serial.println("4. Startup melody");
  tone(BUZZER, 1000, 100);
  delay(150);
  tone(BUZZER, 1500, 100);
  delay(150);
  tone(BUZZER, 2000, 200);
  delay(500);
  
  Serial.println("\nBuzzer test complete!");
}

void loop() {
  // Empty
}
