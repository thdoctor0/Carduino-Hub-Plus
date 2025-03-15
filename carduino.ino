// Carduino Comzas Firebase Edition - thdoctor
// Martin Robotics Ultra-Safe Network

#include <WiFiNINA.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include <Servo.h>

// ================== CONFIGURATION ================== //
#define FIREBASE_HOST "your-project.firebaseio.com"
#define FIREBASE_KEY "your-database-secret"
#define WIFI_SSID "MARTIN ROBOTICS"
#define WIFI_PASS "CARDUINO-MARTIN-ROBOTICS-NETWORK"
#define FIREBASE_PATH "/devices/carduino01"

// Safety Parameters
#define SAFETY_DISTANCE 55    // cm (Normal operation stop)
#define EMERGENCY_DISTANCE 25 // cm (Instant stop)
#define COMMAND_TIMEOUT 10000 // ms (10 seconds)
#define MOTOR_SPEED 175       // PWM (0-255)
#define SCAN_SPEED 45         // Degrees per second

// Pin Configuration
#define MOTOR_A1 2  // FL
#define MOTOR_A2 3  // BL
#define MOTOR_B1 4  // FR
#define MOTOR_B2 5  // BR

#define ENA 9
#define ENB 10

#define TRIG 10
#define ECHO A0

#define SERVO_PIN 11
#define ALARM 12

#define FRONT_LIGHT 6
#define REAR_LIGHT 7
#define LED_PIN 25

// ================== SYSTEM SETUP ================== //
Servo usServo;
unsigned long cmdTimer = 0;
bool autoMode = false;
int currentScanAngle = 90;

// ================== CORE SETUP ================== //
void setup() {
  Serial.begin(115200);

  // mc - Motor control pins
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT); 
  pinMode(MOTOR_B2, OUTPUT);
  pinMode(ENA, OUTPUT); 
  pinMode(ENB, OUTPUT);
  
  // lf - Lighting pins
  pinMode(FRONT_LIGHT, OUTPUT); 
  pinMode(REAR_LIGHT, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(TRIG, OUTPUT); 
  pinMode(ECHO, INPUT);
  
  usServo.attach(SERVO_PIN); 
  connectWiFi(); 
  stopMotors(); 
  lightsOff();  
}

// ================== MAIN LOOP ================== //
void loop() {
  handleFirebase();
  handleCommands();
  handleLights();
  getDistance();

  performSafetyCheck();
  systemHealthCheck();

  if(autoMode) autoDriver();  // mc
  enforceSafety();
}

void handleCommand(String command) {
  command.trim();
  cmdTimer = millis();

  if(command == "UP") moveForward();
  else if(command == "FORWARD") moveForward();
  else if(command == "LEFT") turnLeft();
  else if(command == "RIGHT") turnRight();
  else if(command == "DOWN") moveBackward();
  else if(command == "STOP") stopMotors();
  else if(command == "AUTO") autoMode = true;
  else if(command == "MANUAL") autoMode = false;
  
  sendStatusUpdate();
}

void initializePins() {

  // Gear motor pins
  pinMode(MOTOR_A1, OUTPUT); 
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT); 
  pinMode(MOTOR_B2, OUTPUT);

  // Motor driver pins
  pinMode(ENA, OUTPUT); 
  pinMode(ENB, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG, OUTPUT); 
  pinMode(ECHO, INPUT);

  // Misc pins
  pinMode(ALARM, OUTPUT); 
  pinMode(LED_PIN, OUTPUT);

  // Light controls
  pinMode(FRONT_LIGHT, OUTPUT); 
  pinMode(REAR_LIGHT, OUTPUT);

  // Servo motor for the ultrasonic 
  usServo.attach(SERVO_PIN);
}

void connectNetwork() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(500);
  }
  digitalWrite(LED_PIN, HIGH); // controls the led pin based on wireless connection
}

void initFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
  Firebase.stream(FIREBASE_PATH);
}

// ================== FIREBASE COMMUNICATION ================== //
void handleFirebase() {
  if(Firebase.available()) {
    FirebaseObject event = Firebase.readEvent();
    String path = event.getString("path");
    
    if(path == "/command") {
      handleCommand(event.getString("data"));
    }
    else if(path == "/config") {
      updateConfiguration(event.getJsonObject());
    }
  }
}

// ================== ADVANCED SAFETY SYSTEM ================== //
void performSafetyCheck() {
  float dist = getDistance();
  
  if(dist < EMERGENCY_DISTANCE) {
    emergencyStop();
    sendEmergencyAlert(dist);
  }
  else if(dist < SAFETY_DISTANCE) {
    avoidCollision(dist);
  }
}

void avoidCollision(float distance) {
  stopMotors();
  usServo.write(180); // goes to the right 
  float leftDist = getDistance();
  usServo.write(0); // goes to the left
  float rightDist = getDistance();
  usServo.write(90); // goes to the middle
  // the right left middle cycle is the best because of a faster scanning method because it doesnt need to travel that much in an axis that results in the piece of the code just making out one of the boldest moves and notes the netire thing and the right left middle cycle is more controlable giving me flexibility 

  if(leftDist > rightDist) turnLeft();
  else turnRight();
  
  moveForward();
  delay(map(distance, EMERGENCY_DISTANCE, SAFETY_DISTANCE, 500, 2000));
  stopMotors();
}

// ================== ENHANCED AUTONOMOUS ================== //
void advancedAutoDriver() {
  static unsigned long lastScan = 0;
  if(millis() - lastScan > 1000/SCAN_SPEED) {
    currentScanAngle = (currentScanAngle + 5) % 180;
    usServo.write(currentScanAngle);
    lastScan = millis();
  }

  float frontDist = getDistance();
  if(frontDist > SAFETY_DISTANCE) {
    moveForward();
  } else {
    avoidCollision(frontDist);
  }
}

// ================== CORE FUNCTIONS ================== //
void driveMotors(int a1, int a2, int b1, int b2) {
  digitalWrite(MOTOR_A1, a1); digitalWrite(MOTOR_A2, a2);
  digitalWrite(MOTOR_B1, b1); digitalWrite(MOTOR_B2, b2);
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
}

float getDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH) * 0.034 / 2;
}

void systemHealthCheck() {
  static unsigned long lastUpdate = 0;
  if(millis() - lastUpdate > 5000) {
    sendStatusUpdate();
    lastUpdate = millis();
  }
}

void sendStatusUpdate() {
  Firebase.setFloat(FIREBASE_PATH + "/status/distance", getDistance());
  Firebase.setBool(FIREBASE_PATH + "/status/autoMode", autoMode);
  Firebase.setInt(FIREBASE_PATH + "/status/battery", readBattery());
}

// ================== MOTOR CONTROL (mc) ================== //
// mc - Motor driver core function
void driveMotors(int a1, int a2, int b1, int b2) {
  digitalWrite(MOTOR_A1, a1); 
  digitalWrite(MOTOR_A2, a2);
  digitalWrite(MOTOR_B1, b1); 
  digitalWrite(MOTOR_B2, b2);
  analogWrite(ENA, 150); 
  analogWrite(ENB, 150);
}

// mc - Forward movement
void moveForward() { driveMotors(HIGH, LOW, HIGH, LOW); }

// mc - Left turn
void turnLeft() { driveMotors(LOW, HIGH, HIGH, LOW); }

// mc - Right turn
void turnRight() { driveMotors(HIGH, LOW, LOW, HIGH); }

// mc - Backward movement
void moveBackward() { driveMotors(LOW, HIGH, LOW, HIGH); }

// mc - Stop all motors
void stopMotors() { driveMotors(LOW, LOW, LOW, LOW); }

// ================== LIGHTING FUNCTIONS (lf) ================== //
// lf - Handle lighting system
void handleLights() {
  digitalWrite(FRONT_LIGHT, autoMode ? HIGH : LOW);
  analogWrite(REAR_LIGHT, (getDistance() < 50) ? 255 : 50);
}

// lf - Turn off all lights
void lightsOff() { 
  digitalWrite(FRONT_LIGHT, LOW); 
  digitalWrite(REAR_LIGHT, LOW); 
}

// ================== SAFETY SYSTEM ================== //
void enforceSafety() {
  if(getDistance() < 20) emergencyStop();  // mc
  if(millis() - cmdTimer > 10000) stopSystem();
}

void stopSystem() { 
  stopMotors();  // mc
  lightsOff();   // lf
  autoMode = false;
  cmdTimer = millis();
}

// this is like the 10 th time i recoded this from 0 i hate this 