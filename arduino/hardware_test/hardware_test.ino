// Hardware Test for ESP32-C3 Differential Drive Robot
// Tests all components once on startup

// Pin Definitions
// Motors
#define MOTOR_L_IN1 20
#define MOTOR_L_IN2 21
#define MOTOR_R_IN1 0
#define MOTOR_R_IN2 10

// Motor PWM config
#define MOTOR_FREQ 100   // Hz
#define MOTOR_RES  10    // 10-bit: duty range 0-1023
#define MOTOR_MAX  1023  // full speed

// Encoders
#define ENC_L_CHA 7
#define ENC_L_CHB 3
#define ENC_R_CHA 5
#define ENC_R_CHB 6

// Sensors & Actuators
#define BUZZER 4
#define IR_SENSOR 9
#define US_TRIG 2
#define US_ECHO 1

// Global encoder counters (signed: positive = forward, negative = backward)
volatile long encoderL_count = 0;
volatile long encoderR_count = 0;

// Quadrature decoding: on Channel A rising edge, Channel B state gives direction
// B = LOW  → forward  (+1)
// B = HIGH → backward (-1)
void IRAM_ATTR encoderL_ISR() {
  encoderL_count += digitalRead(ENC_L_CHB) ? -1 : 1;
}

void IRAM_ATTR encoderR_ISR() {
  encoderR_count += digitalRead(ENC_R_CHB) ? -1 : 1;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Hardware Test Started ===\n");
  
  // Configure motors via LEDC
  ledcAttach(MOTOR_L_IN1, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(MOTOR_L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(MOTOR_R_IN1, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(MOTOR_R_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcWrite(MOTOR_L_IN1, 0); ledcWrite(MOTOR_L_IN2, 0);
  ledcWrite(MOTOR_R_IN1, 0); ledcWrite(MOTOR_R_IN2, 0);

  // Configure encoders, sensors, buzzer
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
  ledcWrite(MOTOR_L_IN1, MOTOR_MAX);
  ledcWrite(MOTOR_L_IN2, 0);
  delay(500);
  ledcWrite(MOTOR_L_IN1, 0);
  Serial.println("OK (forward 500ms)");
  
  // Test 3: Right Motor
  Serial.print("3. Right Motor Test... ");
  ledcWrite(MOTOR_R_IN1, MOTOR_MAX);
  ledcWrite(MOTOR_R_IN2, 0);
  delay(500);
  ledcWrite(MOTOR_R_IN1, 0);
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
