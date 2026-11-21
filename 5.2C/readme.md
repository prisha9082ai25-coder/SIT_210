# SIT210 Task 5.2C – Change the Light Intensity Using GUI

**Student Name:** Prisha Khera  
**Student ID:** 2510999082  
**Unit:** SIT210 – Embedded Systems Development  
**Task:** 5.2C – Change the Light Intensity Using GUI

---

## 1. Project Overview

This project implements a Raspberry Pi-based smart lighting control system using Python and a Graphical User Interface (GUI).

Building upon Task 5.1P, this project introduces an additional feature that allows users to adjust the brightness of the living room LED using a slider.

The system uses Tkinter to create the graphical interface and GPIO Zero to control the Raspberry Pi GPIO pins. Pulse Width Modulation (PWM) is used to adjust the brightness of the living room LED, while the bathroom and closet LEDs operate through digital ON/OFF control.

The application provides a simple and interactive way to manage multiple room lights through a single interface.

## 2. Project Objectives

- Develop a GUI-based lighting control application using Python and Tkinter.
- Control multiple LEDs connected to Raspberry Pi GPIO pins.
- Implement PWM to adjust the living room LED brightness.
- Provide brightness adjustment between 0% and 100%.
- Control bathroom and closet LEDs using digital GPIO outputs.
- Integrate hardware and software into a functional embedded system.

## 3. Features

- **Living Room Brightness Control:** Adjust LED intensity using a graphical slider.
- **PWM Implementation:** Control brightness by changing the PWM duty cycle.
- **Bathroom Light Control:** Switch the bathroom LED ON and OFF.
- **Closet Light Control:** Switch the closet LED ON and OFF.
- **Interactive GUI:** Manage the lighting system through a Tkinter interface.
- **Exit Functionality:** Turn off the LEDs before closing the application.

## 4. Hardware Components

| Component | Purpose |
|---|---|
| Raspberry Pi | Main processing and control unit |
| MicroSD Card | Operating system and program storage |
| LEDs | Represent room lighting |
| Resistors | Limit current through LEDs |
| Breadboard | Circuit assembly |
| Jumper Wires | Electrical connections |
| Monitor | Display the graphical interface |
| Keyboard and Mouse | User interaction |
| HDMI Cable | Display connection |
| Raspberry Pi Power Supply | Power the Raspberry Pi |

## 5. GPIO Pin Allocation

| Component | GPIO Pin | Physical Pin | Function |
|---|---|---|---|
| Living Room LED | GPIO18 | Pin 12 | PWM brightness control |
| Bathroom LED | GPIO27 | Pin 13 | Digital ON/OFF |
| Closet LED | GPIO22 | Pin 15 | Digital ON/OFF |
| Ground | GND | Pin 6 | Common ground |

GPIO18 is used for PWM because it supports hardware PWM functionality on the Raspberry Pi.

## 6. Software and Technologies

| Technology | Purpose |
|---|---|
| Raspberry Pi OS | Operating system |
| Python 3 | Main programming language |
| Tkinter | Graphical user interface |
| GPIO Zero | GPIO and LED control |
| Geany | Code development environment |

## 7. System Architecture

The system follows this communication flow:

**User Input → Tkinter GUI → Python Program → Raspberry Pi GPIO → LED**

### Working Process

1. The user interacts with the Tkinter graphical interface.
2. The Python application receives the selected room or brightness value.
3. The program processes the user input.
4. The Raspberry Pi controls the corresponding GPIO pin.
5. The connected LED responds to the selected operation.

For the living room LED, PWM controls the brightness through GPIO18.

The bathroom and closet LEDs use GPIO27 and GPIO22 respectively for digital switching operations.

All communication takes place locally between the Python application and Raspberry Pi GPIO. An internet connection is not required.

## 8. Understanding PWM

Pulse Width Modulation (PWM) is used to control the brightness of the living room LED.

PWM works by rapidly switching a signal ON and OFF. The duty cycle determines the proportion of time the signal remains ON during each cycle.

The GUI slider allows the user to select a brightness percentage.

| Duty Cycle | Expected LED Behaviour |
|---|---|
| 0% | LED OFF |
| 25% | Low brightness |
| 50% | Medium brightness |
| 75% | High brightness |
| 100% | Maximum brightness |

Increasing the duty cycle increases the average power delivered to the LED, making it appear brighter.

## 9. GUI Functionality

The graphical interface provides the following controls:

- Living Room: Select and control the living room light.
- Bathroom: Switch the bathroom light ON or OFF.
- Closet: Switch the closet light ON or OFF.
- Brightness Slider: Adjust living room brightness between 0% and 100%.
- Exit Button: Close the application and switch off the LEDs.

The GUI makes the system easier to operate without requiring users to interact directly with the Python code or GPIO pins.

## 10. Circuit Connections

The LEDs are connected to the Raspberry Pi through the breadboard.

- Living Room LED: GPIO18.
- Bathroom LED: GPIO27.
- Closet LED: GPIO22.
- Ground: Raspberry Pi GND.

Each LED uses a suitable current-limiting resistor.

**Note:** Check the LED polarity and circuit connections before powering the Raspberry Pi.

## 11. Installation and Execution

### Step 1: Update Raspberry Pi

```bash
sudo apt update
sudo apt upgrade
```

### Step 2: Install Required Packages

```bash
sudo apt install python3 python3-gpiozero python3-tk
```

### Step 3: Create the Project File

Create a Python file for the lighting control application.

```bash
nano main.py
```

Add the project Python implementation and save the file.

### Step 4: Run the Application

```bash
python3 main.py
```

The Tkinter graphical interface should open, allowing the user to interact with the lighting controls.

**Note:** Replace `main.py` with the actual Python filename if your project uses a different filename.

## 12. Testing and Expected Results

The project can be tested through the following operations:

| Test | Expected Result |
|---|---|
| Select Living Room | Living room LED responds |
| Set brightness to 25% | LED displays low brightness |
| Set brightness to 50% | LED displays medium brightness |
| Set brightness to 100% | LED displays maximum brightness |
| Select Bathroom | Bathroom LED switches ON |
| Select Closet | Closet LED switches ON |
| Press Exit | Application closes and LEDs switch OFF |

The practical demonstration verifies the interaction between the graphical interface, Python program, Raspberry Pi GPIO pins, and connected LEDs.

## 13. Learning Outcomes

Through this project, I developed practical experience in:

- Python-based GUI development.
- Raspberry Pi GPIO programming.
- PWM-based brightness control.
- Hardware and software integration.
- Embedded system architecture.
- User input processing and event handling.

This task demonstrates how a simple embedded system can provide flexible lighting control through a graphical interface.

## 14. Project Demonstration

The demonstration video covers:

1. Project introduction and objectives.
2. Hardware components and circuit connections.
3. GPIO pin allocation.
4. Software implementation.
5. System architecture and data flow.
6. Living room brightness adjustment using PWM.
7. Bathroom and closet LED control.
8. Exit functionality.

## 15. Repository Information

**Unit:** SIT210 – Embedded Systems Development  
**Assessment:** Task 5.2C  
**Project:** Change the Light Intensity Using GUI  
**Platform:** Raspberry Pi  
**Programming Language:** Python 3

---


