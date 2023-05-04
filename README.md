# Prerequisites
## Arduino IDE
Install **version 1 (legacy)** of Arduino IDE from [here](https://www.arduino.cc/en/software)
## Install  ESP32 boards for Arduino
* Open `File`&rarr;`Preferences`
* Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to `Additional Boards Manager URLs` and click `OK`
* Open `Tools->Board->BoardsManager...`
* search for `esp32` and install
## ESP32 sketch data upload tool
Install [arduino-esp32fs-plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin) as described in the repository
## Libraries
### Libraries not listed in Arduino registry:
Download following `.zip` archives:
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/archive/refs/heads/master.zip)
* [AsyncTCP](https://github.com/me-no-dev/AsyncTCP/archive/refs/heads/master.zip)

After opening `NixieClockFirmware.ino` execute `Sketch`&rarr;`Include Library`&rarr;`Add .ZIP Library...` for all the downloaded `.zip` files

###  Libraries listed in Arduino registry:
Run `Tools`&rarr;`ESP32 Sketch Data Upload` and install:
* `ArduinoJson` by Benoît Blanchon
* `DS3231` by Andrew Wickert


# Build and Run
Before build, select correct board: `Tools`&rarr;`Board`&rarr;`ESP32 Arduino`&rarr;`ESP32DevModule`

After flashing firmware to the board run `Tools`&rarr;`ESP32 Sketch Data Upload` to upload WEB interface data to the board

# Changing brightness settings

When the clock is connected to your local WiFi network you can change automatic brightness controll settings by editing [brightness.json](data/brightness.json) file.

After you are done changing the settings an the file is saved run:

```bash
curl -X POST -H "Content-Type: application/json" -d @data/brightness.json NIXIE_IP/rest/bright
```

where you replace `NIXIE_IP` by the IP address of your clock