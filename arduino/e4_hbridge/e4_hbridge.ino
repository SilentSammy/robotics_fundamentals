// e4_hbridge - Dual motor control demo
// Left:  IN1=20, IN2=21 | Right: IN1=0, IN2=10
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// Motor PWM config
#define MOTOR_FREQ 100   // Hz
#define MOTOR_RES  10    // 10-bit: duty range 0-1023
#define MOTOR_MAX  1023  // full speed

void setup() {
  Serial.begin(115200);
  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);

  // Stop
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
  delay(1000);
}

void loop() {
  // Forward
  Serial.println("Forward");
  ledcWrite(L_IN1, MOTOR_MAX); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, MOTOR_MAX); ledcWrite(R_IN2, 0);
  delay(1000);

  // Stop
  Serial.println("Stop");
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
  delay(500);

  // Backward
  Serial.println("Backward");
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, MOTOR_MAX);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, MOTOR_MAX);
  delay(1000);

  // Stop
  Serial.println("Stop");
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
  delay(500);

  // Spin left (left backward, right forward)
  Serial.println("Spin Left");
  ledcWrite(L_IN1, 0);        ledcWrite(L_IN2, MOTOR_MAX);
  ledcWrite(R_IN1, MOTOR_MAX); ledcWrite(R_IN2, 0);
  delay(600);

  // Stop
  Serial.println("Stop");
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
  delay(1000);
}
