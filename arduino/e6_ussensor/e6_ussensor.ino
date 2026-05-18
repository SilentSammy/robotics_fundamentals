// e6_ussensor - Ultrasonic sensor readouts
#define US_TRIG 2
#define US_ECHO 1

void setup() {
  Serial.begin(115200);
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);
  delay(1000);
  Serial.println("=== Ultrasonic Sensor Test ===");
}

void loop() {
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  long duration = pulseIn(US_ECHO, HIGH, 30000);
  if (duration > 0) {
    float distance = duration * 0.034 / 2;
    Serial.print(distance, 1);
    Serial.println(" cm");
  } else {
    Serial.println("Out of range");
  }

  delay(200);
}
