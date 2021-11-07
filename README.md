# wireless-n64-controller
This project and its documentation is a Work-In-Progress. I'm still working out kinks in the design files and prototypes, and I'm still working on writing everything down.

The idea is to publish the design files and software I used to upgrade a cheap, wired N64 controller to a wireless, Bluetooth controller; hopefully these published files can assist others in completing similar projects.

<img src=images/controller_prototype.jpg width=720>

## Motivation
When this project was started, no original-form-factor N64 wireless controllers were available for purchase. There were some two-handle controllers, inspired by N64 original controllers, but none with the original three-handle setup.

The main goal of this project was to upgrade an existing, cheap three-handle N64 wired controller to use bluetooth, so it could be used with a PC and/or Raspberry Pi.

## Building your own
Detailed building instructions **TODO**.

*Note: this project requires soldering and some basic understanding of electriconics and circuits. I also assume you already have soldering equipment and some ~30 gauge wire to wire up components.*

The following is a high-level list of steps to build your own wireless controller:

1. Purchase the base parts. See the [BOM for more details and links](https://github.com/sfraint/wireless-n64-controller/blob/main/BOM.md).
  1. Base N64 controller you plan to modify. I'm not sure how standardized N64 controller internals are, so it is probably a good idea to confirm the measurements of your controller match up with the spacing of the PCB mounting/securing holes.
  2. ESP32 microcontroller
  3. 5V regulator
  4. AAA terminals
  5. LiPo battery
  6. LiPo battery charging board

2. Print the PCB.

3. Print the battery pak pieces. Alternatively, you could skip using a battery pak and directly wire a battery to the power-in on the PCB or microcontroller.

4. Program the microcontroller.

5. Solder the microcontroller and aux boards/wires to the PCB.

6. Assemble the N64 controller.

7. Have fun! (optional)

## Debugging
**Note: make sure you disconnect the battery before plugging a USB cable into the microcontroller. If you do not do this, you could permanently damage the controller or worse, whatever is on the other end of the USB cable, e.g. your PC.**

At any point, you can remove the PCB + microcontroller from the N64 controller and monitor or reprogram the microcontroller. **TODO** add debugging details.


## Components

### Microcontroller
An [ESP32](https://en.wikipedia.org/wiki/ESP32) microcontroller is used to:

1. read the raw button presses from the button hardware in the controller

2. convert that into a USB HID report

3. send the report via Bluetooth LE to a connected client, e.g. PC

<img src=images/esp32_prototype.jpg width=720>

It is also capable of a few other things like: tracking battery level, resetting Bluetooth connections, etc.

### PCB
A custom PCB was built to replace the existing controller PCB, mainly to a) make space for the ESP32 dev board I used and b) reduce the amount of soldering required.

<img src=images/pcb.png width=720>

<img src=images/controller_internals.jpg width=720>

### Battery Pak
A 3d-printed battery pack that fits into the controller "pak" port was used to supply power to the controller, since it can no longer pull power from the controller cable.

This "pak" fits a small lithium polymer battery as well as charging circuitry, and connects to the controller via typical AAA spring-terminals. The "pak" was designed to fit a 6mm x 40mm x 50mm battery, but could of course fit smaller batteries with adequate padding or adhesives.

<img src=images/battery_pak.png width=720>

## Performance

Rough performance numbers for the initial prototype controller are:

1. battery lifetime - using a ~1500 mAh lithium polymer battery, I expect the controller to last about ~15 hours of continuous use; I'm less sure about low-power mode lifetime, but probably a few weeks (longer if you just detach the battery).

2. controller latency - over 100 sequential button presses had a latency of between 2 and 13 ms
  <img src=images/button_press_latency.png width=720>



