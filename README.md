# wireless-n64-controller
This project and its documentation is a Work-In-Progress. I'm still working on writing everything down and working out kinks in the design files and prototypes.

**The idea is to publish the design files and software used to upgrade an OEM or cheap, wired N64 controller to a wireless, Bluetooth controller**. Hopefully these published files can assist others in completing similar projects.

<img src=images/controller_prototype.jpg width=1024>

## Motivation
When this project was started, no original-form-factor bluetooth N64 wireless controllers were available for purchase. There were some two-handle controllers, inspired by N64 original controllers, but none with the original three-handle setup.

The main goal of this project was to upgrade an existing OEM or cheap three-handle N64 wired controller to use bluetooth, so it could be used with a PC and/or Raspberry Pi.

## Operation

Using the controller is pretty straightforward:

1. Insert a charged battery pak into the controller to turn it on. Note: if the LED is installed, it should light up once the controller is powered on.

2. After a few seconds, pair the controller to a client machine (e.g. phone or PC). The controller should show up as `ESP32 BLE Gamepad` in bluetooth device managers. Note: if the LED is installed, it will start blinking when the controller automatically enters pairing mode after power-up.

3. If the LED is installed, it will blink slowly when the controller is paired with a client machine.

### Analog Calibration

If you press and hold the `Start` button before the controller enters pairing mode, it will enter analog calibration mode, indicated by a rapidly blinking LED.

Upon entering calibration mode, the initial position of the joystick is recorded as the new `center` position. During calibration mode, you should rotate the joystick around in circles, reaching the all the extremes (far up, right, down, left) to reset `min` and `max` values for both the X and Y axes.

Calibration mode lasts for a few seconds (~5 sec). Upon completion, it writes results to non-volatile storage if possible, then resumes normal startup routing - entering pairing mode.

### "Soft" Buttons

This wireless N64 controller has all the normal N64 buttons plus a few extra software buttons, triggered by special button combinations. These software buttons can be associated with special functions in some emulators, like triggering the menu function, save/load state, etc.

There are currently three "soft" buttons, all triggered by pressing the center of the d-pad (i.e. pressing all 4 directions at the same time) plus either `Start`, `A`, or `B`.

## Building your own
More detailed building instructions can be found [here](building.md).

*Note: this project requires soldering and some basic understanding of electriconics and circuits. I also assume you already have soldering equipment and some ~30 gauge wire to wire up components.*

### Steps

1. Purchase the base parts. See the [BOM for more details and links](BOM.md).
   * Base N64 controller you plan to modify. I'm not sure how standardized N64 controller internals are, so it is probably a good idea to confirm the measurements of your controller match up with the spacing of the PCB mounting/securing holes.
   * ESP32 microcontroller
   * AAA terminals
   * LiPo battery
   * LiPo battery charging board

2. Print the PCB.

3. Print the battery pak pieces. Alternatively, you could skip using a battery pak and directly wire a battery to the power-in on the microcontroller.

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

<img src=images/pcb_front.jpg width=720>

<img src=images/installed_pcb.jpg width=720>

### Battery Pak
A 3d-printed battery pack that fits into the controller "pak" port was used to supply power to the controller, since it can no longer pull power from the controller cable.

This "pak" fits a small lithium polymer battery as well as charging circuitry, and connects to the controller via typical AAA spring-terminals. The "pak" was designed to fit a 6mm x 40mm x 50mm battery, but could of course fit smaller batteries with adequate padding or adhesives.

<img src=images/battery_pak.png width=720>

## Performance

Rough performance numbers for the initial prototype controller are:

1. battery lifetime - using a ~1500 mAh lithium polymer battery, I expect the controller to last about ~15 hours of continuous use; I'm less sure about low-power mode lifetime, but probably a few weeks (longer if you just detach the battery).

2. controller latency - over 100 sequential button presses had a latency of between 2 and 13 ms (plus an additional ~2 ms for current software's polling interval - can reduce this using interrupts instead of polling, though chances are analog/jostick inputs will always suffer this extra latency)

   To measure approximate latency, I setup a second ESP32 which was:
   
   * a BLE client subscribed to the gamepad reports

   * running a GPIO interrupt to detect the button press

   Then, the second ESP32 recorded the time the button-interrupt was received as well as the time the BLE button-press-report was received. Got the approximate latency by subtracting the interrupt time from the BLE report time.
  <img src=images/button_press_latency.png width=720>



