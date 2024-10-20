# SCPI-like Protocol

## I2C Bus Overview

Devices controlling the load must connect to the load’s I2C bus to send commands. The protocol currently supports two servers, each responsible for a specific task in load control.

- **Load Server:** Manages active control of the power and analog systems on the load. This server runs on the STM32 in the load module.
- **HMI Server:** Handles operations on the SD card in the front panel, manages load configuration files, and controls the streamer module for custom curve streaming, runs on the ESP32 in the front panel.

### I2C Addresses

- **Load Server:** 0xA
- **HMI Server:** 0x5


## Protocol Specifications

The protocol uses 2-character pieces organized hierarchically, where each piece represents a sub-resource of the previous one. Together, these pieces form the route to a specific resource or operation.

### Separators

The protocol supports three different separators, each with a specific purpose:

- **':' (Colon):** Indicates access to a sub-resource. It means the current resource is not the end of the parse tree.
- **'?' (Question Mark):** Represents a query. When used, it stops the parse tree at that point, and the level where it stops must implement the query functionality.
- **' ' (Space):** Indicates that the next block of text is a value for the current level. It stops the parse tree and passes the remaining string to the current level for processing.

## Load Protocol (Load Server)

### Resource Codes

- **ID:** Identification
- **IN:** Input
- **MD:** Mode
- **CC:** Constant Current (For queries: returns the current value)
- **CV:** Constant Voltage (For queries: returns the current value)
- **CR:** Constant Resistance (For queries: returns the current value)
- **CP:** Constant Power (For queries: returns the current value)

### Identification

- **ID?:** Returns the identification string of the load module.

### Input

- **IN?:** Queries whether the input (output terminals) is currently enabled.
- **IN 0:** Disables the input.
- **IN 1:** Enables the input.

### Operating Modes

- **MD?:** Returns the currently selected operating mode.
- **MD:CC:** Sets the mode to constant current.
- **MD:CV:** Sets the mode to constant voltage.
- **MD:CR:** Sets the mode to constant resistance.
- **MD:CP:** Sets the mode to constant power.

### Levels

- **LV:CC? [A]:** Returns the measured current in amperes.
- **LV:CV? [V]:** Returns the measured voltage in volts.
- **LV:CR? [R]:** Returns the measured resistance in ohms.
- **LV:CP? [W]:** Returns the measured power in watts.

- **LV:CC 0.0000 [A]:** Sets the current level to the specified value (in amperes).
- **LV:CV 0.0000 [V]:** Sets the voltage level to the specified value (in volts).
- **LV:CR 0.0000 [R]:** Sets the resistance level to the specified value (in ohms).
- **LV:CP 0.0000 [W]:** Sets the power level to the specified value (in watts).

## HMI Protocol (HMI Server)

### Resource Codes

- **ID:** Identification
- **LK:** Panel Locking
- **SD:** SD Card
- **ST:** Streamer Module
- **RD:** SD Read Operation
- **WT:** SD Write Operation

### Identification

- **ID?:** Queries the panel module for its identification string.

### Panel Locking

- **LK?:** Queries whether the front panel is currently locked (user interaction disabled) or unlocked (user interaction enabled).
- **LK 0:** Unlocks the front panel, preventing user interaction.
- **LK 1:** Locks the front panel, allowing user interaction.

### SD Card

- **SD?:** Queries if the SD card module is initialized and operational.

### SD Card File Operations

- **SD:RD 0:** Lists the first 10 files starting from the specified page.
- **SD:WT:OP aaaaaaaaa.ext:** Starts writing to a file named `aaaaaaaaa.ext`.
- **SD:WT:CL:** Stops writing to the file.
