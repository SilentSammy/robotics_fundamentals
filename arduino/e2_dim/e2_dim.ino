// e2_dim - PWM dimming at 0/25/50/75/100% with explicit duty cycle
// ledcAttach configures the PWM channel (frequency + resolution).
// ledcWrite sets the duty cycle: duty / (2^resolution) = brightness %
// e.g. 8-bit resolution -> 128/256 = 50%
#define LED 8
#define FREQ 5000     // 5kHz - fast enough to avoid flicker
#define RESOLUTION 8  // 8-bit: duty range 0-255

void setup() {
  ledcAttach(LED, FREQ, RESOLUTION);
}

void loop() {
  ledcWrite(LED, 0);    // 0%
  delay(1000);
  ledcWrite(LED, 64);   // 25%
  delay(1000);
  ledcWrite(LED, 128);  // 50%
  delay(1000);
  ledcWrite(LED, 191);  // 75%
  delay(1000);
  ledcWrite(LED, 255);  // 100%
  delay(1000);
}
