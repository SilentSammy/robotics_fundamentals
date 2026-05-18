// e11_serialtelem - Serial telemetry + drive control
//
// Serial out (20 Hz CSV): EL, ER, IR, US_cm
// Serial in  (CSV):       a, b [, direct]  — direct=0 (default): mixed (x,w)  direct=1: direct (left,right)
//
// The header line printed once in setup() enables labels in the
// Arduino IDE 2.x Serial Plotter. Numerical parsers can skip it.

// Motor pins
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// Encoder pins
#define L_ENC_A 7
#define L_ENC_B 3
#define R_ENC_A 5
#define R_ENC_B 6

// Sensors
#define IR_SENSOR 9
#define US_TRIG   2
#define US_ECHO   1

// Motor PWM config
#define MOTOR_FREQ  100
#define MOTOR_RES   10
#define MOTOR_MAX   1023
#define TURN_SCALE  0.5f

// Telemetry interval
#define TELEM_MS 50   // 20 Hz

// ---- Encoders ----
volatile long encL = 0;
volatile long encR = 0;

void IRAM_ATTR onLeftA()  { encL += (digitalRead(L_ENC_B) == LOW) ? 1 : -1; }
void IRAM_ATTR onRightA() { encR += (digitalRead(R_ENC_B) == LOW) ? 1 : -1; }

// ---- Motor control ----
void setMotor(int in1, int in2, int speed) {
  if (speed > 0)      { ledcWrite(in1, speed);  ledcWrite(in2, 0);      }
  else if (speed < 0) { ledcWrite(in1, 0);       ledcWrite(in2, -speed); }
  else                { ledcWrite(in1, 0);       ledcWrite(in2, 0);      }
}

void drive(float x, float w) {
  int vx = (int)(x * MOTOR_MAX);
  int vw = (int)(w * MOTOR_MAX * TURN_SCALE);
  setMotor(L_IN1, L_IN2, constrain(vx - vw, -MOTOR_MAX, MOTOR_MAX));
  setMotor(R_IN1, R_IN2, constrain(vx + vw, -MOTOR_MAX, MOTOR_MAX));
}

// ---- Ultrasonic ----
float readDistanceCm() {
  digitalWrite(US_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  long d = pulseIn(US_ECHO, HIGH, 30000);
  return d ? d * 0.034f / 2.0f : -1.0f;
}

// ---- Serial command parsing ----
char cmdBuf[32];
int  cmdLen = 0;

void parseCommand(const char* buf) {
  float a, b;
  int direct = 0;  // default: mixed mode
  if (sscanf(buf, "%f,%f,%d", &a, &b, &direct) < 2) return;
  a = constrain(a, -1.0f, 1.0f);
  b = constrain(b, -1.0f, 1.0f);
  if (direct) {
    // direct mode: a = left wheel, b = right wheel
    setMotor(L_IN1, L_IN2, (int)(a * MOTOR_MAX));
    setMotor(R_IN1, R_IN2, (int)(b * MOTOR_MAX));
  } else {
    // mixed mode: a = x (linear), b = w (angular)
    drive(a, b);
  }
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      cmdBuf[cmdLen] = '\0';
      parseCommand(cmdBuf);
      cmdLen = 0;
    } else if (c != '\r' && cmdLen < (int)sizeof(cmdBuf) - 1) {
      cmdBuf[cmdLen++] = c;
    }
  }
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);

  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);
  drive(0, 0);

  pinMode(L_ENC_A, INPUT_PULLUP); pinMode(L_ENC_B, INPUT_PULLUP);
  pinMode(R_ENC_A, INPUT_PULLUP); pinMode(R_ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(L_ENC_A), onLeftA,  RISING);
  attachInterrupt(digitalPinToInterrupt(R_ENC_A), onRightA, RISING);

  pinMode(IR_SENSOR, INPUT);
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);

  Serial.println("EL,ER,IR,US_cm");  // column labels for Serial Plotter
}

// ---- Loop ----
unsigned long lastTelemMs = 0;

void loop() {
  readSerial();

  unsigned long now = millis();
  if (now - lastTelemMs >= TELEM_MS) {
    lastTelemMs = now;

    float us = readDistanceCm();
    int   ir  = digitalRead(IR_SENSOR);

    noInterrupts();
    long el = encL;
    long er = encR;
    interrupts();

    Serial.print(el);    Serial.print(',');
    Serial.print(er);    Serial.print(',');
    Serial.print(ir);    Serial.print(',');
    Serial.println(us, 1);
  }
}
