// e9_obstacle - Forward driving with ultrasonic obstacle stop
// Drives forward at a fixed speed and stops when an obstacle is
// detected closer than STOP_DIST_CM. Resumes when the path clears.
//
// Motors: Left IN1=20, IN2=21 | Right IN1=0, IN2=10
// US:     TRIG=2, ECHO=1

// Motor pins
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// Motor PWM config
#define MOTOR_FREQ 100   // Hz
#define MOTOR_RES  10    // 10-bit: duty range 0-1023
#define MOTOR_MAX  1023

// Tuning (0.0 – 1.0 as a fraction of MOTOR_MAX)
#define DRIVE_SPEED_PCT  0.30f   // forward speed
#define DRIVE_SPEED      ((int)(DRIVE_SPEED_PCT * MOTOR_MAX))

// Ultrasonic sensor
#define US_TRIG      2
#define US_ECHO      1
#define STOP_DIST_CM 20.0f   // stop threshold (cm)

void setMotor(int in1, int in2, int speed) {
  if (speed > 0) {
    ledcWrite(in1, speed); ledcWrite(in2, 0);
  } else if (speed < 0) {
    ledcWrite(in1, 0);     ledcWrite(in2, -speed);
  } else {
    ledcWrite(in1, 0);     ledcWrite(in2, 0);
  }
}

// Returns distance in cm, or -1 if out of range
float readDistanceCm() {
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  long duration = pulseIn(US_ECHO, HIGH, 30000);  // 30ms timeout ≈ 510cm max
  if (duration == 0) return -1;
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
  Serial.println("=== Obstacle Avoidance Started ===");
}

void loop() {
  float dist = readDistanceCm();

  if (dist > 0 && dist < STOP_DIST_CM) {
    setMotor(L_IN1, L_IN2, 0);
    setMotor(R_IN1, R_IN2, 0);
    Serial.print("Obstacle at ");
    Serial.print(dist, 1);
    Serial.println(" cm -> stopped");
  } else {
    setMotor(L_IN1, L_IN2, DRIVE_SPEED);
    setMotor(R_IN1, R_IN2, DRIVE_SPEED);
    if (dist < 0) {
      Serial.println("Out of range -> driving");
    } else {
      Serial.print(dist, 1);
      Serial.println(" cm -> driving");
    }
  }

  delay(50);  // ~20Hz sensing rate
}
