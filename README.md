# teleprinterHat
A RaspberryPi HAT that interfaces a old school teleprinter

![Finished board](https://i.imgur.com/QwgbDhd.jpg)

## History
Some time ago I started planning to get an old Lorenz Lo15 teleprinter working. Then arose the question about how to interface those old machines to the real world. While investigating this I gained quite some knowledge of teleprinter interfacing and Telex networks. For example the normal line voltage of a US teleprinter line is 120VDC and the current is 60 mA.

Preferbly the interfacing should be as simple and small as possible. Using some big expensive 50 Hz 120 VAC transformer, rectifier and 2kOhm 6W resistor was not on the table.

Preferbly the circuit should be able to run off a standard USB charger. I.e. 5V 500mA. Experimenting a bit with a stepup circuit gave that is perfectly possibly to step up 5V to 120V. SO one could ask how could a 5V 500 mA supply be able to supply the power for a 120V 60mA load. Of course it cannot, but the problem lies in the fact tat most of the power is absorbed in the current limiting 2kOhm resistor of the old school circuit. The actual teleprinter has an internal resistance of approximately 100 Ohm. So why 120V? The reason for 120V is to achieve a fast pull of the magnet inside the teleprinter. After the initial pull the steady state voltage drop is between 5 to 10 V at 60mA. 

When I started to work with this project I also made an announcment in the i-Telex forum and got a reply from Jochen that had done something similar and the same point in time. What a coincidence! [piTelex project](https://github.com/fablab-wue/piTelex)

Jochen have compiled a lot of interesting and useful information there but the project is based on a Raspberry Pi and cannt be used stand alone. One of my requirements are that it should be able to run completely stand alone. I don't want the bloat of a Linux kernel just to push a few bytes back and forth!

## Requirments

So I put together a list of requirments

1. Shall only need 5V supply. The generation of up 120V DC is built into the device. Using a step-up converter.
2. Low power consumption. No 6W 2kOhm resistor. Use PWM to control the current through the solenoid. 
3. Configurable for 60mA, 40mA and 20mA current loop.
4. ~~Automatically detect the line voltage to use based on line current and required rise time of current through coil. Measure what kinde of loop inductance there is and adapt to a possibly resistive remote.~~ Deemed not possible with current design.
5. Interface directly to RS-232-E levels on board. (option) using a MAX232 or equivalent and a couple of capacitors
6. Interface directly to USB using FTDI chip on board (option)
7. Do baudot to ASCII conversion locally.
8. Do baudrate adaption locally. Configurable teleprinter baudrate to support at least 45,5, 50, 56, 75, 100 and 110 bps.
9. Configurable uplink RS-232 interface 50, 75, 110, 150, 300 bps to adapt to all sorts of hosts.
10. Dimensions that fits the Rpi Zero preferbly but if it is getting to crammed a standar Rpi HAT is the choice.
11. The RPI should be able to control all settings of the device from GPIO ports. Possibly use SPI to make more advanced configuration possible.
13. LEDs connected to RPi GPIOs to indicate mode or state in RPi Software.
14. Jumpers connected to the RPI GPIOs to control the RPI software and selection om mode of operation.
15. The RPi shall automatically boot up and select operating mode depending on jumpers. For example i-Telex or news RSS, RasPi serial console. No interaction using screen or keyboard connected to the RPi shall be necessary.
16. Relay to support reversal of polarity of the line current (optional)
17. Be able to drive a SSR that can be used to enable power to the teleprinter
18. Diagnostic LED to indicate line malfunction. I.e. to detect an open loop 
19. Current loop interface to support old PDP-11 current loops using opto couplers.
20. Use two 2.5 mm mono plugs for interfacing the teleprinter.
21. Diagnostic LED to indicate what line voltage is selected. Maybe multicolor RGB LED?
22. RX and TX LEDs
23. Support for FSG device (optional)
24. A few test sequences are built in. Continous RY and "The quick brown fox jumps over the lazy dog"
25. Flow control using XON/XOFF, RTS/CTS or DSR/DTR. Selcteable using jumpers. Both via USB/FTDI or RS-232

## Stepup converter

The first thing to verify is that the idea of the step up converter was ok. I did a small circuit board to check the circuit and during this I found that the parameters of the coil si very importantant. It has to have a high enough Isat. Then the choice of MOSFET is important. The MOSFET has to full turn on at only 5V gate voltage which is what is available in this circuit. Thus Vgs(ON) is of high interest. Then to deliver as much of the 5V into the coil means that as little loss of energy in the MOSFET is important. The rDS(ON) is the parameter to look for. 

![Step up hook up](https://i.imgur.com/SqzuRBvm.jpg)

It turned out that this circuit worked well and could easily deliver 120VDC.

## Using PWM to control the selector magnet.

The main idea with this circuit is to generate the required 60mA with as litle loss as possible. By measuring the current in the loop using a small resistance and then hysteresis regulate the current using a PNP transistor as switch. The current is feed through a small inductor which will not saturate. 

As the switch transistor is swithced on the current in the inductor will increase with time. When it reach the upper hysteresis limit the transistor will switch off. The current in the inductance will continue to flow through the diode and the resistor. As the current reaches the lower limit the transistor will switch on again. 

The receiver magnet inductance will only come into play when going from sending a space bit to a mark bit. Then the large total inductance will keep the regulator switched on for several milliseconds depending on the inductance in the magenet. As soon as the line current level is reached the magnetcoil is fully saturated and will act as a pure resistance. The implementation is based on a op-amp for measuring the differential volatage of the current measuring resistor. Then a fast comparator with a 5% hysterisis band generates a square wave from the incoming sawtooth signal. This squarewave then drives the switch transistors. 

The transistor switch is composed of a PNP transistor as the main switch transistor and a NPN level cvonversion transistor driving the PNP transistor through a resistance.

The reference voltage to the hysteresis comparator is supplied from a PWM signal output of the mikroprocessor. Thereby making it possible to control the current level through software. The PWM signal is low pass filtered in a simple RC network and then buffered through a buffering op-amp.

I did a quick mockup with components at hand but it turned out that the comparator needed to be pretty fast and the transistor switch as well. It simply didn't work very well. I decided to try to do simulations in LTspice before doing further breadboarding. 

Searches on internet gave a decent circuit for a P-MOS FET based switch. Simulations in LTspice gave proper values for resistors and capacitors and it was time to make a peroper board.

![Current source prototype](https://i.imgur.com/m5oTShe.jpg)

Schematic.

![Current source prototype schematic](https://i.imgur.com/kEjGToI.png)

The design also included an PNP and NPN transistor to handle keying of the sening signal.

A simple NPN transistor is used to key the current in the loop ON/OFF. This has to be done without any free-wheeling diode since otherwise the release of the magnet when swithcing of the current will be very much delayed and might not even happen at all. 

Initial tests showed good performance. It could regulate the desired current at varying line voltages without any problem.

![Current source regulating](https://i.imgur.com/07wgvVe.jpg)

This picture show the voltage over the measurement resistor. The scale is 100mV per division and the resistor is 51 ohm so the interval it is regulating over is +/- 2mA as designed.

Running the circuit with the actual Lorenz Lo15 teleprinter also yielded good results.

![Current source regulating](https://i.imgur.com/mdcDE5u.jpg)

Keying the signal, 90V line voltage, 40 mA line current. The large spikes are created by the selector magnet as the switch cuts off. Had initial problems that the choosen PNP transistor was unable to cope with these. Intalled a 300 V PNP transistor and a MOV to handle the spikes. Worked well. But I am considering using a TVS instead.

![Current source regulating](https://i.imgur.com/1IbDv2K.jpg)

This picture shows when regulating the 40mA output to the selector magnet when the line is in marking state.

## Prototyping the Step up converter and current source 

I already did a prototype of the step-up converter. But the MOSFET used was quite big and overdimensioned. I also wanted to use a proper gate driver as well as off load the micro controller doing the chore of keeping the PWM rate continuosly measuring the output voltage. A separate, very small, switch control IC, cheap as well, was a better choice. So I designed another prototype board to test out these concept as well as testing the current loop circuit.

![Current source regulating](https://i.imgur.com/Q6KLGss.png)

