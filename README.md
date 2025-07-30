# WCU Wind Tunnel

## Overview of System

This document describes the functioning of the WCU Wind Tunnel software, firmware, and data collection hardware design, and is meant as an entrypoint for anybody who needs to understand any part of the system.

### Nomenclature

- **VFD**: Variable Frequency Device, used to "throttle" the fan. The box next to the fan with the dials.
  
<img src="https://github.com/user-attachments/assets/19e56fcd-8ca9-4bc7-bb18-74fa41cda1f9" width="550">

- **CAB**: "Clear Acrylic Box" housed underneath the table that is the hub between all sensors.
  
<img src="https://github.com/user-attachments/assets/da0145e0-fb84-419c-a83e-07ca165ea0d3" width="550">

- **The DAQ**: The central hub for all sensor information, populated by "modules" that read and write analog and digital voltage levels.
  
<img src="https://github.com/user-attachments/assets/fd467095-4ea5-4b34-bf61-242e4e97b10d" width="550">

- **NI-9239 or "9239"**: A module for the DAQ that acts as a multimeter, reading analog DC voltage across its terminals.
- **NI-9219 or "9219"**: A module for the DAQ with additional functionality for load cells.
- **NI-9263 or "9263"**: A module for the DAQ that outputs an analog voltage signal.
- **VI**: "Virtual Instrument", the LabVIEW program (like a `.py` or `.c` file).
- **Sting**: The metal arm in the test section, equipped with three load cells, LC0-LC2.
  
<img src="https://github.com/user-attachments/assets/4b95a646-b4f1-4379-906d-01a0838465cb" width="550">

### Quick Start Guide

While it is recommended that the user read the [Developer Guide](#developer-guide) in its entirety, this section can be used to quickly understand the basic operation of the system.  
To get data quickly, the following steps can be used to safely start and operate the system:

1. Familiarize yourself with the LabVIEW panel. Pressing Ctrl+H will bring up a help menu. Clicking components will bring up more information on thier purpose.
2. Looking at the graphs, they will not be zeroed. To fix this press SYSTEM TARE (bottom right of the Panel).  You should expect the graphs to the right to be within +- 5 of zero after tare.
3. Looking at the VFD box (the one attached to the fan with dials/switches) make sure it is set to ON, switched to Automated, and check that the Emergency Stop button is not pressed. If it is, twist it clockwise to reset
4. Now the fan is controlled by this panel. On the "Fan Controls" below, set the "VFD mode switch" to the upward postition to to enable Speed % Mode, which controls the fan by a percentage of its maximum power. You can now adjust the box "Speed %" which will spin up the fan.

### Data Collection

This application has extensive data logging capabilites, logging both raw unfiltered data and processed data. In order to utilize this function the following steps are required:

1. Get wind tunnel and LabView set up and working (See quick start).
2. Make sure application is halted by clicking "System Stop" button.
3. Name your spreadsheet and select a directory from the folder icon (This defualts to the current date: YYYY-MM-DD_HH_MM_SS.csv and to the testing data directory on the pc on the cart)
4. Make sure the Enable Data Collection is on and set to "Enabled"
5. Start the program by clicking on white arrow on top left of screen
6. Proceed with experiment
7. *Important*: Hit system stop, **NOT** red stop sign on top left. A terminal will pop up for a second
8. There should be two new CSV's created. One with _processed.csv and one without. The CSV without contains unformatted data (see Developer Guide) with more data points at the cost of readability. The _processed.csv has been formatted for readablilty at the cost of additional data points (these are averaged out into the values shown).
9. If the _processed.csv isn't there and you need it, drag the unproccessed spreadsheet onto "process_csv.py" and this should format the data. Otherwise there is an issue with the python script and it is failing to create the formatted CSV.

## Developer Guide

### Hardware Guide

The electrical system for the wind tunnel consists of a central hub called the [CAB](#nomenclature) that interfaces with a suite of sensors and the VFD that allows the operation of the system.  
This section will describe each subsystem in detail.

---

#### CAB Overview

The central part of the CAB is the [DAQ](https://www.ni.com/docs/en-US/bundle/daq-getting-started-bus-powered-usb/page/getting-started.html).  
This device acts as the firmware between the sensors and LabVIEW.  
There are three modules used to do this:

- The [NI-9239](https://www.ni.com/en-us/shop/model/ni-9239.html) analog input module that acts like a multimeter.
- The [NI-9219](https://www.ni.com/en-us/shop/model/ni-9219.html) is very similar to the NI-9239, with additional functionality for load cell sensors.
- The [NI-9263](https://www.ni.com/en-us/shop/model/ni-9263.html) analog output module used to throttle the VFD, akin to a programmable power supply.


---

#### Test Section Overview

The test section is the box with the clear window where data is collected. This data collection is facilitated by the DAQ.

The DAQ acts as a combined multimeter and power supply, used to monitor and control each subsystem.

- **Pressure Transmitters**:  
  [LPPT25-20-V30H pressure transmitters](https://www.automationdirect.com/adc/shopping/catalog/process_control_-a-_measurement/pressure_sensors/pressure_transmitters/lppt25-20-v30h) output 4-20 mA, mapping to -14.7 to 30 PSIG (relative to ambient).  
  A 220 Ω shunt resistor is used, with conversion done in software:  
  `ma = V / 220 * 1000`.

- **VFD Frequency In**:  
  The VFD can report on its current frequency (the fan used in the wind tunnel runs on AC power, so is throttled via frequency. 0hz means the fan is off, and 60hz means the fan is at full power). This value is read from AO-0 (see VFD documentation) into the NI-923 as a voltage from 0-10v. 1v * 6 = 6hz = 10% of the fans total power.

- **VFD Frequency Out**:
  The VFD can be controlled by an external 0-10v source (0v = 0%, 10v = 100%). This is what throttles the fan. The NI-9263 is used as a voltage source to control the VFD, allowing the device to be controlled in software. 

- **Sting**:  
  A manually operated mechanism with three load cells connected to the NI-9219, measuring lift, drag, and moment.

---

#### VFD

The VFD (Variable Frequency Drive) controls fan speed via 0–10 V input.  
Located on the fan, the box includes:

- A red emergency stop button
- On/off switch
- Potentiometer dial
- Switch for "AUTOMATED" vs. "MANUAL" mode

- There are multiple layers of abstraction to controlling the VFD. At the lowest level the user has the ability to throttle the VFD directly without any interaction with the Labview program. This is Manual Mode and should only be used if LabVIEW is not accessible, or if an issue arises with the system and more direct control is desired. The user can also throttle the fan via Automated mode, with varying levels of absraction, described below. 

- **Manual Mode**:  
  Uses physical potentiometer. Must be monitored by an operator AT ALL TIMES!

- **Automated Mode**:  
  Uses LabVIEW to control fan speed via analog output. Similar to a Fly-By-Wire system the user can throttle the system, but the computer sends the actual commands. This should be the main way of interfacing with the system.
  There are two main options, Speed % mode and PID or Target Speed mode. The Speed % mode works by sending a command to the VFD to spin the fan at X% of its power. PID or Target Speed mode works VIA a [PID Loop]([https://www.automationdirect.com/adc/shopping/catalog/process_control_-a-_measurement/pressure_sensors/pressure_transmitters/lppt25-20-v30h](https://www.isa.org/intech-home/2023/june-2023/features/fundamentals-pid-control)), taking in a target airspeed, provided by the user, and the current airspeed, and adjusts the fan speed to reach the target speed. The current airspeed is either determined by the pitot tubes, giving a direct indicated airspeed, or an estimate based on VFD frequency, chosen by the user upon selection of this mode.

---

## Programming Guide

#### LabVIEW Overview

The wind tunnel is controlled through a LabVIEW program or the VI.  
LabVIEW was chosen for accessibility among engineers, to allow those with limited programming experience the ability to modify the program.

A VI consists of a Front-Panel that hosts controls and data visualization, as well as a Block-Diagram which is the underlying code that runs the program. 
The Front-Panel is all the user will see when the VI is exported as an executable.

#### Descriptions and Tooltips

Most components in the VI include tooltips.  
Access these through **Help → Context Help** and click any block to view documentation and comments.

#### Main Program Loop

The program is designed to mimic the Arduino's architecture: 
- Code outside the loop runs once at start ("setup")
- Code inside runs continuously ("loop")

<img width="550" alt="Screenshot 2025-07-24 103742" src="https://github.com/user-attachments/assets/abdaab84-7a3b-49f5-94a1-0c146afeb840" />

#### Shift Registers

[Shift Registers](https://knowledge.ni.com/KnowledgeArticleDetails?id=kA03q000000YKYuCAO&l=en-US) are used to maintain state between loop iterations.  
Each shift register contains two terminals on each side of the loop. The left terminal sends either the initial value fed in from outside the loop, or the previous value sent into the right terminal. 
Example: "Tare SRs" are used to zero out sensor readings based on a user-controlled boolean. 

<img height="400" alt="Screenshot 2025-07-17 134048" src="https://github.com/user-attachments/assets/a5421822-31db-4b15-97b0-b7fb4ed0f60c" />


#### DAQ Integration

The [DAQ](https://www.ni.com/docs/en-US/bundle/measurement-studio-ni-daqmx-projects-for-.net-4.5.1/page/netdaqasst.html) handles all sensor I/O.

- **DAQ Input**:  
  - Lists each sensor channel
  - Outputs dynamic signals (arrays with metadata)
  - Split into: (1) raw data logged to file, (2) signal processed in VI
  
    <img width="300" alt="image" src="https://github.com/user-attachments/assets/d630b287-842f-4c90-b43c-f8aa91290b97" />


- **DAQ Output**:  
  - Configured similarly
  - Controls voltage outputs to the VFD
 
    <img width="300" alt="image" src="https://github.com/user-attachments/assets/764bf0e2-ae66-456a-91d5-62a7059965c6" />

#### Sub-VI's
To make the code more compact and readable (very important when looking at LabVIEW), there are SUB-VI's, which are essentially functions. These have inputs outputs, and contain code within.
These allow the programmer to cut down on "spaghetti" and make it easier for the next developer down the road. 

The Sub-VI is a self contained program with its own front panel and IO. Below are a few examples of Sub-VI's:

An important feature of any sensor suite is the ability to "Tare" or adjust the data from the sensor to zero when you'd expect it to be at zero (at system idle). Think of zeroing a scale just with different sensors. 

The taring Sub-VI is shown below:

<img width="400" alt="Screenshot 2025-07-24 105234" src="https://github.com/user-attachments/assets/47e67969-b64a-443a-a60d-4ed61b3ea032" />

Each of lines entering the Sub-VI are either inputs or outputs. These are converted into controls on the front panel that feeds into the logic contained in the block diagram.

<img width="400" alt="image" src="https://github.com/user-attachments/assets/8625865d-e004-4978-859c-746c5d62f559" />

The image attached is what is contained within the Sub-VI. On the right is the front panel with the IO terminals labeled. If you hover over the lines entering the Sub-VI they will line up with the various IO terminals.
On the left is the Tare logic. Each of the components has a context help, seen on the top left to help decode what is going on.

#### Data Collection
Data collection is very important for the use of the wind tunnel. Throughout its operation, both its raw and processed data are fed into a spreadsheet for analysis. 
Attached is an image of this system:

<img width="400" alt="image" src="https://github.com/user-attachments/assets/e6cc30e2-c869-4c64-9b66-73d126769e50" />

On the left are the data-to-dynamic.vi's. These convert the processed data (velocity, force readings, etc) back into the dynamic data, compatible with the data the daq outputs. These dynamic data streams are synced with a value t0, stripped from the raw data stream, syncing each of the data points. These data points are then combined into an array of signals (tree looking thing) and sent into data-collection.vi. This Sub-VI activates a write to measurement (csv) function every X ms, determined from the Spreadsheet Write Timer. If this timer sends its signal, and data collection is enabled, the write to measurement function writes the data assembled to a spreadsheet specified by the path-gen.vi (see right side of block diagram outside of main loop). Note that if the spreadsheet write delay is too small, the lag caused by the writing will cause the program to crash.

#### Process CSV

As a quirk of how the data collection is implemented, the outputted data is a little strange. It has no headers, and for every 100 raw data entries, there is only one processed data entry. This is due to how the daq assistant outputs data. It is running continously at 200hz reading 100 samples, so it outputs 100 samples every 0.005 seconds. When the CSV is written to, the full 100 samples being outputted by the daq is written but only the one processed data entry is entered. Luckily, because everything is timestamped it all is written in the correct places, just with gaps where there is no processed data to write. To solve this there is a script, process_csv.py that takes the average of the 100 raw data points and collapes all the data into a nicely formatted CSV. To use the tool simply drag the original CSV onto the python program and it will output a "processed.csv" version in the same directory. There is a trade off of data resolution (in the original) for formatting and ease of use (in the processed version).

<img width="400" alt="image" src="https://github.com/user-attachments/assets/cf0dc15f-7a4e-41f5-8474-12a7945609e9" />

### Accelerometers

Accelerometers are an important troubleshooting tool, giving informtation on vibrations on the test section, sting, and wherever else needs attention. There are three [ADXL345](https://www.adafruit.com/product/1231) accelerometers, with support up to eight sensors. These sensors communicate over [I2C](https://learn.sparkfun.com/tutorials/i2c/all), which the DAQ doesnt natively support. To overcome this a simple microcontroller (MCU) is used to communicate to the sensors, convert the data recieved from I2C into [UART](https://www.analog.com/en/resources/analog-dialogue/articles/uart-a-hardware-communication-protocol.html) a simpler, but very compatible communication format. The MCU is an [STM8](https://www.st.com/en/microcontrollers-microprocessors/stm8s103f3.html) and was used because its what I had on hand. Its programmed in a similar way to Arduino. The sensors are plugged into a I2C multiplexer as they all share the same address. This allows the system to have accelerometers hotplugged into the system, and will work with up to eight concurrent adxl345 sensors with no other work needed from the user. The data is visualized in the accelerometer vi and will show live data from each of the sensors plugged into the system. Make sure the right COM port is selected. See troubleshooting. 


### Troubleshooting
#### No change in data readouts in LabView
- Make sure daq is powered and plugged into the computer.
- Check raw voltage data. Pressure transmitters should read ~2v, and the load cells should read around 0v.

#### Fan not spinning up
- Is everything is plugged in.
- Double check VFD panel.
- Is VFD mode select switch in LabView set to the setting you expect?
- Set VFD Mode Select to Speed %, enter in 10v, the Voltage Out should read 1. VFD frequency should climb to ~6 hz.
- If Frequency at 0, go to VFD, open box. The display should read ~6hz
- If the VFD is reporting 0hz, switch VFD to manual, and use dial on side to throttle VFD. If fan spins up, continue this troubleshooting guide, otherwise the issue is outside this guides scope
- Check wiring leading to the CAB.s

#### Accelerometer issures
- Look at the board above the test section, the blue board should have a blue solid light and a flashing red light. If red light not flashing, press reset button on board.
- Open up Arduino serial moniter on this computer. Check all COM ports until data shows up on the serial moniter. Close arduino and set port accordingly.
- Check all wiring. Each acclerometer should have a green light.

### Requirements
#### Labview
- To run the executable LabView runtime is needed. This software is free. [LabView Runtime](https://www.ni.com/en/support/downloads/software-products/download.labview-runtime.html?srsltid=AfmBOopup8cn0oFzFOwyLhWmhqS4r_rSOEyRL3oeYinoMHA7EQHQmmXR)
- To edit the code you need a copy of NI LabView 2025. Talk to Heather
#### Accelerometer
- To edit the accelerometer code, you need an ST-Link V2 and VS Code with PlatformIO. I recommend reaching out to me (Kyle Moore) if you need to work on anything regarding the sensors. Otherwise here are some resources:
  - [https://circuitdigest.com/microcontroller-projects/getting-started-with-stm8s-using-stvd-and-cosmic-c-compiler](https://www.hackster.io/patrick-fitzgerald2/program-stm8-with-sduino-arduino-ide-st-link-eb5c71)
  - [https://tenbaht.github.io/sduino/](https://www.hackster.io/patrick-fitzgerald2/program-stm8-with-sduino-arduino-ide-st-link-eb5c71)
  - [https://www.hackster.io/patrick-fitzgerald2/program-stm8-with-sduino-arduino-ide-st-link-eb5c71](https://www.hackster.io/patrick-fitzgerald2/program-stm8-with-sduino-arduino-ide-st-link-eb5c71)
