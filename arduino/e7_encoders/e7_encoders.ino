// e7_encoders - Quadrature encoder readouts
// Left:  A=7, B=3 | Right: A=5, B=6
//
// Each motor has a quadrature encoder with two channels (A and B) offset by
// 90 degrees. By comparing which channel changes first we can determine both
// the speed and the direction of rotation.
//
//   A:  ___|‾‾‾|___|‾‾‾|___
//   B:  _|‾‾‾|___|‾‾‾|_____
//        →  forward (B lags A)
//
// We attach an interrupt to the A channel of each encoder. On every rising
// edge of A we sample B: if B is LOW the motor is spinning forward; if B is
// HIGH it is spinning in reverse.

#define L_ENC_A 7
#define L_ENC_B 3
#define R_ENC_A 5
#define R_ENC_B 6

volatile long leftCount  = 0;
volatile long rightCount = 0;

void IRAM_ATTR onLeftA() {
  leftCount += (digitalRead(L_ENC_B) == LOW) ? 1 : -1;
}

void IRAM_ATTR onRightA() {
  rightCount += (digitalRead(R_ENC_B) == LOW) ? 1 : -1;
}

void setup() {
  Serial.begin(115200);

  pinMode(L_ENC_A, INPUT_PULLUP); pinMode(L_ENC_B, INPUT_PULLUP);
  pinMode(R_ENC_A, INPUT_PULLUP); pinMode(R_ENC_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(L_ENC_A), onLeftA,  RISING);
  attachInterrupt(digitalPinToInterrupt(R_ENC_A), onRightA, RISING);

  delay(1000);
  Serial.println("=== Encoder Test ===");
  Serial.println("Spin each wheel by hand and watch the counts change.");
}

void loop() {
  // Snapshot counts with interrupts disabled to avoid a torn read
  noInterrupts();
  long l = leftCount;
  long r = rightCount;
  interrupts();

  Serial.print("Left: "); Serial.print(l);
  Serial.print("  Right: "); Serial.println(r);
  delay(200);
}
