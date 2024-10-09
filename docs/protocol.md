# SCPI Like protocol

## About the BUS

All devices that wants to control the load must be capable of connecting in the I2C bus of the load to send commands to it. Right now there the protocol of the load covers 2 servers, each one being responsible for handling one main task on the load control.

* Load Server: This one is responsible to execute the active control of the power and analog sector of the load, it runs on the STM32 present on the load module
* Panel Server: This one is responsible from handling configurations about the panel module (Holds WiFI, Panel and SD card), it allows the user to configure current network settings as well as read/write on the SD card and activate streamer module, it also allows for locking of the front panel to avoid mistakes like interactions while streamer is activated

### Addresses

* Load Server uses address (0x....)
* Panel Server uses address (0x....)

## About protocol specifications

The protocol consists of small 2 char pieces organized in an hierarchical way where each piece represents a sub resource of the previous one and that all together generate the route to some resource to be operated.

About separators, the protocol supports 3 different separators, each one being used with an specific purpose:
* The ':' separator, this one basically means access to a sub resource and that current resource does not ends the parse tree
* The '?' This one represents a query, the using of this operator means that the parse tree will be stopped as soon as this operator is hit and the level where it stopped must implement the query functionality
* The ' ' this operator indicates that the next block of text provided after it will be passed as value to current level that implements it functionality, it will automatically stops the parse tree and pass the remaining string to current level to process as needed

## Load Protocol (Load Server)

### Resources pieces

* 'ID': Identification
* 'IN': Input
* 'MD': Mode
* 'CC': Constant Current (When used in a query means current current)
* 'CV': Constant Voltage (When used in a query means current voltage)
* 'CR': Constant Resistance (When used in a query means current resistance)
* 'CP': Constant Power (When used in a query means current power)

### Identity

'ID?': This query asks the load module to identify itself, providing current identification string

### Input

'IN?': This query will return if the load has it input activated or not right now, this means if the output terminals are actually enabled

'IN:0': This command will set input to disabled
'IN:1': This command will set input to enabled

### Modes

'MD?': This query will return current selected operating mode

'MD:CC': This command will set current mode to be constant current
'MD:CV': This command will set current mode to be constant voltage
'MD:CR': This command will set current mode to be constant resistance
'MD:CP': This command will set current mode to be constant power

### Levels

'LV:CC?' [A]: This query will return current measured value of current
'LV:CV?' [V]: This query will return current measured value of voltage
'LV:CR?' [R]: This query will return current measured value of resistance
'LV:CP?' [W]: This query will return current measured value of power

'LV:CC 0.0000' [A]: This command will set current level of current to be the provided value
'LV:CV 0.0000' [V]: This command will set current level of voltage to be the provided value
'LV:CR 0.0000' [R]: This command will set current level of resistance to be the provided value
'LV:CP 0.0000' [W]: This command will set current level of power to be the provided value

## Panel Protocol (Panel Server)

### Resources pieces

'ID': Identification
'LK': Panel Locking
'SD': SD card
'ST': Streamer module
'RD': Read operation
'WT': Write operation

### Identity

'ID?': This query asks the panel module to identify itself, providing current identification string

### Panel Locking

'LK?': This query will return if front panel is enabled to user to interact or disabled right now

'LK:0': This command will unlock front panel preventing user to interact with it
'LK:1': This command will lock front panel allowing user to interact with it

# SD card

'SD?': This query returns if SD card module is initialized and operational

# SD card file operations

'SD:RD': This will list all files in sd card,
