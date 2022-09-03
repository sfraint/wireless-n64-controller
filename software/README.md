## Software

This subdirectory contains source code for the software that runs on the ESP32, to track button presses and communicate with the connected bluetooth client.

Note: This uses a slightly modified version of [`ESP32-BLE-Gamepad`](https://github.com/lemmingDev/ESP32-BLE-Gamepad) which is MIT licensed, see the included copy of the BLE gamepad license in the `ESP32-BLE-Gamepad/` source directory.


### Setup

This project requires `ESP-IDF` (works with version 4.4) and `NimBLE-cpp` (currently requires an older version, 1.3)

1. Follow the standard [ESP-IDF setup instructions](https://github.com/espressif/esp-idf)

2. Clone this project if you haven't already, e.g. `git clone https://github.com/sfraint/wireless-n64-controller.git`

3. Setup NimBLE 1.3.1

   - Create a `components` directory within the `software` directory in this project

   - Download [the NimBLE 1.3.1 source release](https://github.com/h2zero/esp-nimble-cpp/archive/refs/tags/1.3.1.zip) and extract it in the `components` directory

   - Once this is done, the `software/components/esp-nimble-cpp-1.3.1/` directory should exist in your project


#### Building and Running

Don't forget to setup env vars once per shell: `. $HOME/esp/esp-idf/export.sh`

Then, from the `software` directory in this project:

1. Build: `idf.py build`

2. Flash: `idf.py -p /dev/ttyUSB0 -b 115200 flash` (or `-p /dev/ttyACM0` or `-p COM4` or whatever port your ESP32 is connected to)

3. (Optionally) Monitor: `idf.py -p /dev/ttyUSB0 monitor` (when you're done, press `Ctrl` + `]` to end the monitor session)

#### Re-Configuration
You shouldn't need to complete idf configuration if you're using an ESP32, as these settings are recorded in the provided `sdkconfig`. If you do need to manually configure the project, then complete the following from the `software` directory in the project:

1. Set correct target, e.g. `idf.py set-target esp32`

2. Configure project `idf.py menuconfig`

   In here, make sure to set the following settings:
   * Component config
     * Bluetooth
       * Bluetooth - `*` (enabled, required for NimBLE)
     * Bluetooth Host - `NimBLE - BLE only`
   * FreeRTOS
     * Tick rate (Hz) - `1000` (required to allow <10 ms sleep, for fast polling)
