# Robotics Fundamentals - Instructor Notes

## Hardware
- **MCU**: ESP32-C3 Super Mini
- 2x DC Motors with quadrature encoders
- 1x Ultrasonic sensor (HC-SR04 or similar)
- 1x IR sensor
- 1x Buzzer
- Motor driver (L298N or similar dual H-bridge)

## Pinout Configuration

### Left Motor
- GPIO20: IN1
- GPIO21: IN2

### Right Motor
- GPIO0: IN1
- GPIO10: IN2

### Left Encoder
- GPIO7: Channel A
- GPIO3: Channel B

### Right Encoder
- GPIO5: Channel A
- GPIO6: Channel B

### IR Sensor
- GPIO9: Digital input

### Ultrasonic Sensor
- GPIO2: TRIG
- GPIO1: ECHO

### Buzzer
- GPIO4: Signal (PWM capable)

## Notes
- Motor PWM: 100 Hz, 10-bit resolution (0–1023 duty range) via LEDC API
- Low PWM frequency (~100 Hz) improves torque at the cost of audible hum
- All encoder pins support interrupt-driven quadrature decoding
- Buzzer uses `tone()` for frequency generation
