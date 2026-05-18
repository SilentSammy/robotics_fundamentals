// ble_server - BLE remote control for differential drive car
// Receives 2 signed bytes over BLE: [left_speed, right_speed]
// Range: -127 (full reverse) to 127 (full forward)
//
// Requires: NimBLE-Arduino library

#include <NimBLEDevice.h>

// Motor pins
#define L_IN1 20
#define L_IN2 21
#define R_IN1 0
#define R_IN2 10

// PWM config (same API as e2_dim)
#define MOTOR_FREQ   100    // Hz - low freq for higher torque
#define MOTOR_RES    10     // 10-bit: duty range 0-1023 (8-bit too coarse below ~305Hz)
#define MOTOR_MAX    1023   // 2^MOTOR_RES - 1
#define TURN_SCALE   0.5   // reduce angular authority relative to linear
#define TRIM         0.0   // motor trim: + boosts right / – boosts left (range ±0.2 typical)

// BLE UUIDs
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define DRIVE_UUID   "12345678-1234-5678-1234-56789abcdef1"

// ---- Motor control ----
void setMotor(int in1, int in2, int speed) {
  // speed: -255 to 255
  if (speed > 0) {
    ledcWrite(in1, speed);
    ledcWrite(in2, 0);
  } else if (speed < 0) {
    ledcWrite(in1, 0);
    ledcWrite(in2, -speed);
  } else {
    ledcWrite(in1, 0);
    ledcWrite(in2, 0);
  }
}

void stopMotors() {
  ledcWrite(L_IN1, 0); ledcWrite(L_IN2, 0);
  ledcWrite(R_IN1, 0); ledcWrite(R_IN2, 0);
}

// ---- BLE callbacks ----
class DriveCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) {
    std::string data = pChar->getValue();
    if (data.length() < 2) return;

    int8_t x = (int8_t)data[0];  // linear  (-127=back, 127=forward)
    int8_t w = (int8_t)data[1];  // angular (-127=left, 127=right)

    // Map to -MOTOR_MAX..MOTOR_MAX, then mix into left/right
    int vx = map(x, -127, 127, -MOTOR_MAX, MOTOR_MAX);
    int vw = map(w, -127, 127, -MOTOR_MAX, MOTOR_MAX) * TURN_SCALE;
    int left  = constrain((int)((vx - vw) * (1.0 - TRIM)), -MOTOR_MAX, MOTOR_MAX);
    int right = constrain((int)((vx + vw) * (1.0 + TRIM)), -MOTOR_MAX, MOTOR_MAX);

    setMotor(L_IN1, L_IN2, left);
    setMotor(R_IN1, R_IN2, right);

    Serial.print("x: "); Serial.print(x);
    Serial.print(" | w: "); Serial.print(w);
    Serial.print(" -> L: "); Serial.print(left);
    Serial.print(" R: "); Serial.println(right);
  }
};

class ServerCallback : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    Serial.print("Client connected: ");
    Serial.println(connInfo.getAddress().toString().c_str());
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    Serial.print("Client disconnected (reason: ");
    Serial.print(reason);
    Serial.println(") - re-advertising");
    stopMotors();
    NimBLEDevice::startAdvertising();
  }
};

// ---- Setup ----
void setup() {
  Serial.begin(115200);

  // Attach all motor pins to LEDC (same as ledcAttach in e2_dim)
  ledcAttach(L_IN1, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(L_IN2, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN1, MOTOR_FREQ, MOTOR_RES);
  ledcAttach(R_IN2, MOTOR_FREQ, MOTOR_RES);
  stopMotors();

  NimBLEDevice::init("RobotCar");

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  NimBLECharacteristic* pDrive = pService->createCharacteristic(
    DRIVE_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  pDrive->setCallbacks(new DriveCallback());

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setName("RobotCar");
  NimBLEDevice::startAdvertising();

  Serial.println("BLE server ready - advertising as 'RobotCar'");
}

void loop() {
  // Everything handled in BLE callbacks
}
