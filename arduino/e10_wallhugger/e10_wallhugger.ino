// e10_wallhugger - Proportional wall-following with ultrasonic sensor
// Keeps a wall at TARGET_DIST cm to the RIGHT using a P controller.
//
//   error = measured_dist - TARGET_DIST
//   turn  = Kp * error   (clamped to ±MAX_TURN)
//
//   error > 0 → wall too far  → turn right (left wheel faster)
//   error < 0 → wall too close → turn left  (right wheel faster)
//
// Motors: Left IN1=20, IN2=21 | Right IN1=0, IN2=10
// US:     TRIG=2, ECHO=1 (sensor faces RIGHT)

// Motor pins
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// Motor PWM config
#define MOTOR_FREQ 100   // Hz
#define MOTOR_RES  10    // 10-bit: duty range 0-1023
#define MOTOR_MAX  1023

// Tuning
#define TARGET_DIST     25.0f  // desired wall distance (cm)
#define BASE_SPEED_PCT   0.25f  // forward speed (fraction of MOTOR_MAX)
#define MAX_TURN_PCT     0.18f  // max turn authority (fraction of MOTOR_MAX)
#define KP               0.012f // proportional gain (fraction per cm)
                                // at 25 cm error → full turn authority

#define BASE_SPEED  ((int)(BASE_SPEED_PCT * MOTOR_MAX))
#define MAX_TURN    ((int)(MAX_TURN_PCT   * MOTOR_MAX))

// Ultrasonic sensor
#define US_TRIG 2
#define US_ECHO 1
#define OUT_OF_RANGE_CM 200.0f  // assumed distance when sensor times out

void setMotor(int in1, int in2, int speed) {
  if (speed > 0) {
    ledcWrite(in1, speed); ledcWrite(in2, 0);
  } else if (speed < 0) {
    ledcWrite(in1, 0);     ledcWrite(in2, -speed);
  } else {
    ledcWrite(in1, 0);     ledcWrite(in2, 0);
  }
}

float readDistanceCm() {
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  long duration = pulseIn(US_ECHO, HIGH, 30000);
  if (duration == 0) return OUT_OF_RANGE_CM;
  return duration * 0.034f / 2.0f;
}

void setup() {
  Serial.begin(115200);

  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);
  setMotor(L_IN1, L_IN2, 0);
  setMotor(R_IN1, R_IN2, 0);

  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);

  delay(2000);
  Serial.println("=== Wall Hugger Started ===");
}

void loop() {
  float dist  = readDistanceCm();
  float error = dist - TARGET_DIST;

  // Positive turn → steer right (left wheel faster, right wheel slower)
  int turn = constrain((int)(KP * error * MOTOR_MAX), -MAX_TURN, MAX_TURN);

  int leftSpeed  = constrain(BASE_SPEED + turn, 0, MOTOR_MAX);
  int rightSpeed = constrain(BASE_SPEED - turn, 0, MOTOR_MAX);

  setMotor(L_IN1, L_IN2, leftSpeed);
  setMotor(R_IN1, R_IN2, rightSpeed);

  Serial.print("dist: "); Serial.print(dist, 1);
  Serial.print(" cm | err: "); Serial.print(error, 1);
  Serial.print(" | turn: "); Serial.print(turn);
  Serial.print(" | L: "); Serial.print(leftSpeed);
  Serial.print(" R: "); Serial.println(rightSpeed);

  delay(50);  // ~20 Hz
}
