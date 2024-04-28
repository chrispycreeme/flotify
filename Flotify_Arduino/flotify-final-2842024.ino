// inayos ni tofer, paki add sa fb: Christofer D. Banluta <3


#include <Arduino.h>
#include <SoftwareSerial.h>
#define rainSensorPin A0;

// Define SoftwareSerial object for SIM communication
SoftwareSerial sim(10, 11);

// SoftwareSerial for HM10 - BLE
SoftwareSerial bleHM10(8, 9);  // RX, TX

// Define pins
const int trigPin = 5;
const int echoPin = 6;

// Global variables
long duration;
float distance;
int safetyDistance;
int _timeout;
String _buffer;
String number = "+639086476904";

int floodLevelIndicator;  //para sa BLE, kung saan may specific value nag ddetermine sa level ng flood and rain;
int rainLevelIndicator;

int noRainLowerLimit = 900;  // u can modify or add more levels ng rain;
int midRainLowerLimit = 600;
int strongRainUpperLimit = 300;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  // Reserve memory for buffer
  _buffer.reserve(50);
  Serial.println("System Started...");

  // Initialize SoftwareSerial for SIM and BLE communication
  sim.begin(9600);
  delay(1000);

  bleHM10.begin(9600);
  delay(1000);

  // Set pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(rainSensorPin, INPUT);
}

void loop() {
  //initialize rain sensor analog read pin
  int rainSensorValues = analogRead(rainSensorPin);
  // Ultrasonic sensor code to measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  safetyDistance = distance;
  float distanceInInches = distance / 2.54; 

  //  take actions based on distance
  if (distance < 30) {
    callNumber();
    floodLevelIndicator = 1;
  } else if (distance < 60) {
    SendMessage();
    floodLevelIndicator = 2;
  } else if (distance < 90) {
    floodLevelIndicator = 3;
  } else {
    floodLevelIndicator = 0;
  }

  // Print distance to serial monitor
  Serial.print("Distance: ");
  Serial.println(distanceInInches);
  Serial.println(" in");
  int distancePercentage = map(distance, 0, 100, 100, 0);  // 100% if yung distance ay closer to 0cm, 0% if you distance is closer to 100cm

  //logic workaround: kaya lower limit nalagay ko, ay para mas madaling paglaruan; for example:
  // sa mid rain, kung yung sensor values ay Below ng noRainLowerLimit AND greater than midRainLowerLimit,
  // sinasabi lang talaga na inbetween sa two values na yun. which means na yung Mid Rain Values has to be somewhere 899 to 601.
  // that way, mas accurate kung anong error ilalabas niya.

  if (rainSensorPin > noRainLowerLimit) {
    Serial.println("No Rain");
    rainLevelIndicator = 1;
  } else if (rainSensorPin < noRainLowerLimit && rainSensorPin > midRainLowerLimit) {
    Serial.println("Mid Rain");
    rainLevelIndicator = 2;
  } else if (rainSensorPin < strongRainUpperLimit) {
    Serial.println("malakas na ulan super");
    rainLevelIndicator = 3;
  }

  //print values para kay hm10

  bleHM10.print(rainSensorValues); //idx 1....
  bleHM10.print(",");
  bleHM10.print(rainLevelIndicator);
  bleHM10.print(",");
  bleHM10.print(distanceInInches);
  bleHM10.print(",");
  bleHM10.print(distancePercentage);
  bleHM10.println(",");

  delay(500);
}

// Function to send SMS
void SendMessage() {
  sim.println("AT+CMGF=1");  // Set SMS mode to text
  delay(100);
  sim.println("AT+CMGS=\"" + number + "\"\r");  // Set SMS recipient
  delay(200);
  String SMS = "BABALA: ANG TUBIG SA ILOG AY NASA GREEN LEVEL NA O 8 METERS! ITAAS ANG MGA GAMIT SA BAHAY AT PUMUNTA SA PINAKAMALAPIT NA EVACUATION CENTER. MAG-INGAT SA PAGLIKAS!";
  sim.println(SMS);  // Send SMS
  delay(100);
  sim.println((char)26);  // Send Ctrl+Z to indicate end of message
  delay(100);
  _buffer = _readSerial();  // Read response from SIM
}

// Function to read from SoftwareSerial
String _readSerial() {
  _timeout = 0;
  while (!sim.available() && _timeout < 12000) {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}

// Function to make a call
void callNumber() {
  sim.print(F("ATD"));      // Command to dial number
  sim.print(number);        // Dial specified number
  sim.print(F(";\r\n"));    // End command
  _buffer = _readSerial();  // Read response from SIM
  Serial.println(_buffer);  // Print response to serial monitor
}