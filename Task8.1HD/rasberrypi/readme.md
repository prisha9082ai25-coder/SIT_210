# Raspberry Pi Voice Control System

## Overview

This part of the project runs on a Raspberry Pi 4. It uses a USB microphone to recognise spoken commands and sends the commands to the Arduino Nano 33 IoT through Bluetooth Low Energy (BLE).

The Raspberry Pi handles voice input, while the Arduino controls the bathroom and hallway LEDs and reads the ambient light level using a BH1750 light sensor.

## Hardware Used

- Raspberry Pi 4 Model B
- USB microphone
- Arduino Nano 33 IoT
- Bluetooth Low Energy connection
- Bathroom LED with a current-limiting resistor
- Hallway LED with a current-limiting resistor
- BH1750 ambient light sensor
- Fan indicator LED

The fan is represented by an LED indicator in this prototype. It does not control a real fan.

## Software and Libraries

- Python 3
- SpeechRecognition
- PyAudio
- Bleak
- ArduinoBLE on the Arduino
- BH1750 library on the Arduino

## How It Works

1. The microphone captures the user's speech.
2. The Python program processes the audio and attempts to recognise the spoken words.
3. The recognised command is checked against the supported commands.
4. The Raspberry Pi sends the command to the Arduino over BLE.
5. The Arduino performs the requested action and can return a response.
6. The user can check the result through the program output and the physical LED indicators.

## Example Voice Commands

Commands depend on the version of the Python program and Arduino firmware being used. The intended commands include:

- Bathroom on
- Bathroom off
- Hallway on
- Hallway off
- Fan on
- Fan off
- Automatic on
- Automatic off
- Light

## Automatic Lighting

The Arduino uses the BH1750 sensor to measure ambient light. In automatic mode, it switches the room LEDs according to the configured light thresholds.

The current Arduino firmware uses:
- Below 20 lux: turn the room lights on.
- Above 40 lux: turn the room lights off.

The gap between the thresholds helps prevent the lights from repeatedly switching near one brightness level.

## Communication

The Raspberry Pi and Arduino communicate using Bluetooth Low Energy. The Pi sends text commands, and the Arduino processes them through its BLE command characteristic.

Voice recognition through Google's recognition service requires an internet connection. BLE communication itself operates locally.

## Setup

1. Install Python 3 and the required audio dependencies on the Raspberry Pi.
2. Create and activate a Python virtual environment.
3. Install the Python packages listed in `requirements.txt`.
4. Upload the Arduino firmware from the `Arduino` folder.
5. Confirm that the Arduino is advertising over BLE.
6. Check the BLE address and service configuration.
7. Run the Python voice-control program.
8. Test the supported commands and observe the LEDs.

## Testing

Testing should cover microphone input, speech recognition, BLE connection, command handling, LED operation, automatic lighting, and unexpected or unrecognised commands.

Record the actual results of each test after performing it. Do not assume that a test has passed without checking it.

## Limitations

- Speech recognition can be affected by background noise.
- Online speech recognition requires internet access.
- BLE connection failures can prevent commands from reaching the Arduino.
- Automatic lighting depends on the sensor readings and configured thresholds.
- The fan is an LED indicator rather than a working fan.

## Future Improvements



Improved voice feedback, better error handling, logging of test results and more commands could be added.



## Conclusion



This has demonstrated a voice controlled smart lighting prototype. The Raspberry Pi offers the voice control interface whilst the Arduino offers the lighting hardware and ambient light sensor. This was done using Python and Bluetooth Low Energy.
