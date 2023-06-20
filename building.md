# Building You Own Wireless Controller

## Get the Parts

Some of the parts will take a while to manufacture and ship. For example, if you order the PCB from overseas, you might have to wait ~3 weeks for that. If you order electronic components from overseas, again you might have to wait several weeks for shipping, so plan accordingly.

### 1. Components

See the [BOM for more details and some example-component links](BOM.md), but you will need:

* Base N64 controller you plan to modify.
   * I'm not sure how standardized N64 controller internals are, so it is probably a good idea to confirm the measurements of your controller match up with the spacing of the PCB mounting/securing holes.
   * OEM controllers use a 6 pin connector compared to the cheaper alternatives which use a 4 pin. Additional steps are required to configure the software as well as modifications to the PCB for an OEM 6 pin controller.
* ESP32 microcontroller
* AAA terminals
* LiPo battery
* LiPo battery charging board
* PH/JST connectors and cables (somewhat optional - you can directly solder wires to everything instead, but I prefer removable plugs/connectors)

Optional components:

* LED
* Resistors

### 2. Print the PCB

You need to have a PCB manufacturer print out the custom PCB used in this project. The files needed to print this custom PCB are in the `pcb/` directory in this repo.

See [PCB ordering](pcb/ordering.md) for more details.

### 3. Print the battery pak pieces

You can print the battery pak pieces yourself with your (or a friend's) 3d printer and save some money, or you could have a 3d-print shop print them for you. The `.stl` (and `.step`) files for printing are in subdirectories under the `battery_pak/` directory in this repo.

See [battery pak readme](battery_pak/README.md) for more details.

Alternatively, you could skip using a battery pak and directly plug/wire a battery to the power-in on the microcontroller.

----

## Build It

### 1. Program the microcontroller

You will need to flash the software for this project on your ESP32 microcontroller. After `ESP-IDF` is setup on your PC, you can connect the microcontroller to your PC via USB and build and flash the software.

If you are using and OEM controller the software must be updated to enable SIXPIN mode before building:

```
input_poll.h - #define SIXPIN_ENABLED  1
```

See [software readme](software/README.md) for more details.

### 2. Assemble the components

Components:

<img src=images/components.jpg width=720>

1. **ESP32**: Solder the pin headers to the ESP32 and solder that to the PCB. Take care to line up the ESP32 pins correctly with the PCB labels, and note that the ESP will be on the "button" side of the PCB.

2. **Test**: At this point, I do a quick test to make sure the ESP32 is registering button presses as expected (power it up and either connect to a PC/phone with some gamepad testing application or connect the ESP32 via USB to your PC and monitor it in debug mode).

3. **2-pin headers**: Solder the 2-pin JST headers/connectors for the `R trig`, `L trig` and `Z btn`. I solder them such that the connector opening is facing upward, but either direction should work.

   <img src=images/2pin_header.jpg width=480>

4. **2-pin cables**: Solder one 2-pin JST cable to each external board: right trigger, left trigger, and Z board (you should cut down the length of the cable before hand so there isn't a ton of extra slack taking up space). Each board has two pads to solder to, and it doesn't matter which wire is soldered to which pad. You will need to cut or desolder the existing wires running to these boards.

5. **4-pin/6-pin header**: Solder the 4-pin PH header/connector for non OEM controllers `Analog`. I solder this so the connector opening is facing downward.

   <img src=images/4pin_header.jpg width=360>

   If using the OEM controller, the 6-pin connector needs modifying to work with the current PCB until this is updated. Carefully bend two of the pins up so the header/connector fits into the 4 pin position on the PCB. Then two additional wires are required to be soldered from these bent pins to the correct ESP32 Pins on the PCB (6 -> Pin 13,5 -> Pin 35) as per the picture below:

   <img src=images/6pin_header.jpg width=360>

   Additionally, for the 6 Pin connectors the wires from the joystick need swapping around in the 6pin header to match the PCB (a small screwdriver can be used to lift the holding clips and release the wires from the connector):

   <img src=images/6pin_pcb.png width=360>

   * Pin 1 - X
   * Pin 2 - Power 
   * Pin 3 - Ground
   * Pin 4 - XQ
   * Pin 5 - Y
   * Pin 6 - YQ

   These should be reordered to:
  * Pin 1 - power
  * Pin 2 - YQ
  * Pin 3 - Ground
  * Pin 4 - X
  * Pin 5 - Y
  * Pin 6 - XQ
   
   (More information can be found here: https://dpedu.io/article/2015-03-11/nintendo-64-joystick-pinout-arduino)

   <img src=images/6pin_header_after.jpg width=360>

6. **4-pin cables**: Solder the 4-pin PH cable to the analog joystick board (you should cut down the length of the cable before hand so there isn't a ton of extra slack taking up space). You will need to cut or desolder the existing wires running to this board - note which wires are labeled as V, X, G, and Y before removing them. Take care to solder the appropriate PH cable to the appropriate pad on the joystick board; on my board the pins were X, Gnd, Y, and V+ (from top to bottom) but yours may be different.

7. **Test**: At this point, I do another quick test to make sure the ESP32 is registering analog joystick data (same as `#2` above).

### 3. *Optional* Assembly

These steps are completely optional. You can do one or all of them if you want.

1. **LED**: Solder the LED and its current-limiting resistor (`R1`) to the PCB. This is only needed if you want visual feedback from the controller (e.g. see when it is powered up).

2. **`Start` button**: Solder the `start` button pull-down resistor (`R2`). This is only needed if you have issues with the `start` button getting "stuck" pressed or erronously registering button presses.


### 4. Assemble the battery pak

#### Battery pak

Components (ignore the fact that the charge board pictured below is a USB-C board, which doesn't fit in the current battery pak):

<img src=images/battery_components.jpg width=720>

1. **Battery**: Solder the battery to the charging board (battery positive to `B+` and battery negative to `B-`).

2. **Terminals**: **After this step, be very careful not to let the terminals touch each other. If they touch, it will short the battery and could cause a fire. I would recommend putting electrical tape over the flat terminal for now.**

   Solder one set of the flat+spring terminals to the charging board output. Solder the flat terminal to the negative board output `OUT-` and solder the spring terminal to the positive board output `OUT+`.

3. **Assembly**: Put the charging board in the cutout in the battery pak, place the battery over that, and place the flat and spring terminals in the appropriate cutouts in the battery pak. The spring terminal should be at the top-left, when the pak-clip cutout is at the top and facing you.

   Before putting the battery over the charging board, the final assembly should look something like this:
   
   <img src=images/battery_pak_internals.jpg width=720>

4. (Remove electrical tap covering the flat terminal and) Clip the two pieces of the battery pak together.

#### Controller-side terminals

1. **Terminal wiring**: With one of the PH cables plugged into the ESP32, you should be able to tell which wire corresponds to `+` and which corresponds to `-` based on markings on the ESP32 board. Solder the `+` wire to another flat terminal and solder the `-` wire to another spring terminal.

2. **Assembly**: Super glue the terminals to the terminal-block piece so they line up with the opposite terminal shape from the battery pak (i.e. spring terminal should plug into flat terminal, and vice versa):

   <img src=images/battery_terminals.jpg width=720>

### 5. Reassemble the N64 controller

1. Install the PCB in the front-piece of the controller shell.

2. Plug in the external JST/PH cables into the appropriate PCB-connector.

3. Reinstall the external boards/mounts (triggers, joystick) in the appropriate spots.

   Reassembled front-piece of the controller shell should look something like this:

   <img src=images/installed_pcb.jpg width=720>

4. Install the terminal-block in the expansion-pak-port of the back-piece of the controller shell:

   <img src=images/battery_terminals.jpg width=720>

5. Mount the back-piece of the controller shell on the front-piece - make sure to reinstall the left and right trigger buttons when you do this.



