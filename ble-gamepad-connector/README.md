# Example Bluetooth BLE Gamepad
This is the code to turn an ESP32 connected to a wired, USB N64 controller into a blutooth controller.

## Setup

### Initial Setup

This setup needs to be done once per project:

1. Set correct target, e.g. `idf.py set-target esp32`
2. Configure project `idf.py menuconfig`
  * In here, make sure to enable Bluetooth, NimbleCpp, set compiler optimizations to `O2`, etc.

### Building and Running
0. Setup env vars: `. $HOME/esp/esp-idf/export.sh`
1. Build: `idf.py build`
2. Flash: `idf.py -p /dev/ttyACM0 -b 115200 flash`
3. Monitor: `idf.py -p /dev/ttyACM0 monitor` (when you're done, press `Ctrl` + `]` to end the monitor session)

