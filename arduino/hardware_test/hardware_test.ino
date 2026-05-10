// Hardware Test for ESP32-C3 Differential Drive Robot
// Tests all components once on startup

// Pin Definitions
// Motors
#define MOTOR_L_IN1 7
#define MOTOR_L_IN2 8
#define MOTOR_R_IN1 5
#define MOTOR_R_IN2 6

// Encoders
#define ENC_L_CHA 20
#define ENC_L_CHB 21
#define ENC_R_CHA 9
#define ENC_R_CHB 10

// Sensors & Actuators
#define BUZZER 3
#define IR_SENSOR 0
#define US_TRIG 2
#define US_ECHO 1

// Global encoder counters
volatile long encoderL_count = 0;
volatile long encoderR_count = 0;

void IRAM_ATTR encoderL_ISR() {
  encoderL_count++;
}

void IRAM_ATTR encoderR_ISR() {
  encoderR_count++;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Hardware Test Started ===\n");
  
  // Configure pins
  pinMode(MOTOR_L_IN1, OUTPUT);
  pinMode(MOTOR_L_IN2, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT);
  pinMode(MOTOR_R_IN2, OUTPUT);
  
  pinMode(ENC_L_CHA, INPUT_PULLUP);
  pinMode(ENC_L_CHB, INPUT_PULLUP);
  pinMode(ENC_R_CHA, INPUT_PULLUP);
  pinMode(ENC_R_CHB, INPUT_PULLUP);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_SENSOR, INPUT);
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);
  
  // Attach encoder interrupts
  attachInterrupt(digitalPinToInterrupt(ENC_L_CHA), encoderL_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_CHA), encoderR_ISR, RISING);
  
  // Test 1: Buzzer
  Serial.print("1. Buzzer Test... ");
  tone(BUZZER, 1000, 200);
  delay(300);
  Serial.println("OK (should hear beep)");
  
  // Test 2: Left Motor
  Serial.print("2. Left Motor Test... ");
  digitalWrite(MOTOR_L_IN1, HIGH);
  digitalWrite(MOTOR_L_IN2, LOW);
  delay(500);
  digitalWrite(MOTOR_L_IN1, LOW);
  Serial.println("OK (forward 500ms)");
  
  // Test 3: Right Motor
  Serial.print("3. Right Motor Test... ");
  digitalWrite(MOTOR_R_IN1, HIGH);
  digitalWrite(MOTOR_R_IN2, LOW);
  delay(500);
  digitalWrite(MOTOR_R_IN1, LOW);
  Serial.println("OK (forward 500ms)");
  
  // Final beep
  delay(500);
  tone(BUZZER, 2000, 100);
  delay(150);
  tone(BUZZER, 2500, 100);
  
  Serial.println("\n=== Motor Test Complete ===");
  Serial.println("=== Starting Continuous Sensor Monitoring ===\n");
}

void loop() {
  // Encoders
  Serial.print("Enc L: ");
  Serial.print(encoderL_count);
  Serial.print(" | Enc R: ");
  Serial.print(encoderR_count);
  
  // IR Sensor
  int irValue = digitalRead(IR_SENSOR);
  Serial.print(" | IR: ");
  Serial.print(irValue == HIGH ? "clear" : "detect");
  
  // Ultrasonic Sensor
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  
  long duration = pulseIn(US_ECHO, HIGH, 30000);
  Serial.print(" | US: ");
  if (duration > 0) {
    float distance = duration * 0.034 / 2;
    Serial.print(distance, 1);
    Serial.print(" cm");
  } else {
    Serial.print("---");
  }
  
  Serial.println();
  delay(1000);
}
