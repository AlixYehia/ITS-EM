char command; // Variable to store the incoming command from Bluetooth

int ENA = 3;  // ENA connected to digital pin 3
int ENB = 9;  // ENB connected to digital pin 9
int MOTOR_A1 = 4; // MOTOR_A1 connected to digital pin 4
int MOTOR_A2 = 5; // MOTOR_A2 connected to digital pin 5
int MOTOR_B1 = 6; // MOTOR_B1 connected to digital pin 6
int MOTOR_B2 = 7; // MOTOR_B2 connected to digital pin 7

void setup() {
  pinMode(ENA, OUTPUT); // Initialize ENA pin as an output
  pinMode(ENB, OUTPUT); // Initialize ENB pin as an output
  pinMode(MOTOR_A1, OUTPUT); // Initialize MOTOR_A1 pin as an output
  pinMode(MOTOR_A2, OUTPUT); // Initialize MOTOR_A2 pin as an output
  pinMode(MOTOR_B1, OUTPUT); // Initialize MOTOR_B1 pin as an output
  pinMode(MOTOR_B2, OUTPUT); // Initialize MOTOR_B2 pin as an output
  Serial.begin(9600); // Initialize serial communication
}

void loop() {
  if (Serial.available() > 0) {
    command = Serial.read(); // Read the incoming command from Bluetooth
    executeCommand(command); // Execute the command
  }
}

void executeCommand(char cmd) {
  switch (cmd) {
    case 'F': // Move forward
      moveForward();
      break;
    case 'B': // Move backward
      moveBackward();
      break;
    case 'L': // Turn left
      turnLeft();
      break;
    case 'R': // Turn right
      turnRight();
      break;
    case 'S': // Stop
      stopMotors();
      break;
    default:
      break;
  }
}

void moveForward() {
  digitalWrite(MOTOR_A1, LOW);       // A1 backward of the right side of the car
  digitalWrite(MOTOR_A2, HIGH);      // A2 forward of the right side of the car
  digitalWrite(MOTOR_B1, LOW);       // B1 backward of the left side of the car
  digitalWrite(MOTOR_B2, HIGH);      // B2 forward of the left side of the car
  analogWrite(ENA, 70);
  analogWrite(ENB, 55);
}

void moveBackward() {
  digitalWrite(MOTOR_A1, HIGH);       // A1 backward of the right side of the car
  digitalWrite(MOTOR_A2, LOW);        // A2 forward of the right side of the car
  digitalWrite(MOTOR_B1, HIGH);       // B1 backward of the left side of the car
  digitalWrite(MOTOR_B2, LOW);        // B2 forward of the left side of the car
  analogWrite(ENA, 70);
  analogWrite(ENB, 55);
}

void turnLeft() {
  digitalWrite(MOTOR_A1, LOW);        // A1 backward of the right side of the car
  digitalWrite(MOTOR_A2, HIGH);       // A2 forward of the right side of the car
  digitalWrite(MOTOR_B1, LOW);        // B1 backward of the left side of the car
  digitalWrite(MOTOR_B2, LOW);        // B2 forward of the left side of the car
  analogWrite(ENA, 70);
  analogWrite(ENB, 55);
}

void turnRight() {
  digitalWrite(MOTOR_A1, LOW);        // A1 backward of the right side of the car
  digitalWrite(MOTOR_A2, LOW);        // A2 forward of the right side of the car
  digitalWrite(MOTOR_B1, LOW);        // B1 backward of the left side of the car
  digitalWrite(MOTOR_B2, HIGH);       // B2 forward of the left side of the car
  analogWrite(ENA, 70);
  analogWrite(ENB, 55);
}

void stopMotors() {
  digitalWrite(MOTOR_A1, LOW);        // A1 backward of the left side of the car
  digitalWrite(MOTOR_A2, LOW);        // A2 forward of the right side of the car
  digitalWrite(MOTOR_B1, LOW);        // B1 backward of the right side of the car
  digitalWrite(MOTOR_B2, LOW);        // B2 forward of the left side of the car
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
