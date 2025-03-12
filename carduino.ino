#include <WiFiNINA.h>
#include <Servo.h>

#define AUTHOR "thdoctor"

#define MOTOR_A_IN1 2
#define MOTOR_A_IN2 3
#define MOTOR_B_IN1 4
#define MOTOR_B_IN2 5
#define ENA 9
#define ENB 10
#define US_ANALOG_PIN A0
#define SERVO_PIN 6

Servo usServo;
WiFiSSLClient client;
HttpClient http(client, "api.github.com", 443);

const char* SSID = "MARTIN ROBOTICS";
const char* PASS = "CARDUINO-MARTIN-ROBOTICS-NETWORK";
unsigned long lastCommandTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  usServo.attach(SERVO_PIN);
  usServo.write(90);
  connectWiFi();
  stopMotors();
}

void loop() {
  static String currentCommand = "";
  static unsigned long lastCheck = 0;
  
  if(millis() - lastCheck > 3000) {
    currentCommand = fetchGitHubCommand();
    lastCheck = millis();
  }
  
  if(currentCommand.length() > 0) {
    executeCommand(currentCommand);
    lastCommandTime = millis();
  }
  
  if(millis() - lastCommandTime > 10000) {
    stopMotors();
    lastCommandTime = millis();
  }
  
  if(analogRead(US_ANALOG_PIN) * 0.488 < 20) emergencyStop();
}

String fetchGitHubCommand() {
  http.beginRequest();
  http.get("/repos/thdoctor0/Carduino-Hub-Plus/command.json");
  http.endRequest();
  
  if(http.responseStatusCode() != 200) return "";
  
  String response = client.readString();
  DynamicJsonDocument doc(256);
  deserializeJson(doc, response);
  
  String content = doc["content"].as<String>();
  content.replace("\n", "");
  return base64Decode(content);
}

void executeCommand(String cmd) {
  cmd.trim();
  if(cmd == "UP") moveForward();
  else if(cmd == "DOWN") moveBackward();
  else if(cmd == "LEFT") turnLeft();
  else if(cmd == "RIGHT") turnRight();
  else if(cmd == "STOP") stopMotors();
}

void emergencyStop() {
  stopMotors();
  while(analogRead(US_ANALOG_PIN) * 0.488 < 30) {
    usServo.write(180);
    delay(300);
    usServo.write(0);
    delay(300);
  }
  usServo.write(90);
}

void moveForward() {
  digitalWrite(MOTOR_A_IN1, HIGH);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN1, HIGH);
  digitalWrite(MOTOR_B_IN2, LOW);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
}

void turnLeft() {
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, HIGH);
  digitalWrite(MOTOR_B_IN1, HIGH);
  digitalWrite(MOTOR_B_IN2, LOW);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
}

void stopMotors() {
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

String base64Decode(String input) {
  int len = base64_dec_len(input.c_str(), input.length());
  char output[len];
  base64_decode(output, input.c_str(), input.length());
  return String(output);
}

void connectWiFi() {
  while(WiFi.begin(SSID, PASS) != WL_CONNECTED);
}
