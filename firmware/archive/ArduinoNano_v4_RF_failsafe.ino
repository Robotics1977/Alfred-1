// === Alfred 1 Motor Firmware - Full Sensor Integration + RF Failsafe ===
// TB6612FNG Motor Driver + Quadrature Encoders + HC-SR04 Ultrasonics + 433MHz Failsafe
// NOTE: RF failsafe hardware integration was deferred. Kept here for when that work resumes.

#include <RCSwitch.h>

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

// 433MHz RF Receiver pin
const int RF_RECEIVER_PIN = A5;  // Connect receiver DATA pin here

// Configuration
const int MAX_SPEED = 255;
const int MIN_SPEED = 0;
const unsigned long TIMEOUT_MS = 1000;
const int DEADBAND = 10;
const unsigned long ENCODER_REPORT_INTERVAL = 50;  // Report every 50ms (20Hz)
const unsigned long SONAR_READ_INTERVAL = 100;     // Read sonars every 100ms (10Hz)
const int SONAR_MAX_DISTANCE = 400;                // Max distance in cm

// RF Failsafe Configuration
const unsigned long FAILSAFE_TIMEOUT = 500;         // 500ms without signal = stop
const unsigned long EMERGENCY_STOP_CODE = 123456;   // Emergency stop button code
const unsigned long KEEPALIVE_CODE = 654321;        // Transmitter heartbeat code

// State tracking
unsigned long lastCommandTime = 0;
unsigned long lastEncoderReportTime = 0;
unsigned long lastSonarReadTime = 0;
unsigned long lastRFSignalTime = 0;
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
bool motorsEnabled = true;
bool rfFailsafeActive = false;

// Encoder variables (volatile for interrupt safety)
volatile long encoderLeftCount = 0;
volatile long encoderRightCount = 0;
volatile int lastEncLeftA = 0;
volatile int lastEncRightA = 0;

// Sonar distances (cm)
int sonarRear = 0;
int sonarLeft = 0;
int sonarRight = 0;

// RF Receiver object
RCSwitch mySwitch = RCSwitch();

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
  
  // Initialize RF receiver
  mySwitch.enableReceive(digitalPinToInterrupt(RF_RECEIVER_PIN));
  lastRFSignalTime = millis();
  
  // Start with motors stopped
  stopMotors();
  digitalWrite(STBY, HIGH);
  
  // Setup encoder interrupts on channel A pins
  // Note: A0-A3 don't support hardware interrupts on Nano, we'll poll them
  lastEncLeftA = digitalRead(ENC_A_A);
  lastEncRightA = digitalRead(ENC_B_A);
  
  Serial.println("Alfred Motor Controller v4.0 - RF Failsafe Enabled");
  Serial.println("Commands: SET <left> <right>, STOP, STATUS, ENABLE, DISABLE, RESET_ENC, SENSORS, RF_STATUS");
  Serial.println("Sensors: Quadrature Encoders + 3x HC-SR04 Ultrasonics + 433MHz RF Failsafe");
  Serial.println("Speed range: -255 to +255");
  Serial.println("RF Failsafe: Emergency stop + Out-of-range protection");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check RF failsafe FIRST (highest priority)
  checkRFFailsafe();
  
  // Update encoders (polling method since analog pins don't support interrupts)
  updateEncoders();
  
  // Safety timeout
  if (motorsEnabled && !rfFailsafeActive && (currentTime - lastCommandTime > TIMEOUT_MS)) {
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
      if (!rfFailsafeActive) {
        motorsEnabled = true;
        digitalWrite(STBY, HIGH);
        lastCommandTime = millis();
        Serial.println("OK: Motors enabled");
      } else {
        Serial.println("ERROR: Cannot enable - RF failsafe active");
      }
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
    else if (input == "RF_STATUS") {
      printRFStatus();
    }
    else if (input == "HELP") {
      printHelp();
    }
    else {
      Serial.println("ERROR: Unknown command. Type HELP for commands.");
    }
  }
}

void checkRFFailsafe() {
  // Check for incoming RF signals
  if
