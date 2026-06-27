// === Alfred 1 Motor Firmware - Full Sensor Integration ===
// TB6612FNG Motor Driver + Quadrature Encoders + HC-SR04 Ultrasonics

// Motor Driver pins
const int STBY = A4;
const int AIN1 = 8;
const int AIN2 = 9;
const int APWM = 10;
const int BIN1 = 11;
const int BIN2 = 12;
const int BPWM = 13;

// Encoder pins (Quadrature)
const int ENC_A_A = A0;  // Motor A (Left) Encoder Channel A
const int ENC_A_B = A1;  // Motor A (Left) Encoder Channel B
const int ENC_B_A = A2;  // Motor B (Right) Encoder Channel A
const int ENC_B_B = A3;  // Motor B (Right) Encoder Channel B

// HC-SR04 Ultrasonic pins
const int SONAR_REAR_TRIG = 2;
const int SONAR_REAR_ECHO = 3;
const int SONAR_LEFT_TRIG = 4;
const int SONAR_LEFT_ECHO = 5;
const int SONAR_RIGHT_TRIG = 6;
const int SONAR_RIGHT_ECHO = 7;

// Configuration
const int MAX_SPEED = 255;
const int MIN_SPEED = 0;
const unsigned long TIMEOUT_MS = 1000;
const int DEADBAND = 10;
const unsigned long ENCODER_REPORT_INTERVAL = 50;  // Report every 50ms (20Hz)
const unsigned long SONAR_READ_INTERVAL = 100;     // Read sonars every 100ms (10Hz)
const int SONAR_MAX_DISTANCE = 400;                // Max distance in cm

// State tracking
unsigned long lastCommandTime = 0;
unsigned long lastEncoderReportTime = 0;
unsigned long lastSonarReadTime = 0;
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
bool motorsEnabled = true;

// Encoder variables (volatile for interrupt safety)
volatile long encoderLeftCount = 0;
volatile long encoderRightCount = 0;
volatile int lastEncLeftA = 0;
volatile int lastEncRightA = 0;

// Sonar distances (cm)
int sonarRear = 0;
int sonarLeft = 0;
int sonarRight = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize motor pins
  pinMode(STBY, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(APWM, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(BPWM, OUTPUT);
  
  // Initialize encoder pins
  pinMode(ENC_A_A, INPUT_PULLUP);
  pinMode(ENC_A_B, INPUT_PULLUP);
  pinMode(ENC_B_A, INPUT_PULLUP);
  pinMode(ENC_B_B, INPUT_PULLUP);
  
  // Initialize ultrasonic pins
  pinMode(SONAR_REAR_TRIG, OUTPUT);
  pinMode(SONAR_REAR_ECHO, INPUT);
  pinMode(SONAR_LEFT_TRIG, OUTPUT);
  pinMode(SONAR_LEFT_ECHO, INPUT);
  pinMode(SONAR_RIGHT_TRIG, OUTPUT);
  pinMode(SONAR_RIGHT_ECHO, INPUT);
  
  // Start with motors stopped
  stopMotors();
  digitalWrite(STBY, HIGH);
  
  // Setup encoder interrupts on channel A pins
  // Note: A0-A3 don't support hardware interrupts on Nano, we'll poll them
  lastEncLeftA = digitalRead(ENC_A_A);
  lastEncRightA = digitalRead(ENC_B_A);
  
  Serial.println("Alfred Motor Controller v3.0 - Full Sensors");
  Serial.println("Commands: SET <left> <right>, STOP, STATUS, ENABLE, DISABLE, RESET_ENC, SENSORS");
  Serial.println("Sensors: Quadrature Encoders + 3x HC-SR04 Ultrasonics");
  Serial.println("Speed range: -255 to +255");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update encoders (polling method since analog pins don't support interrupts)
  updateEncoders();
  
  // Safety timeout
  if (motorsEnabled && (currentTime - lastCommandTime > TIMEOUT_MS)) {
    stopMotors();
    Serial.println("TIMEOUT: Motors stopped");
    motorsEnabled = false;
  }
  
  // Report encoder data periodically
  if (currentTime - lastEncoderReportTime >= ENCODER_REPORT_INTERVAL) {
    reportEncoders();
    lastEncoderReportTime = currentTime;
  }
  
  // Read ultrasonic sensors periodically
  if (currentTime - lastSonarReadTime >= SONAR_READ_INTERVAL) {
    readAllSonars();
    lastSonarReadTime = currentTime;
  }
  
  // Process serial commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();
    
    if (input.startsWith("SET")) {
      handleSetCommand(input);
    }
    else if (input == "STOP") {
      stopMotors();
      Serial.println("OK: Motors stopped");
    }
    else if (input == "STATUS") {
      printStatus();
    }
    else if (input == "ENABLE") {
      motorsEnabled = true;
      digitalWrite(STBY, HIGH);
      lastCommandTime = millis();
      Serial.println("OK: Motors enabled");
    }
    else if (input == "DISABLE") {
      stopMotors();
      motorsEnabled = false;
      digitalWrite(STBY, LOW);
      Serial.println("OK: Motors disabled");
    }
    else if (input == "RESET_ENC") {
      resetEncoders();
      Serial.println("OK: Encoders reset");
    }
    else if (input == "SENSORS") {
      printSensorData();
    }
    else if (input == "HELP") {
      printHelp();
    }
    else {
      Serial.println("ERROR: Unknown command. Type HELP for commands.");
    }
  }
}

void updateEncoders() {
  // Read current state of encoder A channels
  int encLeftA = digitalRead(ENC_A_A);
  int encLeftB = digitalRead(ENC_A_B);
  int encRightA = digitalRead(ENC_B_A);
  int encRightB = digitalRead(ENC_B_B);
  
  // Left encoder (Motor A)
  if (encLeftA != lastEncLeftA) {
    if (encLeftA == encLeftB) {
      encoderLeftCount++;
    } else {
      encoderLeftCount--;
    }
    lastEncLeftA = encLeftA;
  }
  
  // Right encoder (Motor B)
  if (encRightA != lastEncRightA) {
    if (encRightA == encRightB) {
      encoderRightCount++;
    } else {
      encoderRightCount--;
    }
    lastEncRightA = encRightA;
  }
}

void resetEncoders() {
  encoderLeftCount = 0;
  encoderRightCount = 0;
}

void reportEncoders() {
  // Format: ENC <left_count> <right_count>
  Serial.print("ENC ");
  Serial.print(encoderLeftCount);
  Serial.print(" ");
  Serial.println(encoderRightCount);
}

void readAllSonars() {
  sonarRear = readUltrasonic(SONAR_REAR_TRIG, SONAR_REAR_ECHO);
  sonarLeft = readUltrasonic(SONAR_LEFT_TRIG, SONAR_LEFT_ECHO);
  sonarRight = readUltrasonic(SONAR_RIGHT_TRIG, SONAR_RIGHT_ECHO);
  
  // Format: SONAR <rear> <left> <right>
  Serial.print("SONAR ");
  Serial.print(sonarRear);
  Serial.print(" ");
  Serial.print(sonarLeft);
  Serial.print(" ");
  Serial.println(sonarRight);
}

int readUltrasonic(int trigPin, int echoPin) {
  // Send trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read echo pulse
  long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
  
  // Calculate distance in cm
  int distance = duration / 58;
  
  // Return 0 if out of range or no echo
  if (distance <= 0 || distance > SONAR_MAX_DISTANCE) {
    return 0;
  }
  
  return distance;
}

void handleSetCommand(String input) {
  int firstSpace = input.indexOf(' ');
  int secondSpace = input.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    Serial.println("ERROR: Invalid format. Use: SET <left> <right>");
    return;
  }
  
  int left = input.substring(firstSpace + 1, secondSpace).toInt();
  int right = input.substring(secondSpace + 1).toInt();
  
  if (abs(left) > MAX_SPEED || abs(right) > MAX_SPEED) {
    Serial.println("ERROR: Speed out of range (-255 to +255)");
    return;
  }
  
  if (abs(left) < DEADBAND) left = 0;
  if (abs(right) < DEADBAND) right = 0;
  
  setMotorSpeeds(left, right);
  lastCommandTime = millis();
  motorsEnabled = true;
  
  Serial.print("OK: Left=");
  Serial.print(currentLeftSpeed);
  Serial.print(" Right=");
  Serial.println(currentRightSpeed);
}

void setMotorSpeeds(int left, int right) {
  currentLeftSpeed = constrain(left, -MAX_SPEED, MAX_SPEED);
  currentRightSpeed = constrain(right, -MAX_SPEED, MAX_SPEED);
  
  // Motor A (Left)
  if (currentLeftSpeed > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(APWM, currentLeftSpeed);
  } else if (currentLeftSpeed < 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(APWM, abs(currentLeftSpeed));
  } else {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
    analogWrite(APWM, 0);
  }
  
  // Motor B (Right)
  if (currentRightSpeed > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(BPWM, currentRightSpeed);
  } else if (currentRightSpeed < 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(BPWM, abs(currentRightSpeed));
  } else {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, HIGH);
    analogWrite(BPWM, 0);
  }
}

void stopMotors() {
  setMotorSpeeds(0, 0);
  currentLeftSpeed = 0;
  currentRightSpeed = 0;
}

void printStatus() {
  Serial.println("=== MOTOR STATUS ===");
  Serial.print("Enabled: ");
  Serial.println(motorsEnabled ? "YES" : "NO");
  Serial.print("Left Speed: ");
  Serial.println(currentLeftSpeed);
  Serial.print("Right Speed: ");
  Serial.println(currentRightSpeed);
  Serial.print("Time since last command: ");
  Serial.print(millis() - lastCommandTime);
  Serial.println(" ms");
  Serial.println();
  printSensorData();
}

void printSensorData() {
  Serial.println("=== SENSOR DATA ===");
  Serial.print("Left Encoder: ");
  Serial.println(encoderLeftCount);
  Serial.print("Right Encoder: ");
  Serial.println(encoderRightCount);
  Serial.print("Sonar Rear: ");
  Serial.print(sonarRear);
  Serial.println(" cm");
  Serial.print("Sonar Left: ");
  Serial.print(sonarLeft);
  Serial.println(" cm");
  Serial.print("Sonar Right: ");
  Serial.print(sonarRight);
  Serial.println(" cm");
}

void printHelp() {
  Serial.println("=== ALFRED MOTOR CONTROLLER v3.0 ===");
  Serial.println("Commands:");
  Serial.println("  SET <left> <right> - Set motor speeds (-255 to +255)");
  Serial.println("  STOP               - Stop all motors");
  Serial.println("  STATUS             - Show motor and sensor status");
  Serial.println("  ENABLE             - Enable motor driver");
  Serial.println("  DISABLE            - Disable motor driver");
  Serial.println("  RESET_ENC          - Reset encoder counts to zero");
  Serial.println("  SENSORS            - Show all sensor readings");
  Serial.println("  HELP               - Show this help");
  Serial.println();
  Serial.println("Automatic Data Streams:");
  Serial.println("  ENC <left> <right>        - Encoder counts (20Hz)");
  Serial.println("  SONAR <rear> <left> <right> - Distances in cm (10Hz)");
  Serial.println();
  Serial.println("Examples:");
  Serial.println("  SET 100 100   - Forward at ~40% speed");
  Serial.println("  SET -150 -150 - Reverse at ~60% speed");
  Serial.println("  SET 200 -200  - Spin in place");
  Serial.println("  RESET_ENC     - Zero out encoder counts");
}
