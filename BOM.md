# Wireless controller bill of materials
Note: as mentioned elsewhere, you *could* skip having a battery pak and wire a battery directly to the controller, but you'll still need to somehow secure the battery (e.g. electrical tape). *I also assume you already have soldering equipment and some ~30 gauge wire to wire up components.*

*Also note: the links below are just to serve as a reference. You can use equivalent items that better suit your needs, like bulk packs, cheaper or higher capacity battery, etc.*

The total price of building 1 controller is around or slightly above a similar off-the-shelf wireless controller at ~$60, but you could produce ~4 of them for closer to ~$35 per controller (since shipping doesn't scale up with more items purchased and some things like battery terminals and PCBs really only come in multi-packs).

## Controller parts

Component | Quantity | Link | Price | Note
--- | --- | --- | --- | ---
N64 USB controller | 1 | [Link](https://www.amazon.com/Classic-Controller-iNNEXT-Joystick-Raspberry/dp/B0744DW55H) | $12.00 | 
--- | --- | --- | --- | ---
ESP32 | 1 | [Link](https://www.aliexpress.com/item/32864722159.html) | $3.31 | 
Mini boost converter board | 1 | [Link](https://www.aliexpress.com/item/4000322419351.html) | $0.65 | 
Shipping | --- | --- | ~$2.50 | 
--- | --- | --- | --- | ---
PCB | 1 | [Link**](https://github.com/sfraint/wireless-n64-controller/blob/main/pcb/ordering.md) | ~$15 | **See link for details
--- | --- | --- | --- | ---
**Total** | --- | --- | ~$33.46 | 

You can also optionally buy JST headers to solder on to the board, to make the off-board connections more modular and cleaner.

## Battery pak parts

Component | Quantity | Link | Price | Note
--- | --- | --- | --- | ---
AAA terminals | 1 | [Link](https://www.amazon.com/12Pairs-Battery-Negative-Positive-Conversion/dp/B08Y5HVKLH) | $8.00 | May be able to find a smaller, cheaper pack somewhere
--- | --- | --- | --- | ---
Lithium polymer battery | 1 | [Link](https://www.aliexpress.com/item/1005003258173852.html) | $6.00 | You can also use a smaller/cheaper battery. Just make sure it fits in the 6mm x 40mm x 50mm battery pak cutout.
--- | --- | --- | --- | ---
Lithium polymer battery charging board | 1 | [Link](https://www.aliexpress.com/item/4000522397541.html) | $2.00 | 
--- | --- | --- | --- | ---
Battery pak top | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$2 | Linked "thing" is all three of the required pak parts
Battery pak bottom | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$3 | Linked "thing" is all three of the required pak parts
Battery pak controller-terminal | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$2 | Linked "thing" is all three of the required pak parts
Shipping | --- | --- | ~$7 | You'll probably want to have all three pak parts 3d printed from the same supplier to save on shipping
--- | --- | --- | --- | ---
**Total** | --- | --- | ~$30 | 


