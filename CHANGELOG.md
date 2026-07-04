# Changelog

All notable changes to this project are documented here.

## [v3.0] - Final submitted build (April 2025)
- Added raindrop sensor for real-time rainfall detection
- Added 3-LED soil moisture status indicator (dry / moderate / wet)
- Added buzzer alert on vibration detection
- ThingSpeak logging expanded to 5 fields (temp, humidity, soil %, vibration, rain)
- This is the version described in the project report and poster

## [v2.0] - IoT cloud integration
- Added ESP8266 Wi-Fi connectivity
- Added ThingSpeak cloud data logging
- Added Blynk app integration for remote relay control
- Added OLED live readout

## [v1.0] - Hardware prototype
- Initial sensor wiring: DHT11, soil moisture, vibration
- Relay-driven alert motor

## [Unreleased / experimental]
- GPS + GSM SMS alert variant (`firmware/experimental-gps-sms-alert/`) —
  built alongside the main firmware but not part of the submitted report;
  kept as a documented future-work branch
