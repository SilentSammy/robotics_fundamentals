// e5_irsensor - IR sensor readouts
#define IR_SENSOR 9

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR, INPUT);
  delay(1000);
  Serial.println("=== IR Sensor Test ===");
}

void loop() {
  int value = digitalRead(IR_SENSOR);
  Serial.println(value == LOW ? "Obstacle detected" : "Clear");
  delay(200);
}
