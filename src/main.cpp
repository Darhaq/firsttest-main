#include <WiFi.h>
#include <ThingSpeak.h>
#include <Arduino.h>

// Define GPIO pins for sensors
const int sensor1Pin = 25;  // Sensor 1 for "IN"
const int sensor2Pin = 23;  // Sensor 2 for "OUT"

// Wi-Fi credentials
const char* ssid = "E308";           // Replace with your Wi-Fi SSID
const char* password = "98806829";  // Replace with your Wi-Fi password

// ThingSpeak settings
unsigned long channelID = 2771658;          // Channel ID
const char* writeAPIKey = "762I0WX8SZUMHMI4";  // Write API Key
WiFiClient client;

// Variables
unsigned long lastUpdate = 0;  // Last update timestamp
int counterIn = 0;             // People entering
int counterOut = 0;            // People exiting
int finalCount = 0;            // Final visits today
int currentInRoom = 0;         // Current number in room
unsigned long sensor1Cooldown = 0;
unsigned long sensor2Cooldown = 0;

// Flag to track whether finalCount has already been updated
bool finalCountUpdated = false;

void setup() {
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  ThingSpeak.begin(client);
}

void loop() {
  static int lastSensor1Value = 0;
  static int lastSensor2Value = 1;

  int sensor1Value = digitalRead(sensor1Pin);
  int sensor2Value = digitalRead(sensor2Pin);

  unsigned long currentMillis = millis();
  if (sensor1Cooldown > currentMillis) sensor1Value = 0;
  if (sensor2Cooldown > currentMillis) sensor2Value = 1;

  if (lastSensor1Value == 0 && sensor1Value == 1) {
    counterIn++;
    Serial.print("CounterIn: ");
    Serial.println(counterIn);
    sensor2Cooldown = currentMillis + 3000;
  }
  lastSensor1Value = sensor1Value;

  if (lastSensor2Value == 1 && sensor2Value == 0) {
    counterOut++;
    Serial.print("CounterOut: ");
    Serial.println(counterOut);
    sensor1Cooldown = currentMillis + 3000;
  }
  lastSensor2Value = sensor2Value;

  // Calculate the current number of people in the room
  currentInRoom = counterIn - counterOut;

  // Update finalCount only when counterIn equals counterOut
  if (counterIn == counterOut && !finalCountUpdated) {
    finalCount = counterIn; // Update finalCount only once when counters match
    finalCountUpdated = true; // Prevent further updates until new mismatch occurs
  } else if (counterIn != counterOut) {
    finalCountUpdated = false; // Reset flag when counters are mismatched
  }

  if (currentMillis - lastUpdate >= 21000) {
    lastUpdate = currentMillis;

    ThingSpeak.setField(1, counterIn);
    ThingSpeak.setField(2, counterOut);
    ThingSpeak.setField(3, finalCount);
    ThingSpeak.setField(4, currentInRoom);

    int result = ThingSpeak.writeFields(channelID, writeAPIKey);
    if (result == 200) {
      Serial.println("Data sent to ThingSpeak successfully");
    } else {
      Serial.println("Failed to send data to ThingSpeak");
    }
  }

  delay(100);
}
