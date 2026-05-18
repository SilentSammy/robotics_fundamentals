// e8_linefollower - Single IR sensor edge follower
// Follows the LEFT edge of a dark line on a light surface.
//
// Strategy: keep the sensor on the boundary so it toggles.
//   Line seen  (LOW)  → steer left  (line is under sensor, drift left to find edge)
//   Line absent (HIGH) → steer right (lost the edge, curve back toward line)
//
// Motors: Left IN1=20, IN2=21 | Right IN1=0, IN2=10
// IR:     GPIO9 (LOW = line detected)

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
#define BASE_SPEED_PCT  0.30f   // outer-wheel speed during a turn
#define TURN_SPEED_PCT  0.15f   // inner-wheel speed during a turn
#define STALL_TIMEOUT_MS 1000   // stop if sensor state unchanged for this long (ms)

#define BASE_SPEED  ((int)(BASE_SPEED_PCT * MOTOR_MAX))
#define TURN_SPEED  ((int)(TURN_SPEED_PCT * MOTOR_MAX))

// Stall detection state
int lastLine = -1;              // -1 forces a "change" on the first read
unsigned long lastChangeMs = 0;

// IR sensor
#define IR_SENSOR 9       // LOW = line detected

void setMotor(int in1, int in2, int speed) {
  if (speed > 0) {
    ledcWrite(in1, speed); ledcWrite(in2, 0);
  } else if (speed < 0) {
    ledcWrite(in1, 0);     ledcWrite(in2, -speed);
  } else {
    ledcWrite(in1, 0);     ledcWrite(in2, 0);
  }
}

void setup() {
  Serial.begin(115200);

  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);
  setMotor(L_IN1, L_IN2, 0);
  setMotor(R_IN1, R_IN2, 0);

  pinMode(IR_SENSOR, INPUT);

  delay(2000);  // time to place the robot before it starts moving
  lastChangeMs = millis();      // start the stall timer from here
  Serial.println("=== Line Follower Started ===");
}

void loop() {
  int line = digitalRead(IR_SENSOR);  // LOW = line seen
  unsigned long now = millis();

  // Update stall timer on any state change
  if (line != lastLine) {
    lastLine = line;
    lastChangeMs = now;
  }

  // Stop if the sensor has been stuck in the same state too long
  if (now - lastChangeMs >= STALL_TIMEOUT_MS) {
    setMotor(L_IN1, L_IN2, 0);
    setMotor(R_IN1, R_IN2, 0);
    Serial.println("Stalled -> stopped");
    return;
  }

  if (line == LOW) {
    // On the line - steer left
    setMotor(L_IN1, L_IN2, TURN_SPEED);
    setMotor(R_IN1, R_IN2, BASE_SPEED);
    Serial.println("Line  -> steer LEFT");
  } else {
    // Off the line - steer right
    setMotor(L_IN1, L_IN2, BASE_SPEED);
    setMotor(R_IN1, R_IN2, TURN_SPEED);
    Serial.println("Clear -> steer RIGHT");
  }
}
