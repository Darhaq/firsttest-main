#include <WiFi.h>
#include <ThingSpeak.h>
#include <Arduino.h>

// Define GPIO pins for sensors
const int sensor1Pin = 25;  // Sensor 1 for "IN"
const int sensor2Pin = 23;  // Sensor 2 for "OUT"

// Wi-Fi credentials
const char* ssid = "E308";           // Replace with your Wi-Fi SSID
const char* password = "98806829";  // Replace with your Wi-Fi password

//const char* ssid = "TP-link2B15";           // Replace with your Wi-Fi SSID
//const char* password = "15282996"; 


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

void setup() {
  // Initialize sensor pins as input
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);

  // Start the serial monitor
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  static int lastSensor1Value = 0;
  static int lastSensor2Value = 1;

  // Read sensor values
  int sensor1Value = digitalRead(sensor1Pin);
  int sensor2Value = digitalRead(sensor2Pin);

  // Manage sensor cooldowns (3 seconds)
  unsigned long currentMillis = millis();
  if (sensor1Cooldown > currentMillis) sensor1Value = 0;
  if (sensor2Cooldown > currentMillis) sensor2Value = 1;

  // Sensor 1 logic: Increment counterIn when value changes from 0 to 1
  if (lastSensor1Value == 0 && sensor1Value == 1) {
    counterIn++;
    Serial.print("CounterIn: ");
    Serial.println(counterIn);

    // Shutdown sensor 2 for 3 seconds
    sensor2Cooldown = currentMillis + 3000;
  }
  lastSensor1Value = sensor1Value;

  // Sensor 2 logic: Increment counterOut when value changes from 1 to 0
  if (lastSensor2Value == 1 && sensor2Value == 0) {
    counterOut++;
    Serial.print("CounterOut: ");
    Serial.println(counterOut);

    // Shutdown sensor 1 for 3 seconds
    sensor1Cooldown = currentMillis + 3000;
  }
  lastSensor2Value = sensor2Value;

  // Calculate currentInRoom and finalCount
  currentInRoom = counterIn - counterOut;
  if (counterIn == counterOut) {
    finalCount = counterIn;
  }

  // Send data to ThingSpeak every 21 seconds
  if (currentMillis - lastUpdate >= 21000) {
    lastUpdate = currentMillis;

    // Update ThingSpeak fields
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
  delay(100);  // Small delay to stabilize readings
}