# Verification
- Target: ESP32-WROOM-32 / esp32:esp32:esp32.
- Arduino ESP32 core 2.0.17; IRremote 4.4.1; ESP32 BLE Keyboard 0.3.2-beta.
- Actual Arduino CLI compile succeeded: flash 1,145,797 bytes (87%); static RAM 40,804 bytes (12%).
- Host C++ test passed: multidigit input, 1000ms boundary, cancellation, zero-only suppression, four-digit limit, millis rollover.
- Not hardware-tested: IR receiver electrical compatibility and decoding, Windows pairing/reconnection, actual PowerPoint key handling, NVS persistence across board power cycles.
- Receiver pin order and voltage support must be confirmed from its model/markings before wiring.
- Standard keyboard behavior: keys affect the focused window; use English keyboard layout, and slideshow mode for numeric jumps.
