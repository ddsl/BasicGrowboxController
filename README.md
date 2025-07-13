# 🌱 Basic Growbox Controller (for Chili Peppers)

## 📌 Description
This project is a basic growbox controller designed for the **Wemos D1 Mini** board. The device automates lighting, irrigation, and temperature monitoring for growing chili peppers. The project includes the following features:

- **Lighting Control**: Sunrise and sunset simulation using red and blue LEDs. Time is synchronized via Wi-Fi and NTP.
- **Irrigation Control**: Adjustable pump interval and duration for hydroponics (hybrid DWC and Kratky methods).
- **Temperature Monitoring**: Current temperature indication using a DS18B20 sensor.

The project also provides a web interface for device management and data retrieval in JSON format.

---

## 🔧 Key Features

### 1. **Lighting**
- Control of red and blue LEDs:
  - Red light is primary, blue is secondary.
  - The difference between red and blue LED operation times is set by the `RedBlueDiffHours` constant:
    - If `RedBlueDiffHours = 2`, red LEDs operate 1 hour longer: 1 hour of only red light in the morning, then 12 hours of both red and blue light, and 1 hour of only red light in the evening.
    - Recommended full daylight duration for chili peppers: 14 hours. Try setting `RedBlueDiffHours = 2` or `4`.

### 2. **Irrigation**
- Control of a DC 3-5V & 100-200mA water pump:
  - Adjustable irrigation interval and duration.
  - Recommendation: Irrigate every 1 hour for 1-5 minutes (suitable for hybrid DWC and Kratky methods).

### 3. **Temperature**
- Support for the DS18B20 digital temperature sensor.
- Current temperature is available via JSON request.

---

## 🛠️ Requirements

### Hardware:
- **Wemos D1 Mini** board.
- DC 3-5V 100-200mA water pump (my from Aliexpress).
- MOSFET (e.g., IRF520) for pump control.
- Two relays for controlling blue and red LEDs.
- DS18B20 digital temperature sensor.
- LEDs with drivers (red and blue LEDs).

### Software:
- Arduino IDE or PlatformIO.
- Libraries:
  - `ESP8266WiFi` — for Wi-Fi connection.
  - `NTPClient` — for time synchronization.
  - `ESP8266WebServer` — for creating a web server.
  - `FS` and `ESP8266FtpServer` — for working with SPIFFS via FTP.
  - `OneWire` and `DallasTemperature` — for working with the DS18B20 sensor.
  - `Ticker` — for task scheduling.
  - `Time` — for time management.

---

## 🚀 Setup and Usage

1. **Wi-Fi Configuration**:
   - Open the `WifiCreds.h` file and specify your Wi-Fi SSID and password:
     ```cpp
     #define SSID "your_wifi_ssid"
     #define PASSWORD "your_wifi_password"
     ```

3. **Code Upload**:
   - Open the project in Arduino IDE.
   - Select the **Wemos D1 Mini** board and the corresponding port.
   - Upload the code to the board.
   - Connect via ftp to the device (ftp://growcon.local; default creds: *grow*/*grow*; could set in code) and upload ```index.html``` & ```script.js``` files for dashboard

4. **Using the Web Interface**:
   - After uploading the code, the device will create an access point or connect to the specified Wi-Fi network.
   - Open a browser and enter "growcon.local" hostname (if you want set another name in code where MDNS responder is used).
   - Control lighting, irrigation, and retrieve temperature data.

---

## 📊 Example Operation

### Automatic Mode:
- Irrigation: Every 1 hour for 3 minutes.
- Lighting:
  - 06:00 – Red LEDs turn on.
  - 07:00 – Blue LEDs turn on.
  - 19:00 – Blue LEDs turn off.
  - 20:00 – Red LEDs turn off.

### Manual Mode:
- Set mode to MANUAL via the web interface
- Control the pump and lighting via the web interface.

---

## 📝 Notes
- The built-in LED on the Wemos D1 Mini blinks to indicate activity.
- It is recommended to use POST requests instead of GET for pin state changes to adhere to standards.
- This is my pretty old project (2019). The project was loaded as a one commit project and will not be maintained
- LLM helped me with this readme though

---

## 📜 License
MIT License
