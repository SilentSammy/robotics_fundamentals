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
- GPIO7: IN1
- GPIO8: IN2

### Right Motor
- GPIO5: IN1
- GPIO6: IN2

### Left Encoder
- GPIO20: Channel A
- GPIO21: Channel B

### Right Encoder
- GPIO9: Channel A
- GPIO10: Channel B

### IR Sensor
- GPIO0: Digital input

### Ultrasonic Sensor
- GPIO2: TRIG
- GPIO1: ECHO

### Buzzer
- GPIO3: Signal (PWM capable)

## Notes
- Motor control uses 2 pins per motor (direction + PWM speed control on one pin)
- All encoder pins support interrupt-driven counting
- Buzzer pin supports PWM for tone generation
- GPIO6 is used for ultrasonic ECHO but I2C can be reconfigured to GPIO20/21 if UART is not needed
