## Software

This subdirectory contains source code for the software that runs on the ESP32, to track button presses and communicate with the connected bluetooth client.

Note: This uses a slightly modified version of [`ESP32-BLE-Gamepad`](https://github.com/lemmingDev/ESP32-BLE-Gamepad) which is MIT licensed, see the included copy of the BLE gamepad license in the `ESP32-BLE-Gamepad/` source directory.


### Setup
This project requires `ESP-IDF` and `NimBLE-cpp`, follow the standard [ESP-IDF setup instructions](https://github.com/espressif/esp-idf) and [`NimBLE-cpp` setup instructions](https://github.com/h2zero/esp-nimble-cpp).

#### Initial Configuration
This setup needs to be done once per project:

1. Set correct target, e.g. `idf.py set-target esp32`

2. Configure project `idf.py menuconfig`

   In here, make sure to set the following settings:
   * Component config
     * Bluetooth
       * Bluetooth - `*` (enabled, required for NimBLE)
     * Bluetooth Host - `NimBLE - BLE only`
   * FreeRTOS
     * Tick rate (Hz) - `1000` (required to allow <10 ms sleep, for fast polling)


#### Building and Running

Don't forget to setup env vars once per shell: `. $HOME/esp/esp-idf/export.sh`

Then:

1. Build: `idf.py build`

2. Flash: `idf.py -p /dev/ttyUSB0 -b 115200 flash` (or `-p /dev/ttyACM0`)

3. (Optionally) Monitor: `idf.py -p /dev/ttyUSB0 monitor` (when you're done, press `Ctrl` + `]` to end the monitor session)

