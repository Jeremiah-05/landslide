# Hardware Components

As built and documented in `docs/Project_Report.pdf` and
`docs/Circuit_Block_Diagram.png`.

| Component | Role |
|---|---|
| ESP8266 NodeMCU | Main microcontroller — Wi-Fi, sensor reads, cloud/app comms |
| DHT11 | Temperature and humidity |
| Soil moisture sensor (analog) | Soil saturation, mapped to 0-100% |
| Vibration sensor (digital) | Ground movement detection |
| Raindrop / rain sensor (digital) | Real-time rainfall detection |
| SSD1306 OLED (128x64, I2C) | On-site live readout |
| Relay module | Switches the alert motor |
| DC vibration motor(s) | Physical on-site alert (manual via Blynk button, or automatic depending on variant) |
| Buzzer | Audible alert on vibration detection |
| Red / Yellow / Green LEDs | Soil moisture status (dry / moderate / wet) |
| 9V battery | Portable power for the prototype |

## Wiring (from `Circuit_Block_Diagram.png`)

- OLED (I2C) — SDA to D2, SCL to D1
- DHT11 — data pin to D4
- Soil moisture sensor — analog output to A0
- Vibration sensor — digital output to D5
- Rain sensor — digital output to D3
- Relay — control pin to D6
- Buzzer — D7
- LEDs — Green D8, Yellow D9, Red D10

## Notes

- The soil moisture LED thresholds are firmware-defined (below 30% = dry/red,
  30-60% = moderate/yellow, above 60% = wet/green) — see `lowThreshold` and
  `mediumThreshold` in `firmware/LandslideDetection/LandslideDetection.ino`.
- The rain sensor's digital threshold is set via the small potentiometer on
  the sensor board itself, not in firmware.
