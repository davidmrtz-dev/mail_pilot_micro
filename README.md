# Mail Pilot – ESP8266 Firmware

This firmware is part of the **Mail Pilot** project, created for the [Postmark Challenge](https://dev.to/challenges/postmark). It enables home automation devices to be controlled remotely via email-triggered commands.

## 📦 Overview

The ESP8266 continuously polls a Rails API for the latest command and executes it (e.g., turn on/off room lights, open/close door). Once executed, it reports the result back to the server.

This MVP implementation requires no authentication and is designed to be kept as simple and hackable as possible.

## ⚙️ Hardware

- **Board**: NodeMCU ESP8266 (ESP-12E)
- **Display**: OLED 128x64 (SSD1306, I2C)
- **Servo**: Generic SG90
- **Relays**: 4 digital pins control power to "ROOM_1" through "ROOM_4"
- **Network**: Wi-Fi connection

## 🔌 Wiring

| Pin   | Device           |
|-------|------------------|
| D0    | ROOM_4 relay     |
| D4    | ROOM_1 relay     |
| D5    | Servo motor      |
| D6    | ROOM_2 relay     |
| D7    | ROOM_3 relay     |
| I2C   | OLED display     |

## 📡 Command Flow

1. The ESP8266 polls `GET /device/commands/latest` every 3 seconds.
2. If a new command is found (with an `id`), it executes it.
3. It then PATCHes either:
   - `/device/commands/:id/complete`
   - `/device/commands/:id/failed`

## 🛠️ Setup Instructions

### 1. Install Libraries

Ensure the following libraries are installed via Arduino Library Manager:

- `ESP8266WiFi`
- `ESP8266HTTPClient`
- `ArduinoJson`
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `Servo`

### 2. Board Configuration

In the Arduino IDE:

- **Board**: `NodeMCU 1.0 (ESP-12E Module)`
- **Flash Size**: `4MB (FS: 1MB OTA:~1019KB)`
- **Upload Speed**: `115200`
- **Core**: Use `esp8266` core version >= 3.x

### 3. Update Credentials

In the `mail_pilot_2.ino` file:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* API_BASE = "http://your.ngrok.io"; // Or IP if on local network
```

## 💬 Supported Commands

| Command      | Description           |
| ------------ | --------------------- |
| ROOM\_1\_ON  | Turns ON relay 1      |
| ROOM\_1\_OFF | Turns OFF relay 1     |
| ROOM\_2\_ON  | Turns ON relay 2      |
| ROOM\_2\_OFF | Turns OFF relay 2     |
| ROOM\_3\_ON  | Turns ON relay 3      |
| ROOM\_3\_OFF | Turns OFF relay 3     |
| ROOM\_4\_ON  | Turns ON relay 4      |
| ROOM\_4\_OFF | Turns OFF relay 4     |
| DOOR\_OPEN   | Rotates servo to 180° |
| DOOR\_CLOSE  | Rotates servo to 0°   |

## 🔍 Display

Messages scroll across the OLED screen to indicate the current command status, such as:

- `ROOM 1 ON`
- `ROOM 3 OFF`
- `DOOR OPENED`
- `UNKNOWN COMMAND`

This visual feedback makes debugging and user interaction more intuitive.

## 🙏 Acknowledgments

Built with 💡 creativity, ☕ caffeine, and a strong desire to automate the world—one email at a time.

Inspired by the [Postmark Challenge](https://dev.to/challenges/postmark).
