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

## Serial Telemetry Protocol (e11_serialtelem)

### Output — 20 Hz CSV
```
EL,ER,IR,US_cm
```
| Column | Type | Description |
|---|---|---|
| `EL` | int | Left encoder count (signed, interrupt-driven) |
| `ER` | int | Right encoder count (signed, interrupt-driven) |
| `IR` | 0/1 | IR sensor (1 = clear, 0 = obstacle detected) |
| `US_cm` | float | Ultrasonic distance in cm (-1 = out of range) |

### Input — CSV command
```
a, b [, direct]
```
| Field | Range | Description |
|---|---|---|
| `a` | -1.0 – 1.0 | Linear velocity x (mixed) or left wheel speed (direct) |
| `b` | -1.0 – 1.0 | Angular velocity w (mixed) or right wheel speed (direct) |
| `direct` | 0 / 1 | Mode: `0` = mixed (default), `1` = direct |

**Mixed mode** (`direct=0`): `(a, w)` are body-frame commands, mixed into left/right wheel speeds on the MCU.
**Direct mode** (`direct=1`): `(a, b)` are sent straight to the left and right motors, bypassing mixing.
