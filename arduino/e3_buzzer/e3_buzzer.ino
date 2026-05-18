// e3_buzzer - Melody with explicit PWM frequency per note
// ledcAttach sets up the PWM channel; ledcChangeFrequency changes the pitch.
// Duty cycle is fixed at 50% (128/256) - only frequency changes pitch.
#define BUZZER 4
#define RESOLUTION 8   // 8-bit: duty range 0-255
#define DUTY_50PCT 128 // 50% duty cycle

// Note frequencies (Hz)
#define C4 262
#define E4 330
#define G4 392
#define C5 523

void setup() {
  ledcAttach(BUZZER, C4, RESOLUTION);
  ledcWrite(BUZZER, 0);  // start silent
}

void loop() {
  ledcChangeFrequency(BUZZER, C4, RESOLUTION);
  ledcWrite(BUZZER, DUTY_50PCT);
  delay(200);

  ledcChangeFrequency(BUZZER, E4, RESOLUTION);
  ledcWrite(BUZZER, DUTY_50PCT);
  delay(200);

  ledcChangeFrequency(BUZZER, G4, RESOLUTION);
  ledcWrite(BUZZER, DUTY_50PCT);
  delay(200);

  ledcChangeFrequency(BUZZER, C5, RESOLUTION);
  ledcWrite(BUZZER, DUTY_50PCT);
  delay(400);
  
  delay(1000);
  ledcWrite(BUZZER, 0);
}
