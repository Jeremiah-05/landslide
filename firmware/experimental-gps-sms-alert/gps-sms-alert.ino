/*
 * Landslide Detection System - Experimental GPS + SMS Alert Variant
 * -------------------------------------------------------------------
 * NOT the version described in the submitted project report / poster.
 * This is a separate exploratory build that swaps the rain sensor and
 * LED/buzzer alerts for GPS location tracking and SMS alerts over a
 * GSM module. Kept here as a documented future-work branch rather than
 * being silently merged into the main sketch.
 *
 * SETUP:
 *   1. Copy "secrets.h.example" to "secrets.h" in this same folder.
 *   2. Fill in your own Wi-Fi, Blynk, ThingSpeak, and alert phone number.
 *   3. Install the libraries listed in ../libraries.txt via Library Manager.
 */

#include "secrets.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN D4
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN A0
#define VIBRATION_PIN D5
#define RELAY_PIN D6  // Drives the alert motor

SoftwareSerial gsmSerial(D2, D3);
SoftwareSerial gpsSerial(D1, D2);
TinyGPSPlus gps;

char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
unsigned long channelID = THINGSPEAK_CHANNEL_ID;
const char* apiKey = THINGSPEAK_API_KEY;

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);

const int threshold_value = 500;

String getGPSLocation();
void sendSMS(String message);

bool relayState = LOW;

void setup() {
  Serial.begin(115200);
  gsmSerial.begin(9600);
  gpsSerial.begin(9600);

  pinMode(VIBRATION_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  ThingSpeak.begin(client);
  Blynk.begin(auth, ssid, password);

  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1");
  delay(1000);

  Wire.begin(D2, D1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  Blynk.run();

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  int vibration = digitalRead(VIBRATION_PIN);

  String locationData = getGPSLocation();
  float latitude = gps.location.lat();
  float longitude = gps.location.lng();

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, soilMoisture);
  ThingSpeak.setField(4, vibration);
  ThingSpeak.setField(5, latitude);
  ThingSpeak.setField(6, longitude);

  int response = ThingSpeak.writeFields(channelID, apiKey);
  if (response == 200) {
    Serial.println("Data successfully sent to ThingSpeak");
  } else {
    Serial.println("Failed to send data to ThingSpeak. HTTP error code: " + String(response));
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Temp: "); display.print(temp); display.println(" C");
  display.print("Hum: "); display.print(humidity); display.println(" %");
  display.print("Soil: "); display.print(soilMoisture); display.println(" %");
  display.print("Vib: "); display.println(vibration == HIGH ? "Detected" : "None");

  display.setCursor(0, 50);
  display.print("GPS: ");
  display.print(latitude, 4); display.print(", ");
  display.print(longitude, 4);

  display.display();

  if (vibration == HIGH || soilMoisture > threshold_value) {
    digitalWrite(RELAY_PIN, HIGH);
    sendSMS("ALERT: Landslide risk detected!");
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(2000);
}

String getGPSLocation() {
  String gpsData = "";
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        gpsData = "Lat: " + String(gps.location.lat(), 6) + ", Lon: " + String(gps.location.lng(), 6);
      } else {
        gpsData = "Location not available";
      }
    }
  }
  return gpsData;
}

void sendSMS(String message) {
  gsmSerial.println("AT+CMGS=\"" + String(ALERT_PHONE_NUMBER) + "\"");
  delay(1000);
  gsmSerial.print(message);
  delay(1000);
  gsmSerial.write(26); // Ctrl+Z sends the message
  delay(1000);
}

BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = HIGH;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    relayState = LOW;
  }
}
