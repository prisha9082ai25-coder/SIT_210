# Arduino Smart Lighting Controller

## 1. Project Overview

Project for the SIT210 – Embedded Systems Development unit. This project aims to develop the Arduino portion of a voice controlled smart lighting system. An Arduino Nano 33 IoT, Raspberry Pi 4 Model B and BH1750 light sensor were used.

The main aim of this project was to create a more convenient way of controlling lights. Commands are sent from the Raspberry Pi to the Arduino over Bluetooth Low Energy (BLE). The Arduino will then receive these commands and control the LED outputs. The Arduino will also detect how much light there is and automatically turn on/off the bathroom and hallway LEDs.

The Arduino is the main controller for the hardware. It receives sensor data, controls LED outputs, receives and processes commands via BLE and send messages back to the Raspberry Pi.

This project uses embedded coding, sensors, wireless communication and outputs. Basic error handling has been considered to identify issues such as if the sensor isn’t available or if the command isn’t supported.


## 2. Project Objectives

The main objectives of the Arduino component are:

The main aims of this component are:

-Controlling the LED lights in the bathroom and hallway using commands sent from the Raspberry Pi.

-Measure the ambient light levels using the BH1750 sensor.

-Automatically switch on/off the lights based on the light level thresholds.

-Use BLE to communicate wirelessly with the Raspberry Pi.

-Send information back to the Raspberry Pi.

-Manually control the lighting.

-Detect some errors such as failed sensor initialisation, incorrect sensor data and unknown commands.

-Have a clear structure with separate functions for initialisation, handling commands and controlling outputs.

Overall these aims show how a microcontroller can be used to create an embedded system that takes data from a sensor, communicates and controls outputs.

## 3. Hardware Components

The following components are used in the Arduino part of the system.

| Component | Purpose |
|---|---|
| Arduino Nano 33 IoT | Controls the LEDs, reads the sensor, and communicates through BLE |
| Raspberry Pi 4 Model B | Processes voice commands and sends control instructions to the Arduino |
| BH1750 light sensor | Measures ambient light intensity in lux |
| Bathroom LED | Represents the bathroom light |
| Hallway LED | Represents the hallway light |
| Fan indicator LED | Represents the status of a fan |
| 220–330 ohm resistors | Limit current through the LEDs |
| Breadboard | Provides a convenient way to assemble the circuit |
| Jumper wires | Connect the components to the Arduino |

The Arduino Nano 33 IoT is suitable for this implementation because it provides digital input/output pins and built-in Bluetooth Low Energy support. This allows it to control the connected LEDs and communicate with the Raspberry Pi without requiring a separate BLE module.

The BH1750 sensor is used because it provides light intensity readings in lux. These readings allow the program to make lighting decisions based on the surrounding brightness instead of relying only on manual commands.

## 4. Pin Configuration and Connections

The Arduino uses three digital pins to control the LED indicators.

| Component | Arduino connection |
|---|---|
| Bathroom LED | Digital pin D10 |
| Hallway LED | Digital pin D11 |
| Fan indicator LED | Digital pin D12 |
| BH1750 SDA | A4 |
| BH1750 SCL | A5 |
| BH1750 VCC | 3.3 V |
| BH1750 GND | GND |

The bathroom and hallway LEDs can be controlled separately during manual operation. In automatic mode, the program is designed to control both room LEDs together.

Each LED is connected through a suitable series resistor to limit current. The BH1750 sensor communicates with the Arduino using the I2C interface. The SDA line carries data, while the SCL line provides the clock signal for communication.

Correct wiring is important because a disconnected sensor or an incorrectly connected LED can prevent the relevant part of the system from operating as expected.

## 5. Software and Libraries

The Arduino program is written in C++ using the Arduino programming environment.

It uses the following libraries.

### 5.1 ArduinoBLE

The ArduinoBLE library provides the functions required to establish Bluetooth Low Energy communication.

In this project, BLE allows the Raspberry Pi to send text commands to the Arduino. The Arduino processes the received commands and updates a response characteristic with information about the result.

### 5.2 Wire

The Wire library supports I2C communication. It is used by the BH1750 library to communicate with the light sensor.

### 5.3 BH1750

The BH1750 library provides functions for initialising the light sensor and obtaining light intensity readings.

The sensor readings are expressed in lux, which is the standard unit used to measure illuminance.

## 6. Program Structure

The Arduino sketch is divided into several sections so that each part has a clear responsibility.

### 6.1 Pin and Variable Configuration

At the beginning of the program, constants are used to identify the LED pins. Variables are also declared to track sensor availability, automatic mode, and the shared state used by the automatic lighting controller.

The program defines two light thresholds:

- The ON threshold is 20 lux.
- The OFF threshold is 40 lux.

These values determine when the automatic controller requests the two room LEDs to switch on or off.

Keeping these values near the beginning of the sketch makes the program easier to understand and adjust.

### 6.2 Setup Function

The `setup()` function runs once when the Arduino starts.

It performs the following operations:

1. Starts serial communication at 9600 baud.
2. Configures the three LED pins as outputs.
3. Sets the bathroom and hallway LEDs to OFF.
4. Sets the fan indicator LED to OFF.
5. Starts I2C communication.
6. Attempts to initialise the BH1750 sensor.
7. Starts Bluetooth Low Energy.
8. Registers the BLE service and its characteristics.
9. Sets the initial command and response values.
10. Starts BLE advertising so that the Raspberry Pi can discover the Arduino.

The program also prints status messages to the Serial Monitor. These messages help identify whether the sensor was detected and whether BLE initialisation was successful.

If BLE initialisation fails, the program prints an error message and remains in a loop rather than continuing normal operation without its required communication functionality.

### 6.3 Main Loop

The `loop()` function runs repeatedly while the Arduino is operating.

It checks for BLE events, schedules light sensor readings, applies automatic lighting decisions when enabled, and checks for incoming commands.

The sensor is scheduled to be read approximately every two seconds. The program uses `millis()` to track elapsed time instead of placing a two-second delay in the main loop.

This approach avoids deliberately blocking the program for the full sensor interval and allows it to continue processing other operations.

## 7. Automatic Lighting Control

One of the main features of the program is automatic lighting based on the BH1750 sensor readings.

The controller checks the ambient light level approximately every two seconds. If automatic mode is enabled and the sensor reading is valid, it compares the reading with the configured thresholds.

### 7.1 Turning the Lights On

When the measured light level falls below 20 lux, the controller checks whether the shared automatic-light state indicates that the lights are currently off.

If both conditions are satisfied, the program switches the bathroom and hallway LEDs on.

This behaviour is intended to provide lighting when the surrounding environment becomes sufficiently dark.

### 7.2 Turning the Lights Off

When the measured light level rises above 40 lux, the controller checks whether the shared automatic-light state indicates that the lights are currently on.

If both conditions are satisfied, the program switches the bathroom and hallway LEDs off.

This avoids keeping the two lights on when the measured ambient light is sufficiently bright.

### 7.3 Hysteresis

The program uses separate thresholds for switching the lights on and off.

The lights switch on below 20 lux and switch off above 40 lux. Between these thresholds, the automatic controller does not change the light state based on the reading.

This approach is known as hysteresis. It helps reduce repeated switching when sensor measurements fluctuate near a threshold.

For example, if the light level changes slightly around 20 lux, using a separate OFF threshold helps prevent the controller from repeatedly switching the lights on and off.

### 7.4 Manual Override

The program also supports manual control.

When a bathroom or hallway ON/OFF command is received, automatic mode is disabled. This prevents the automatic lighting controller from immediately overriding the requested manual operation.

Automatic operation can be enabled again by sending the `automatic on` command.

One limitation of the current implementation is that the automatic controller uses a single shared variable to track the state of both room LEDs. Since manual commands can control the LEDs independently, the stored shared state may not always match the actual state of both LEDs. This is an area that could be improved in a future version.

## 8. Bluetooth Low Energy Communication

BLE provides the wireless communication link between the Raspberry Pi and the Arduino.

The Arduino advertises itself using the device name `SmartLighting`. The BLE service contains two characteristics, each with a different purpose.

### 8.1 Command Characteristic

The command characteristic receives text commands from the Raspberry Pi.

Examples include commands to switch an LED on or off, enable automatic lighting, or request a sensor reading.

The program checks whether a new value has been written to this characteristic. When a command is received, it passes the text to the command-processing function.

### 8.2 Response Characteristic

The response characteristic is used to provide information to the Raspberry Pi.

Depending on the command, the response may contain a status message, an error message, or a light-level reading.

The characteristic supports reading and notifications. Notifications can inform subscribed clients when its value changes, although the Raspberry Pi must subscribe to notifications to receive them that way.

### 8.3 Communication Process

The communication process follows these steps:

1. The Raspberry Pi discovers the Arduino through BLE advertising.
2. The Raspberry Pi connects to the Arduino.
3. The Raspberry Pi writes a command to the command characteristic.
4. The Arduino detects the incoming command.
5. The command is passed to the command-processing function.
6. The relevant LED operation or sensor request is performed.
7. The Arduino updates the response characteristic.

Both programs must use matching BLE configuration and compatible command strings for communication to work correctly.

## 9. Supported Commands

The Arduino recognises the following commands.

| Command | Description |
|---|---|
| `bathroom on` | Turns the bathroom LED on and disables automatic mode |
| `bathroom off` | Turns the bathroom LED off and disables automatic mode |
| `hallway on` | Turns the hallway LED on and disables automatic mode |
| `hallway off` | Turns the hallway LED off and disables automatic mode |
| `automatic on` | Enables automatic lighting |
| `automatic off` | Disables automatic lighting |
| `fan on` | Turns the fan indicator LED on |
| `fan off` | Turns the fan indicator LED off |
| `light` | Requests a light-level reading |

The command-processing function removes leading and trailing spaces and converts received commands to lowercase before comparing them with the supported command strings.

This makes command matching less sensitive to capitalisation and unnecessary spaces around the command.

If a command does not match any supported option, the program returns an `Unknown command` response and prints a corresponding message in the Serial Monitor.

## 10. Sensor Integration

The BH1750 sensor measures the amount of ambient light and reports its reading in lux.

During startup, the program attempts to initialise the sensor. The result is stored in the `sensorReady` variable.

If the sensor initialises successfully, the program can read its measurements. If initialisation fails, a warning is printed and sensor-dependent operations are skipped.

The program also checks the result of a sensor reading before returning it in response to a `light` command. A negative reading is treated as an error rather than a valid light measurement.

The sensor is used for two purposes:

- Providing light-level information through BLE when requested.
- Supplying measurements to the automatic lighting controller.

This means the sensor contributes both to monitoring the environment and to making automatic control decisions.

## 11. Error Handling and Fault Management

Error handling is important because hardware and communication systems do not always operate as expected.

The current sketch includes basic checks for several conditions.

### 11.1 Sensor Initialisation Failure

If the BH1750 cannot be initialised, the program prints a warning to the Serial Monitor and records that the sensor is unavailable.

The sensor-dependent code checks this status before attempting to use the sensor.

This helps prevent the program from treating an unavailable sensor as if it were functioning normally.

### 11.2 Invalid Sensor Reading

When the Raspberry Pi requests a light-level reading, the program checks whether the returned value is valid.

If the reading is negative, the Arduino returns a sensor error message instead of sending the invalid number as a normal measurement.

### 11.3 Unsupported Commands

If the received command does not match any supported command, the program returns an `Unknown command` response.

This makes it easier to identify incorrect command strings during testing.

### 11.4 BLE Initialisation Failure

If BLE cannot start, the program prints an error message and stops normal execution.

This is a basic failure-handling mechanism because BLE is required for communication with the Raspberry Pi.

### 11.5 Limitations of Fault Handling

The current sketch provides basic error detection and reporting, but it does not recover automatically from every possible fault.

For example, the Arduino sketch does not implement a dedicated automatic BLE reconnection mechanism. Sensor disconnection after successful initialisation also requires further testing to understand the behaviour in that situation.

These limitations should be considered when evaluating the reliability of the system. Future improvements could include stronger sensor availability checks, more complete recovery logic, and better tracking of individual LED states.

## 12. Testing and Verification

Testing should cover the normal operation of the system as well as its response to incorrect commands and hardware or communication problems.

The following test plan identifies the main areas that should be checked.

| Test | Procedure | Expected behaviour |
|---|---|---|
| Startup | Reset or power on the Arduino | Startup and initialisation messages appear in the Serial Monitor |
| Sensor readings | Observe the system under different lighting conditions | The Serial Monitor displays ambient light measurements in lux |
| Bathroom control | Send bathroom ON and OFF commands | The bathroom LED follows the requested command |
| Hallway control | Send hallway ON and OFF commands | The hallway LED follows the requested command |
| Automatic lighting | Test below the ON threshold and above the OFF threshold | The two room LEDs respond according to the configured thresholds |
| Hysteresis | Observe the light state between 20 and 40 lux | The automatic controller does not switch the lights based on readings in this interval |
| Fan indicator | Send fan ON and OFF commands | The fan indicator LED follows the command |
| Light-level request | Send the `light` command | The response contains a valid lux reading when the sensor is available |
| Invalid command | Send an unsupported command | The program returns an unknown-command response |
| Sensor unavailable | Test with the sensor unavailable during startup | The program reports that the sensor was not detected |
| BLE connection | Connect from the Raspberry Pi and send a supported command | The Arduino receives and processes the command |
| BLE disconnection | Disconnect the Raspberry Pi during operation | The observed behaviour is checked and documented |

The expected behaviours in this table describe what should be verified. They should not be interpreted as proof that every test has passed.

Testing evidence include photographs of the assembled circuit, Serial Monitor output, Arduino IDE screenshots, and a demonstration of commands being sent from the Raspberry Pi.

Recording the actual observations is useful for identifying differences between expected and observed behaviour. Any unexpected result should be investigated before the system is described as fully reliable.

## 13. Setup and Upload Instructions

The following steps explain how to prepare the Arduino sketch.

1. Install the Arduino IDE.
2. Install the ArduinoBLE library through the Arduino IDE Library Manager.
3. Install the BH1750 library by Christopher Laws through the Library Manager.
4. Connect the Arduino Nano 33 IoT to the computer using a USB cable.
5. Open the `SmartLighting.ino` sketch.
6. Select the Arduino Nano 33 IoT board in the board selection menu.
7. Select the correct serial port.
8. Compile the sketch and check for any errors.
9. Upload the sketch to the Arduino.
10. Open the Serial Monitor and select a baud rate of 9600.
11. Check the startup messages and sensor readings.
12. Connect the Raspberry Pi and test the supported BLE commands.

If the sensor is not detected, check its power, ground, SDA and SCL connections before investigating the software.

If BLE communication fails, check that the Arduino is powered on, advertising, and using the same BLE configuration as the Raspberry Pi program.

## 14. Integration with the Raspberry Pi

The Arduino and Raspberry Pi perform different responsibilities within the complete system.

The Raspberry Pi handles microphone input, speech recognition, and the transmission of commands. The Arduino receives those commands and performs the relevant operation on the connected hardware.

For example, when the user requests that the hallway light be turned on, the Raspberry Pi processes the spoken request and sends the corresponding command to the Arduino. The Arduino then processes the command and switches the hallway LED on.

The Arduino can also return sensor readings or status information through BLE. This allows the Raspberry Pi to request information from the embedded controller.

The two components therefore work together to provide voice-based control while keeping the hardware operations on the Arduino.

## 15. Project Evidence and Documentation

The following evidence are included in the project repository to explain the implementation clearly:

- A photograph of the assembled Arduino circuit.
- A screenshot of the Arduino IDE showing the sketch.
- A Serial Monitor screenshot showing the startup messages and light readings.
- A circuit diagram showing the LED and BH1750 connections.
- A system architecture diagram showing communication between the Raspberry Pi and Arduino.
- A testing document containing the actual test procedures, observations, and outcomes.
- Demonstration evidence showing manual commands and automatic lighting behaviour.
- 
## 16. Limitations and Future Improvements

Although the current implementation provides the main control functions, several improvements could make the system more reliable.

### 16.1 Individual LED State Tracking

The current automatic controller uses one shared variable to track the state of both room LEDs. Manual commands can control the LEDs independently, so the shared variable may become inconsistent with their actual states.

### 16.2 Improved Sensor Fault Detection

The sketch checks whether the sensor initialised successfully and handles negative readings during a light-level request. Additional checks could help identify a sensor that becomes disconnected after startup.

### 16.3 Improved BLE Recovery

A future version could include more explicit connection monitoring and recovery behaviour if BLE communication is interrupted.

### 16.4 Expanded Testing

Additional tests could examine repeated commands, controller restarts, sensor disconnection, and changing light levels near the thresholds.

These improvements would help establish how the system behaves under different conditions rather than evaluating it only during normal operation.

## 17. Conclusion

The Arduino Smart Lighting Controller combines LED control, ambient light sensing, and Bluetooth Low Energy communication in a single embedded program.

The bathroom and hallway LEDs can be controlled manually, while automatic mode uses the BH1750 sensor to respond to changes in ambient light. The fan indicator provides an additional example of digital output control, and the BLE response characteristic allows information to be returned to the Raspberry Pi.

The project demonstrates how a microcontroller can interact with sensors and external devices as part of a larger voice-controlled system. Its existing error checks provide basic fault reporting, while improved state tracking, sensor fault detection, communication recovery, and systematic testing remain useful areas for further development.
