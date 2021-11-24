# Wireless controller bill of materials
Note: as mentioned elsewhere, you *could* skip having a battery pak and wire a battery directly to the controller, but you'll still need to somehow secure the battery (e.g. electrical tape). *I also assume you already have soldering equipment and some ~30 gauge wire to wire up components.*

*Also note: the links below are just to serve as a reference. You can use equivalent items that better suit your needs, like bulk packs, cheaper or higher capacity battery, etc.*

----

**If you can print or build your own battery "pak", the total project price should be < $50 for a single controller or < $90 for 4.**

Otherwise the total price of building 1 controller is around or slightly above a similar off-the-shelf wireless controller at ~$60, but you could still produce ~4 of them for closer to $30 or $35 per controller (since many items only come in multi-packs, e.g. PCBs, battery terminals; and buying in "bulk" saves on shipping costs).

## Controller parts

Component | Quantity | Link | Price | Note
--- | --- | --- | --- | ---
N64 USB controller | 1 | [Link](https://www.amazon.com/Classic-Controller-iNNEXT-Joystick-Raspberry/dp/B0744DW55H) | $12.00 | 
--- | --- | --- | --- | ---
ESP32 Lolin32 Lite | 1 | [Link](https://www.aliexpress.com/item/4000038780903.html) | $2.67 | These are "discontinued" but I've found them readily avaialble and one of the most cost-effective uControllers for this project
Shipping | --- | --- | ~$1.50 | 
--- | --- | --- | --- | ---
PCB | 1 | [Link**](https://github.com/sfraint/wireless-n64-controller/blob/main/pcb/ordering.md) | ~$15 (for 5, shipped) | **See link for details
--- | --- | --- | --- | ---
**Total** | --- | --- | ~$31 | 

### Optional parts

You can also optionally buy JST+PH headers and cables to solder on to the boards (instead of soldering wires directly), to make the off-board connections more modular and cleaner (one set {below} is enough for several controllers). Note: you have quite a bit of flexibility here, e.g. could skip buying the 2-pin PH parts and just use 2-pin JST + 4-pin PH.

Component | Quantity | Link | Price | Note
--- | --- | --- | --- | ---
PH (2.0mm) 2-pin dual-sided cables | 1 | [Link](https://www.aliexpress.com/item/4001353071671.html) | $1.19 | 
PH (2.0mm) 2-pin right angle | 1 | [Link](https://www.aliexpress.com/item/1005003115054198.html) | $0.57 | 
JST (1.25mm) 2-pin dual-sided cables | 1 | [Link](https://www.aliexpress.com/item/4001353071671.html) | $1.23 | 
JST (1.25mm) 2-pin right angle | 1 | [Link](https://www.aliexpress.com/item/1005003115054198.html) | $0.62 | 
PH (2.0mm) 4-pin dual-sided cables | 1 | [Link](https://www.aliexpress.com/item/4001353071671.html) | $1.71 | 
PH (2.0mm) 4-pin right angle | 1 | [Link](https://www.aliexpress.com/item/1005003115054198.html) | $0.66 | 
Shipping | --- | --- | ~$3 | 
--- | --- | --- | --- | ---
**Total** | --- | --- | ~$9 | 


## Battery "pak" parts

*Note: if possible, you should print the `*`custom parts yourself, or have a friend with a 3d printer help.*

Component | Quantity | Link | Price | Note
--- | --- | --- | --- | ---
AAA terminals | 1 | [Link](https://www.aliexpress.com/item/32884289489.html) | ~$2.50 | May be able to find a smaller, cheaper pack - *also make sure your 3d printed controller-terminal block will fit the terminals you order*
--- | --- | --- | --- | ---
Lithium polymer battery | 1 | [Link](https://www.aliexpress.com/item/1005003258173852.html) | $6.00 | You can also use a smaller/cheaper battery. Just make sure it fits in the 6mm x 40mm x 50mm battery pak cutout.
--- | --- | --- | --- | ---
Lithium polymer battery charging board | 1 | [Link](https://www.aliexpress.com/item/4000522397541.html) | $2.00 | 
--- | --- | --- | --- | ---
`*`Battery pak top | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$2 | Linked "thing" is all three of the required pak parts
`*`Battery pak bottom | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$3 | Linked "thing" is all three of the required pak parts
`*`Battery pak controller-terminal | 1 | [Link](https://www.thingiverse.com/thing:5028827) | ~$2 | Linked "thing" is all three of the required pak parts
`*`Shipping | --- | --- | ~$7 | You'll probably want to have all three pak parts 3d printed from the same supplier to save on shipping
--- | --- | --- | --- | ---
**Total** | --- | --- | ~$24.5 | 


