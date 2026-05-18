# Robotics Fundamentals

A hands-on workshop where you build and program a differential drive robot from scratch. Each exercise introduces one new concept, and by the end you'll have a robot that senses its environment and drives itself.

## The Robot

An **ESP32-C3** microcontroller drives two DC motors through an H-bridge. It has:
- Quadrature encoders on both wheels
- An IR sensor (line detection)
- An ultrasonic sensor (distance)
- A buzzer

---

## Concepts

### Serial Communication
Every sketch you write will use serial to print debug output. Before touching hardware, `e0_serialcomms` establishes the basic loop: the MCU sends text, you read it; you send text, the MCU reads it. This pattern — send a command, get a response — underpins everything else in the workshop.

### Digital I/O
A GPIO pin is either HIGH or LOW. `e1_led` drives an output pin to blink an LED. `e5_irsensor` reads an input pin from a sensor that detects whether a surface is reflective. Most hardware interaction starts here.

### PWM — Pulse Width Modulation
A pin can only be fully on or fully off, but switching it on and off fast enough produces an *effective* intermediate voltage. `e2_dim` uses this to smoothly dim an LED. `e3_buzzer` takes it further: changing the switching frequency changes the pitch of the buzzer. Motor speed control uses the same principle throughout.

### H-Bridge Motor Control
An H-bridge lets you reverse current direction through a motor, giving you forward, reverse, and brake. `e4_hbridge` drives both wheels through a sequence of movements. The same `setMotor()` pattern is reused in every subsequent sketch.

### Interrupts and Encoders
Polling a pin in `loop()` is fine for slow sensors, but motor encoders tick hundreds of times per second. Missing a tick means losing position. `e7_encoders` attaches an interrupt to each encoder's channel A — the MCU pauses whatever it's doing, increments a counter, and resumes. The direction is determined by sampling channel B at that moment.

### Reactive Control
The simplest form of autonomous behavior: if condition → do this, else → do that. `e8_linefollower` steers left when the IR sensor sees a line and right when it doesn't, chasing the edge. `e9_obstacle` stops the motors the moment the ultrasonic sensor reads closer than a threshold. No memory, no model — just stimulus and response.

### Proportional Control
Bang-bang control is abrupt. A proportional controller scales the correction by how far off you are — a small error produces a small correction, a large error produces a large one. `e10_wallhugger` uses this to keep the robot at a fixed distance from a wall: `correction = Kp × (measured − target)`, mixed into the left and right wheel speeds.

### Telemetry and Remote Control
A robot that can't report what it's sensing is hard to tune. `e11_serialtelem` streams all sensor readings to the PC at 20 Hz in CSV format while simultaneously listening for drive commands. The `ble_server` sketch does the same wirelessly over BLE, letting you control the robot from a Python script without a USB cable.

---

## Utility Sketches

- `hello_world` — confirms the board is alive and serial is working
- `buzzer_test` — verifies the buzzer and plays a few tones
- `hardware_test` — runs all motors, reads all sensors in one go; use this first

## BLE Remote Control

`ble_server` turns the robot into a BLE peripheral. A host (PC or phone) connects and sends `(x, w)` commands — linear and angular velocity — which are mixed into left/right motor speeds on the robot.

---

## Serial Telemetry Format (`e11_serialtelem`)

**Output** — 20 Hz CSV:
```
EL,ER,IR,US_cm
```

**Input** — CSV command:
```
a,b[,direct]
```
`direct=0` (default): mixed mode — `a` is linear velocity, `b` is angular velocity.  
`direct=1`: direct mode — `a` and `b` are left and right wheel speeds directly.

All values in the range `−1.0` to `1.0`.
