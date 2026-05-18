// e0_serialcomms - Introduction to serial communication
// Prompts for your name over Serial and greets you back.
//
// Demonstrates:
//   - Serial.begin()      open the serial port at a baud rate
//   - Serial.println()    send a line to the PC
//   - Serial.readStringUntil()  receive a line from the PC
//   - String.trim()       strip the trailing newline/whitespace

void setup() {
  Serial.begin(115200);
  delay(1000);  // wait for the Serial Monitor to connect

  Serial.println("Hello! What is your name?");
}

void loop() {
  if (Serial.available()) {
    String name = Serial.readStringUntil('\n');
    name.trim();  // remove trailing \r\n or spaces

    if (name.length() == 0) return;  // ignore empty lines

    Serial.print("Nice to meet you, ");
    Serial.print(name);
    Serial.println("!");
    Serial.println("What is your name?");
  }
}
