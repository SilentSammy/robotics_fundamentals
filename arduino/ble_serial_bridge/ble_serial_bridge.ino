// ble_serial_bridge - BLE remote control + serial telemetry
// BLE in:     2 signed bytes [x, w]  (-127 to 127)
// Serial in:  CSV  a,b[,direct]      (-1.0 to 1.0)
// Serial out: CSV  EL,ER,IR,US_cm    (20 Hz)
//             Non-data lines are prefixed with '#' for parser safety.
//
// NimBLE callbacks run on a separate FreeRTOS task, so they must not
// write to Serial directly. Instead the callback stores the command in
// a volatile struct; loop() reads it and owns all Serial output.
//
// Requires: NimBLE-Arduino library

#include <NimBLEDevice.h>

// Motor pins
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// PWM config
#define MOTOR_FREQ   100
#define MOTOR_RES    10
#define MOTOR_MAX    1023
#define TURN_SCALE   0.5f
#define TRIM         0.0f

// BLE UUIDs
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define DRIVE_UUID   "12345678-1234-5678-1234-56789abcdef1"

// Encoder pins
#define L_ENC_A 7
#define L_ENC_B 3
#define R_ENC_A 5
#define R_ENC_B 6

// Sensors
#define IR_SENSOR 9
#define US_TRIG   2
#define US_ECHO   1

// Telemetry
#define TELEM_MS 50   // 20 Hz

// ---- BLE command handoff (written by BLE task, read by loop) ----
struct BleCmd { float x, w; bool fresh; };
volatile BleCmd bleCmd = {0, 0, false};

// ---- Motor control ----
void setMotor(int in1, int in2, int speed) {
  if (speed > 0)      { ledcWrite(in1, speed);  ledcWrite(in2, 0);      }
  else if (speed < 0) { ledcWrite(in1, 0);       ledcWrite(in2, -speed); }
  else                { ledcWrite(in1, 0);        ledcWrite(in2, 0);      }
}

void stopMotors() {
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
}

void drive(float x, float w) {
  int vx = (int)(x * MOTOR_MAX);
  int vw = (int)(w * MOTOR_MAX * TURN_SCALE);
  setMotor(L_IN1, L_IN2, constrain((int)((vx - vw) * (1.0f - TRIM)), -MOTOR_MAX, MOTOR_MAX));
  setMotor(R_IN1, R_IN2, constrain((int)((vx + vw) * (1.0f + TRIM)), -MOTOR_MAX, MOTOR_MAX));
}

// ---- Encoders ----
volatile long encL = 0;
volatile long encR = 0;

void IRAM_ATTR onLeftA()  { encL += (digitalRead(L_ENC_B) == LOW) ? 1 : -1; }
void IRAM_ATTR onRightA() { encR += (digitalRead(R_ENC_B) == LOW) ? 1 : -1; }

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
  int direct = 0;
  if (sscanf(buf, "%f,%f,%d", &a, &b, &direct) < 2) return;
  a = constrain(a, -1.0f, 1.0f);
  b = constrain(b, -1.0f, 1.0f);
  if (direct) {
    setMotor(L_IN1, L_IN2, (int)(a * MOTOR_MAX));
    setMotor(R_IN1, R_IN2, (int)(b * MOTOR_MAX));
  } else {
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

// ---- BLE callbacks ----
class DriveCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) {
    std::string data = pChar->getValue();
    if (data.length() < 2) return;
    // Store for loop() to consume — do NOT call Serial here
    bleCmd.x = (int8_t)data[0] / 127.0f;
    bleCmd.w = (int8_t)data[1] / 127.0f;
    bleCmd.fresh = true;
  }
};

class ServerCallback : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    // Intentionally no Serial print — loop() owns serial
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    stopMotors();
    NimBLEDevice::startAdvertising();
  }
};

// ---- Setup ----
void setup() {
  Serial.begin(115200);

  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES); ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);
  stopMotors();

  pinMode(L_ENC_A, INPUT_PULLUP); pinMode(L_ENC_B, INPUT_PULLUP);
  pinMode(R_ENC_A, INPUT_PULLUP); pinMode(R_ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(L_ENC_A), onLeftA,  RISING);
  attachInterrupt(digitalPinToInterrupt(R_ENC_A), onRightA, RISING);

  pinMode(IR_SENSOR, INPUT);
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);

  NimBLEDevice::init("RobotCar");
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  NimBLECharacteristic* pDrive = pService->createCharacteristic(
    DRIVE_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  pDrive->setCallbacks(new DriveCallback());
  pService->start();
  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setName("RobotCar");
  NimBLEDevice::startAdvertising();

  Serial.println("# RobotCar ready");
  Serial.println("EL,ER,IR,US_cm");  // column labels for Serial Plotter
}

// ---- Loop ----
unsigned long lastTelemMs = 0;

void loop() {
  // Apply any pending BLE command
  if (bleCmd.fresh) {
    bleCmd.fresh = false;
    drive(bleCmd.x, bleCmd.w);
  }

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
