# WCU Wind Tunnel

## Table of Contents
- [Introduction](#introduction)
  - [Overview of System](#overview-of-system)
  - [Nomenclature](#nomenclature)
  - [Quick Start Guide](#quick-start-guide)
- [Developer Guide](#developer-guide)
  - [Hardware Guide](#hardware-guide)
    - [CAB Overview](#cab-overview)
    - [DAQ Hardware Description](#daq-hardware-description)
    - [VFD](#vfd)
  - [Programming Guide](#programming-guide)
- [Notes](#notes)
- [Figures](#figures)

---

## Introduction

### Overview of System

This document describes the functioning of the WCU Wind Tunnel.  
This system is capable of up to 70 m/s (~160 mph) and has an `Xm³` test section.  
It can gather data on lift, drag, and stall characteristics of models tested, and can serve as an excellent educational aid.  
It may also be useful in advancing various forms of aerodynamic research.

### Nomenclature

- **VFD**: Variable Frequency Device, used to "throttle" the fan. The box next to the fan with the dials.
- **CAB**: "Clear Acrylic Box" housed underneath the table that is the hub between all sensors.
- **The DAQ**: The central hub for all sensor information, populated by "modules" that read and write analog and digital voltage levels.
- **NI-9239 or "9239"**: A module for the DAQ that acts as a multimeter, reading analog DC voltage across its terminals.
- **NI-9219 or "9219"**: A module for the DAQ with additional functionality for load cells.
- **NI-9263 or "9263"**: A module for the DAQ that outputs an analog voltage signal.
- **VI**: "Virtual Instrument", the LabVIEW program (like a `.py` or `.c` file).
- **LCX**: There are three load cells in the system and are labeled LC0-LC2. These can be seen in the test section

### Quick Start Guide

While it is recommended that the user read the [Developer Guide](#developer-guide) in its entirety, this section can be used to quickly understand the basic operation of the system.  
To get data quickly, the following steps can be used to safely start and operate the system.

---

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
- The [NI-9219](https://www.ni.com/en-us/shop/model/ni-9219.html) with additional functionality for load cell sensors.
- The [NI-9263](https://www.ni.com/en-us/shop/model/ni-9263.html) analog output module used to throttle the VFD, akin to a programmable power supply.

See annotated images of the DAQ and hardware in the [Figures](#figures) section.

---

#### DAQ Hardware Description

The DAQ acts as a combined multimeter and power supply, used to monitor and control each subsystem. The NI-9239 module has two terminals per channel (0 = positive, 1 = negative).

- **Pressure Transmitters**:  
  [LPPT25-20-V30H pressure transmitters](https://www.automationdirect.com/adc/shopping/catalog/process_control_-a-_measurement/pressure_sensors/pressure_transmitters/lppt25-20-v30h) output 4-20 mA, mapping to -14.7 to 30 PSIG (relative to ambient).  
  A 220 Ω shunt resistor is used, with conversion done in software:  
  `ma = V / 220 * 1000`.

- **VFD Frequency In**:  
  The VFD outputs 0–10 V from AO1, which maps to a percentage. This voltage is multiplied by 6 in software to estimate fan frequency.

- **VFD Frequency Out**:  
  The NI-9263 module sends 0–10 V to the VFD via AI0, mapping to 0–60 Hz.  
  Users can set a fixed percentage of max speed or use PID velocity control (see *TODO SOFTWARE DOCS*).

- **Sting**:  
  A manually operated mechanism with three load cells connected to the NI-9219, measuring lift, drag, and moment. See [Figures](#figures).

---

#### VFD

The VFD (Variable Frequency Drive) controls fan speed via 0–10 V input.  
Located on the fan, the box includes:

- A red emergency stop button
- On/off switch
- Potentiometer dial
- Switch for "AUTOMATED" vs. "MANUAL" mode

- **Manual Mode**:  
  Uses physical potentiometer. Must be monitored by an operator.

- **Automated Mode**:  
  Uses LabVIEW to control fan speed via analog output.  
  User can specify frequency (0–60 Hz) or desired airspeed (0–70 m/s).

---

### Programming Guide

#### LabVIEW Overview

The wind tunnel is controlled through a LabVIEW program (the [VI](#nomenclature)).  
Though non-traditional, LabVIEW was chosen for accessibility among engineers.

#### Descriptions and Tooltips

Most components in the VI include tooltips.  
Access these through **Help → Context Help** and click any block to view documentation and comments.

#### Main Program Loop

The main VI is enclosed in a while loop.  
- Code outside the loop runs once at start ("setup")
- Code inside runs continuously ("loop"), similar to Arduino

#### Shift Registers

[Shift Registers](https://knowledge.ni.com/KnowledgeArticleDetails?id=kA03q000000YKYuCAO&l=en-US) are used to maintain state between loop iterations.  
Example: "Tare SRs" are used to zero out sensor readings based on a user-controlled boolean.

#### DAQ Integration

The [DAQ](https://www.ni.com/docs/en-US/bundle/measurement-studio-ni-daqmx-projects-for-.net-4.5.1/page/netdaqasst.html) handles all sensor I/O.

- **DAQ Input**:  
  - Lists each sensor channel
  - Outputs dynamic signals (arrays with metadata)
  - Split into: (1) raw data logged to file, (2) signal processed in VI

- **DAQ Output**:  
  - Configured similarly
  - Controls voltage outputs to the VFD

---

## Notes


---

## Figures

### CAB Power Block Terminals

![CAB Power Block Terminals](CAB1.png)

### Top Down View of DAQ in CAB

![Top down view of DAQ in CAB](CAB2.png)
