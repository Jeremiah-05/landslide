/*
 * Landslide Detection System
 * ---------------------------
 * Karunya Institute of Technology and Sciences
 * 23CS2050 - Embedded Systems, April 2025
 *
 * Authors: Jeremiah J (URK23CS1218), Maha Deepak S (URK23CS1220),
 *          Nithishkumar K (URK23CS1201)
 *
 * Monitors soil moisture, ground vibration, rainfall, temperature, and
 * humidity around a slope. Displays live readings on an OLED, gives a
 * local buzzer/LED warning, logs data to ThingSpeak, and lets a user
 * control the alert relay remotely through the Blynk app.
 *
 * SETUP:
 *   1. Copy "secrets.h.example" to "secrets.h" in this same folder.
 *   2. Fill in your own Wi-Fi, Blynk, and ThingSpeak credentials there.
 *   3. Install the libraries listed in ../libraries.txt via Library Manager.
 */

#include "secrets.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- DHT11 ----------------
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- Sensors / actuators ----------------
#define SOIL_MOISTURE_PIN A0
#define VIBRATION_PIN     D5
#define RELAY_PIN         D6   // Drives the alert / vibration motor
#define BUZZER_PIN        D7
#define RAIN_SENSOR_PIN   D3   // Digital output from rain sensor

// LED pins for soil moisture level indication
#define GREEN_LED_PIN  D8   // High moisture
#define YELLOW_LED_PIN D9   // Medium moisture
#define RED_LED_PIN    D10  // Low moisture

// ---------------- Thresholds ----------------
const int soilDryValue    = 1023;
const int soilWetValue    = 100;
const int lowThreshold    = 30;
const int mediumThreshold = 60;

// ---------------- Cloud clients ----------------
char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;
unsigned long channelID = THINGSPEAK_CHANNEL_ID;
const char* apiKey      = THINGSPEAK_API_KEY;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);

  // Pin modes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP); // Pull-up prevents a floating pin

  // Initial states
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF
  digitalWrite(BUZZER_PIN, LOW);  // Buzzer OFF
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  ThingSpeak.begin(client);

  Wire.begin(D2, D1); // OLED I2C: SDA -> D2, SCL -> D1
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  Blynk.run();

  // Read sensor values
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoistureRaw = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(soilMoistureRaw, soilWetValue, soilDryValue, 100, 0);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  int vibration = digitalRead(VIBRATION_PIN);
  int rainStatus = digitalRead(RAIN_SENSOR_PIN); // LOW = rain detected

  // Send to ThingSpeak (Field 1-5: Temp, Humidity, Soil Moisture, Vibration, Rain)
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, soilMoisturePercent);
  ThingSpeak.setField(4, vibration);
  ThingSpeak.setField(5, rainStatus);
  int response = ThingSpeak.writeFields(channelID, apiKey);
  if (response == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("ThingSpeak error: " + String(response));
  }

  // OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temp); display.println(" C");
  display.print("Humidity: "); display.print(humidity); display.println(" %");
  display.print("Soil: "); display.print(soilMoisturePercent); display.println(" %");
  display.print("Vibration: "); display.println(vibration ? "YES" : "NO");
  display.print("Rain: "); display.println(rainStatus == LOW ? "YES" : "NO");
  display.display();

  // Vibration alert (buzzer)
  if (vibration == HIGH) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Vibration detected!");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // LED soil indication
  if (soilMoisturePercent < lowThreshold) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    Serial.println("Soil too dry.");
  } else if (soilMoisturePercent < mediumThreshold) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    Serial.println("Soil is moderate.");
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    Serial.println("Soil is good.");
  }

  // Rain detection message
  if (rainStatus == LOW) {
    Serial.println("Rain detected!");
  } else {
    Serial.println("No rain.");
  }

  delay(1000); // Sensor reading interval
}

// Blynk virtual button (V1): manual control of the relay / alert motor
BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    digitalWrite(RELAY_PIN, LOW);  // Relay ON
    Serial.println("Alert motor ON");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Relay OFF
    Serial.println("Alert motor OFF");
  }
}
