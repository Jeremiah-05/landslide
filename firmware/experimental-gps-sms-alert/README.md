# Experimental: GPS + SMS Alert Variant

**Status: exploratory / not part of the submitted project report or poster.**

This sketch is an alternate build that was developed alongside the main
firmware. Instead of a rain sensor and local buzzer/LED alerts, it adds:

- A GPS module (NEO-6M via `TinyGPS++`) to tag readings with location
- A GSM module for SMS alerts when soil moisture or vibration cross a
  threshold
- Automatic relay activation (no manual Blynk button needed)

It shares the same soil moisture, vibration, DHT11, OLED, ThingSpeak, and
Blynk foundation as the main sketch, but was not the version tested,
documented, or reported in `docs/Project_Report.pdf`.

Treat this as a "future work" branch — Chapter 6 of the report lists
expanded alerting (SMS/notifications to authorities) as a recommended
next step, and this is a first attempt at that. If you finish testing
and validating it, consider promoting it to a versioned release (e.g.
`v4.0`) with its own results section in the docs.
