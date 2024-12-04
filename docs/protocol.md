# SCPI-like Protocol

## I2C Bus Overview

Devices controlling the load must connect to the load's I2C bus to send commands. The protocol supports hierarchical parsing with registers and sub-registers to interact with the load server and HMI server.

### I2C Addresses

- **Load Server:** 0xA  
  Responsible for managing active control of power and analog systems. This runs on the STM32 in the load module.  
- **HMI Server:** 0x5  
  Manages operations on the SD card, load configuration, and streaming functionalities. This runs on the ESP32 in the front panel.

---

## Protocol Specifications for I2C Communication

### Command Format

The protocol uses a structured command format, represented as:  
**C:R:S:VV**  

- **C:** Command type (0x0 for query, 0x1 for set)
- **R:** Register (defines the resource or mode being accessed)
- **S:** Sub-register (specific sub-resource or parameter)
- **VV:** Value (2 bytes, applicable for set operations)

### Supported Commands

#### Identification (ID)
- **Query ID**:  
  Command: `0x0:0x0:0x0`  
  Returns the identification string of the load module.  

#### Input Control (IN)
- **Disable Input**:  
  Command: `0x1:0x1:0x0:0x0`  
  Disables the input (output terminals).  
- **Enable Input**:  
  Command: `0x1:0x1:0x0:0x1`  
  Enables the input (output terminals).  
- **Query Input State**:  
  Command: `0x0:0x1:0x0`  
  Returns whether the input is currently enabled or disabled.  

#### Operating Modes (MD)
- **Query Mode**:  
  Command: `0x0:0x2:0x0`  
  Returns the currently selected operating mode.  
- **Set Mode to Constant Current (CC)**:  
  Command: `0x1:0x2:0x1`  
- **Set Mode to Constant Voltage (CV)**:  
  Command: `0x1:0x2:0x2`  
- **Set Mode to Constant Resistance (CR)**:  
  Command: `0x1:0x2:0x3`  
- **Set Mode to Constant Power (CP)**:  
  Command: `0x1:0x2:0x4`  

#### Levels (LV)
- **Query Level**:  
  Command: `0x0:0x3`  
  Queries the current level for the specified sub-register.  
- **Set Current Level**:  
  Command: `0x1:0x3:0x1:VV` (Value in amperes)  
- **Set Voltage Level**:  
  Command: `0x1:0x3:0x2:VV` (Value in volts)  
- **Set Resistance Level**:  
  Command: `0x1:0x3:0x3:VV` (Value in ohms)  
- **Set Power Level**:  
  Command: `0x1:0x3:0x4:VV` (Value in watts)  

#### Status (ST)
- **Query Status**:  
  Command: `0x0:0x4:0x0`  
  Returns a structured response with the following fields:  
  - **Byte 1:** Input state (0 or 1)  
  - **Byte 2:** Current mode (CC, CV, CR, CP)  
  - **Bytes 3-4:** Current set current value  
  - **Bytes 5-6:** Current set voltage value  
  - **Bytes 7-8:** Current set resistance value  
  - **Bytes 9-10:** Current set power value  
  - **Bytes 11-12:** Measured current value  
  - **Bytes 13-14:** Measured voltage value  


### Commands
| Command |  Value | Description |
|---------|--------|-------------|
| Query      | 0x0    | Query the value of a register |
| Set        | 0x1    | Set the value of a register |


### Registers

| Mode | Register | Description |
|------|----------|-------------|
| ID  | 0x0        | Constant Current |
| IN  | 0x1        | Input |
| MD  | 0x2        | Mode |
| LV  | 0x3        | Levels |
| ST  | 0x4        | Status |

### Sub-Registers

| Mode | Sub-Register | Description |
|------|----------|-------------|
| __  | 0x0        | General |
| CC  | 0x0        | Current |
| CV  | 0x1        | Voltage |
| CR  | 0x2        | Resistance |
| CP  | 0x3        | Power |



## HMI Protocol (HMI Server)

### Resource Codes

- **ID** Identification
- **LK** Panel Locking
- **ST** Streamer module
- **CX** Set File Descriptor
- **LS** SD Read Operation
- **WT** SD Write Operation

### Identification

- **ID?** Queries the panel module for its identification string.

### Panel Locking

- **LK?** Queries whether the front panel is currently locked (user interaction disabled) or unlocked (user interaction enabled).
- **LK 0** Unlocks the front panel, preventing user interaction.
- **LK 1** Locks the front panel, allowing user interaction.

### SD Card File Operations

#### Listing available files

- **LS 0** Queries files in SD card `streams` directory from page zero, each page contains 10 files.

#### Selecting a file descriptor

- **CX?** Queries the current file descriptor.
- **CX <name 20 chars>.st** Sets the file descriptor to the specified file. The file descriptor is used in subsequent operations to refer to the file.
- **CX 0** Resets the file descriptor to zero.

#### Write operations (Set file descriptor before writing, if file descriptor does not exist, it will be created)

- **WT** This will clean current file in file descriptor.
- **WT <cmd>** This will write the data to the file in file descriptor, data can be any of valid commands in protocol.

#### Streamer module

- **ST?** Will return 1 if streamer module is streaming data, 0 otherwise.
- **ST 0** Will stop the streamer module.
- **ST 1** Will start the streamer module using the file descriptor as the source.
