
# Low-Cost IoT-Based Two-Bed IV Drip Monitoring System

## SIT210 – Embedded Systems Development
### Task 11.1P/C/D/HD – Final Project Artefact

## 1. Project Overview

The Low-Cost IoT-Based Two-Bed IV Drip Monitoring
System is an embedded IoT prototype developed as
part of SIT210 Embedded Systems Development.

The project investigates the use of embedded
sensors, wireless communication and web-based
visualization to monitor two independent liquid
containers.

The Arduino Nano 33 IoT performs sensor
acquisition, measurement processing, flow
estimation and local warning generation.

A Raspberry Pi 4 receives measurements through
Bluetooth Low Energy and hosts a responsive
monitoring dashboard accessible through
computers and mobile devices.

The system combines weight-based measurements
with optical drop detection to provide multiple
sources of monitoring information.

This project uses water exclusively for academic
demonstration and is not intended for clinical
or patient use.

---

## 2. Background and Motivation

Manual observation of multiple fluid containers
can require repeated attention, particularly
in environments containing several beds.

Research into intravenous infusion administration
has identified errors and discrepancies associated
with infusion procedures.

Existing electronic monitoring systems demonstrate
the potential of microcontrollers, sensors and
automated warning mechanisms.

This project investigates these concepts through
an affordable two-bed embedded monitoring
prototype.

The proposed architecture combines independent
sensor channels, local warnings and centralized
wireless monitoring.

The system monitors measurements but does not
control liquid delivery.

Air bubble detection and clinical decision-making
are outside the project scope.

---

## 3. Project Objectives

- Monitor two beds independently.
- Measure liquid weight using load cells.
- Estimate remaining liquid volume and percentage.
- Reduce measurement fluctuations through filtering.
- Detect and count optical drops.
- Calculate drops per minute.
- Estimate flow using changes in weight.
- Support optical volumetric flow estimation.
- Compare available flow measurements.
- Detect low-volume and empty conditions.
- Detect interruptions in optical drop activity.
- Detect supported measurement faults.
- Handle bottle removal and disturbances.
- Provide visual and audible warning indications.
- Display local information through an OLED.
- Transmit measurements using Bluetooth LE.
- Develop a centralized monitoring dashboard.
- Support mobile monitoring.
- Support automatic Raspberry Pi startup.
- Investigate responsiveness, accuracy and reliability.

---

## 4. Functional Requirements

| ID | Requirement | Implementation Status |
|---|---|---|
| FR1 | Independent weight measurement for two beds | Implemented using two load cells and HX711 modules |
| FR2 | Optical drop detection and counting | Implemented; quantitative accuracy validation pending |
| FR3 | Weight filtering and remaining volume estimation | Implemented |
| FR4 | Weight-based and optical flow estimation and comparison | Partially implemented; optical calibration pending |
| FR5 | Local display and audible warnings | Implemented using OLED, LEDs and buzzer |
| FR6 | Arduino to Raspberry Pi communication | Implemented using BLE instead of USB serial |
| FR7 | Dashboard with individual bed information, flow, warnings and target comparison | Implemented; optical flow fields depend on calibration |
| FR8 | Disturbance and bottle replacement handling | Implemented; further validation required |

---

## 5. Non-Functional Requirements

| ID | Requirement | Implementation |
|---|---|---|
| NFR1 | Warning response within five seconds | Local processing and periodic updates; formal timing validation pending |
| NFR2 | Independent local operation | Arduino monitoring does not depend on Raspberry Pi connectivity |
| NFR3 | Fault tolerance | Sensor checks, disturbance handling, reconnection and stale-data detection |
| NFR4 | Low cost | Commonly available development boards and sensors |
| NFR5 | Scalability | Independent bed structures and modular software |

---

## 6. Hardware Components

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino Nano 33 IoT | 1 | Embedded sensor processing and BLE |
| Raspberry Pi 4 | 1 | Data reception and dashboard hosting |
| Four-wire load cell | 2 | Weight measurement |
| HX711 module | 2 | Load cell amplification and digital conversion |
| HW-201 IR sensor | 2 | Optical drop detection |
| SSD1306 OLED 128x64 | 1 | Local monitoring display |
| KC-1206 buzzer module | 1 | Audible warnings |
| LED | 2 | Independent bed warnings |
| 330-ohm resistor | 2 | LED current limiting |
| Water demonstration bottle | 2 | Demonstration liquid containers |
| Breadboard | As required | Circuit assembly |
| Jumper wires | As required | Electrical connections |
| Support stand | 1 | Supporting the demonstration assembly |
| Power supplies and USB cables | As required | Controller power and programming |

---

## 7. Software and Technologies

### Arduino

- Arduino IDE
- C/C++
- HX711 library
- ArduinoBLE library
- Wire library
- Adafruit GFX library
- Adafruit SSD1306 library

### Raspberry Pi

- Linux
- Python 3
- Flask
- Bleak
- Asyncio
- Threading
- Struct
- Collections
- Datetime
- Systemd

### Web Dashboard

- HTML
- CSS
- JavaScript
- JSON
- HTTP APIs
- HTML Canvas

### Communication Interfaces

| Interface | Application |
|---|---|
| Differential electrical signal | Load cell to HX711 |
| DOUT and SCK | HX711 to Arduino |
| Digital GPIO | IR sensors and warning outputs |
| I2C | OLED communication |
| Bluetooth Low Energy | Arduino to Raspberry Pi |
| Wi-Fi / local network | Remote dashboard access |
| HTTP | Flask and browser communication |
| USB serial | Programming and debugging |

---

## 8. System Architecture

The system follows a distributed embedded
architecture in which local monitoring and
remote visualization are separated.

```text
BED 1                         BED 2
  |                             |
Load Cell 1                   Load Cell 2
  |                             |
HX711 Module 1                HX711 Module 2
  |                             |
  +-------------+---------------+
                |
IR Sensor 1 --->|<--- IR Sensor 2
                |
       Arduino Nano 33 IoT
                |
      +---------+----------+
      |                    |
 Local Outputs       Bluetooth LE
      |                    |
 OLED Display        Raspberry Pi 4
 Warning LEDs              |
 Buzzer              Python Backend
                           |
                      Flask Server
                           |
                     Local Network
                           |
                  +--------+--------+
                  |                 |
           Computer Dashboard   Mobile Dashboard
```

### System Data Flow

1. Sensors generate measurement signals.
2. Arduino acquires the sensor readings.
3. Weight measurements are calibrated and filtered.
4. Remaining volume and flow are estimated.
5. Individual bed conditions are evaluated.
6. OLED, LEDs and buzzer are updated.
7. Measurement packets are transmitted through BLE.
8. Raspberry Pi receives and decodes the packets.
9. Flask provides measurements through HTTP APIs.
10. JavaScript updates the dashboard and graphs.

---

## 9. Hardware Wiring Documentation

### Arduino Pin Configuration

| Component | Signal | Arduino Pin |
|---|---|---|
| HX711 Bed 1 | DT / DOUT | D2 |
| HX711 Bed 1 | SCK | D3 |
| HX711 Bed 2 | DT / DOUT | D4 |
| HX711 Bed 2 | SCK | D5 |
| IR Sensor Bed 1 | OUT | D6 |
| IR Sensor Bed 2 | OUT | D7 |
| Warning LED Bed 1 | Signal | D8 |
| Warning LED Bed 2 | Signal | D9 |
| Buzzer | Signal | D10 |
| OLED | SDA | A4 |
| OLED | SCL / SCK | A5 |

The Arduino Nano 33 IoT operates using 3.3V logic.

### Load Cell Interface

Each load cell is connected to an independent
HX711 module.

| HX711 Terminal | Function |
|---|---|
| E+ | Positive excitation |
| E- | Negative excitation |
| A+ | Positive measurement signal |
| A- | Negative measurement signal |

Load cell wire colours are manufacturer-dependent.

Both HX711 modules use the 3.3V-compatible
configuration and share a common ground.

### Optical Sensors

The two HW-201 sensors provide independent
digital optical detection signals.

The firmware uses active-LOW detection.

### OLED Configuration

- Controller: SSD1306
- Resolution: 128x64
- Communication: I2C
- Address: 0x3C
- SDA: A4
- SCL: A5

### Warning Outputs

Two LEDs provide individual bed warnings.

Each LED uses a current-limiting resistor.

The shared buzzer module uses D10 and an
active-HIGH firmware configuration.

Module voltage and current compatibility must
be checked before reproducing the hardware.

All wiring changes must be performed with
power disconnected.

---

## 10. Load Cell Working Principle

A load cell converts mechanical force into
an electrical signal using strain gauges.

When a mechanical load is applied, the
strain gauges experience a change in resistance.

A Wheatstone bridge arrangement produces
a small differential voltage corresponding
to the applied force.

The HX711 amplifies this signal and converts
it into a digital measurement.

The Arduino reads this digital value and
converts it into an estimated weight using
the calibration parameters.

Independent calibration is necessary because
the two load cells have different electrical
and mechanical characteristics.

---

## 11. Sensor Calibration

Calibration was performed using unloaded
load cells, empty demonstration bottles and
bottles containing approximately 500 mL of water.

### Recorded Calibration Values

| Parameter | Bed 1 | Bed 2 |
|---|---:|---:|
| Zero offset | 243471.75 | -154616.50 |
| Empty bottle reading | 254162.50 | -137512.25 |
| Full bottle reading | 453046.00 | 40442.75 |
| Calibration factor | 397.767 | 355.910 |
| Estimated empty bottle weight | 26.88 g | 48.06 g |

### Calibration Factor

Calibration Factor =
(Full Bottle Reading - Empty Bottle Reading)
 / Reference Water Weight

The reference water weight was approximately
500 grams.

### Total Weight

Total Weight =
(Raw Reading - Zero Offset) / Calibration Factor

### Remaining Water Weight

Water Weight = Total Weight - Empty Bottle Weight

### Remaining Volume

For the water-only demonstration:

1 gram of water is treated as approximately
1 millilitre.

Remaining Volume (mL) = Water Weight (g)

### Remaining Percentage

Remaining Percentage =
(Remaining Volume / 500) × 100

The displayed percentage is restricted to
the range of 0 to 100 percent.

The calibration values are specific to the
original prototype hardware and mounting
arrangement.

---

## 12. Arduino Firmware Implementation

The Arduino firmware contains the complete
embedded monitoring logic.

### 12.1 Independent Bed Structures

Each bed maintains its own:

- Raw load cell reading.
- Total weight.
- Bottle weight.
- Remaining water weight.
- Estimated volume.
- Remaining percentage.
- Cumulative drop count.
- Drops per minute.
- Flow measurements.
- Sensor status.
- Bottle presence.
- Disturbance state.
- Measurement timestamps.
- Warning status.

This structure allows independent processing
of both monitoring channels.

### 12.2 Weight Measurement

The Arduino checks whether each HX711 is
ready before reading its digital measurement.

Raw readings are converted into weight
using the corresponding calibration factor.

Unavailable or invalid readings are handled
through the sensor fault logic.

### 12.3 Exponential Moving Average

An Exponential Moving Average is used to
reduce measurement fluctuations.

Filtered Weight =
Alpha × Current Weight
+ (1 - Alpha) × Previous Filtered Weight

The smoothing coefficient is 0.20.

Filtering improves measurement stability
but may introduce some response delay.

### 12.4 Optical Drop Detection

The IR sensors are monitored using polling.

The firmware detects active-LOW optical events
and increments the corresponding bed counter.

To reduce repeated detection, the firmware uses:

- Minimum accepted drop interval of 250 ms.
- Clear HIGH period of 30 ms before rearming.
- Independent drop counters.
- Independent event timestamps.

The final implementation uses polling rather
than the originally proposed interrupt-based
detection method.

### 12.5 Disturbance Detection

Sudden weight changes greater than 30 grams
are treated as potential disturbances.

Flow measurements are temporarily invalidated
until the weight measurement stabilizes.

This reduces the effect of mechanical movement
on the estimated flow.

### 12.6 Bottle Removal and Replacement

Bottle presence is evaluated using a threshold
derived from the empty bottle weight.

When removal is detected, the current flow
measurement state is reset.

When a bottle is detected again, the relevant
measurement state is reinitialized.

### 12.7 Firmware Timing Parameters

| Parameter | Value |
|---|---:|
| Minimum accepted drop interval | 250 ms |
| Optical rearming period | 30 ms |
| Measurement reporting interval | 2 seconds |
| Flow measurement window | 15 seconds |
| No-drop timeout | 30 seconds |
| HX711 sensor timeout | 3 seconds |
| OLED refresh interval | 1 second |
| EMA coefficient | 0.20 |
| Disturbance threshold | 30 g |
| Low-volume threshold | 100 mL |
| Empty threshold | 5 mL |

These values are academic demonstration
parameters rather than clinical specifications.

---

## 13. Program Flow

### Arduino Program Flow

```text
START
  |
Initialize HX711 modules and optical sensors
  |
Initialize OLED, LEDs and buzzer
  |
Initialize Bluetooth LE
  |
Poll optical sensors
  |
Check HX711 availability
  |
Acquire raw weight measurements
  |
Convert raw readings into weight
  |
Check bottle presence and disturbances
  |
Apply measurement filtering
  |
Calculate remaining volume
  |
Calculate available flow measurements
  |
Evaluate individual bed status
  |
Update local warning outputs
  |
Refresh OLED
  |
Transmit BLE packets when due
  |
Repeat
```

### Raspberry Pi Program Flow

```text
START
  |
Initialize Flask
  |
Start Bluetooth background thread
  |
Discover Arduino
  |
Establish BLE connection
  |
Subscribe to data characteristic
  |
Receive measurement packets
  |
Validate and decode packets
  |
Update individual bed measurements
  |
Store recent measurement history
  |
Provide data through Flask APIs
  |
Update browser dashboard
  |
Monitor connection and data freshness
  |
Reconnect if communication is lost
```

---

## 14. Flow Rate Estimation

### 14.1 Weight-Based Flow

Weight-based flow is estimated using the
decrease in remaining water volume over time.

Flow (mL/h) =
Volume Lost (mL) / Elapsed Time (hours)

The measurement window is 15 seconds.

Measurements are invalidated during detected
disturbances, bottle removal and sensor faults.

### 14.2 Drops Per Minute

Drops Per Minute =
Accepted Drops / Elapsed Time (minutes)

This calculation does not require a volumetric
drop factor.

### 14.3 Optical Volumetric Flow

The supported optical calculation is:

Optical Flow (mL/h) =
(Drops Per Minute × 60) / Drop Factor

The optical drop factors have not yet been
experimentally calibrated.

Consequently, optical volumetric flow is
reported as unavailable.

### 14.4 Flow Comparison

When both estimates are available:

Flow Difference =
Absolute Value of (Weight Flow - Optical Flow)

The numerical comparison remains pending
optical calibration.

### 14.5 Demonstration Target Comparison

The dashboard supports an independently entered
reference flow value for each bed.

Target Difference =
Measured Weight Flow - Entered Reference Flow

The entered value is for academic comparison
and does not control liquid delivery.

---

## 15. Warning and Fault Detection

| Status | Description |
|---|---|
| NORMAL | No active warning detected |
| LOW_VOLUME | Remaining volume at or below 100 mL, above the empty threshold |
| EMPTY | Remaining volume at or below 5 mL |
| NO_DROPS | Configured no-drop timeout has elapsed |
| BOTTLE_REMOVED | Bottle removal detected |
| SENSOR_FAULT | Detectable HX711 measurement fault |
| DISTURBANCE | Significant weight disturbance detected |
| INITIALIZING | Measurement state is initializing |

The firmware prioritizes measurement faults,
bottle removal and disturbances before
volume and no-drop conditions.

The NO_DROPS status does not independently
distinguish between interrupted flow and
an optical sensor detection problem.

---

## 16. OLED Display, LEDs and Buzzer

### OLED Display

The SSD1306 OLED displays:

- Bed identifiers.
- Remaining volume.
- Remaining percentage.
- Individual bed status.
- Graphical liquid level indicators.
- Bluetooth or local-operation indication.

The OLED refreshes approximately once
per second.

### LED Indicators

Each bed has an independent warning LED.

The corresponding LED is activated when
the bed reports a status other than NORMAL.

### Buzzer

A shared buzzer generates periodic audible
warnings when either bed reports an active
warning.

The buzzer uses millis-based timing rather
than a deliberately blocking delay.

### Independent Local Monitoring

Local sensing and warning generation are
performed by the Arduino.

The Arduino can continue local monitoring
when the Raspberry Pi is unavailable,
provided the Arduino remains powered.

---

## 17. Bluetooth Low Energy Communication

Bluetooth Low Energy provides wireless
communication between both controllers.

### BLE Roles

Arduino Nano 33 IoT:

- BLE peripheral.
- Advertises the monitoring service.
- Transmits measurement notifications.

Raspberry Pi:

- BLE central.
- Discovers the Arduino.
- Establishes a connection.
- Subscribes to measurement notifications.

### BLE Configuration

Device Name:

`IV_Monitoring`

Service UUID:

`7a2b0001-6d5a-4c4e-9a10-123456789001`

Data Characteristic UUID:

`7a2b0002-6d5a-4c4e-9a10-123456789001`

### Binary Packet Structure

A 20-byte binary packet is transmitted
for each bed.

| Byte Position | Measurement |
|---|---|
| 0 | Bed identifier |
| 1 | Status code |
| 2 | Sensor availability |
| 3 | Bottle presence |
| 4–5 | Remaining volume |
| 6–7 | Remaining percentage |
| 8–11 | Cumulative drop count |
| 12–13 | Drops per minute |
| 14–15 | Weight-based flow |
| 16–17 | Optical flow |
| 18–19 | Total measured weight |

The Python decoding format is:

`<BBBBHHIHHHH`

The packet uses little-endian byte ordering.

Decimal measurements are encoded using
a scaling factor of ten.

The reserved value 65535 represents
unavailable or invalid numerical measurements.

### Connection Recovery

The Arduino supports advertising again
after disconnection.

The Raspberry Pi repeatedly attempts to
discover and reconnect to the Arduino.

Successful reconnection has been observed,
although intermittent disconnections remain
an area requiring further investigation.

---

## 18. Raspberry Pi Backend

The Raspberry Pi application is developed
using Python and Flask.

### Bleak

The Bleak library provides BLE discovery,
connection and notification handling.

### Asyncio

Asyncio manages asynchronous Bluetooth
communication operations.

### Struct

The struct library decodes the received
binary measurement packets.

Packets with incorrect lengths or invalid
bed identifiers are ignored.

### Multithreading

Bluetooth communication runs in a background
thread while Flask handles web requests.

### Thread Synchronization

A thread lock protects shared measurement
data during concurrent access.

### Measurement History

A deque stores up to 150 recent measurements
independently for each bed.

The history is stored in memory and is
lost when the application restarts.

### Stale Data Detection

Measurements older than eight seconds
are marked as stale.

Live measurements are cleared when
Bluetooth communication is disconnected.

---

## 19. Web Monitoring Dashboard

The dashboard is developed using HTML,
CSS and JavaScript.

It provides independent monitoring cards
for both beds.

### Dashboard Features

- Bluetooth connection status.
- Individual bed status.
- Graphical liquid level indicators.
- Remaining volume.
- Remaining percentage.
- Total measured weight.
- Cumulative drop count.
- Drops per minute.
- Weight-based flow.
- Optical flow when calibrated.
- Difference between flow estimates.
- Demonstration target flow input.
- Target difference.
- Load cell status.
- Bottle attachment status.
- Last measurement timestamp.
- Remaining volume history graph.

Unavailable measurements are explicitly
identified rather than replaced with
unsupported values.

### Flask API Endpoints

| Endpoint | Purpose |
|---|---|
| `/` | Main dashboard page |
| `/api/data` | Latest bed measurements |
| `/api/history` | Recent measurement history |

The browser periodically requests updated
measurements and refreshes the interface.

The dashboard uses colour-coded statuses
to distinguish normal conditions from warnings.

---

## 20. Mobile Monitoring

The dashboard supports responsive layouts
for desktop and mobile devices.

The Raspberry Pi hosts the Flask server
on port 5000.

A device connected to the same trusted
local network can access the dashboard using:

`http://RASPBERRY_PI_IP:5000`

No dedicated mobile application is required.

The Raspberry Pi must remain powered
and connected to the network.

The demonstration server does not provide
authentication and must not be exposed
to the public internet.

---

## 21. Headless Raspberry Pi Operation

A systemd service was configured to
automatically launch the monitoring
application during Raspberry Pi startup.

The configuration supports:

- Automatic application startup.
- Background execution.
- Automatic process restart.
- Startup after Bluetooth and network services.

This allows the Raspberry Pi to operate
without a permanently connected monitor
or keyboard.

The Raspberry Pi must remain powered
for remote dashboard access.

---

## 22. Soft Real-Time System Design

The prototype follows a soft real-time
embedded system architecture.

Time-sensitive operations are handled
locally by the Arduino.

These include:

- Optical sensor polling.
- Weight acquisition.
- Warning evaluation.
- Local output updates.

The Raspberry Pi handles dashboard
visualization, measurement history
and network communication.

The firmware uses a continuous main
loop and millis-based scheduling.

A full Real-Time Operating System
is not implemented.

The original design specifies a
five-second warning response target.

Formal timing validation remains
necessary before claiming that the
target has been achieved.

---

## 23. Fault Tolerance

| Mechanism | Implementation Status |
|---|---|
| Independent local monitoring | Implemented |
| HX711 availability checks | Implemented |
| Invalid raw reading detection | Implemented |
| Weight filtering | Implemented |
| Disturbance detection | Implemented |
| Bottle removal handling | Implemented |
| Bottle replacement handling | Implemented |
| Repeated optical event filtering | Implemented |
| No-drop detection | Implemented |
| BLE reconnection | Implemented |
| Stale-data detection | Implemented |
| Raspberry Pi process restart | Implemented |
| Independent IR fault detection | Incomplete |
| Complete frozen-value detection | Incomplete |
| Automatic flow discrepancy alarm | Not implemented |
| Arduino hardware watchdog | Not implemented |

The system incorporates fault-handling
mechanisms, but complete fail-safe
operation has not been validated.

---

## 24. Engineering Design Principles

### Quick Response

Sensor processing and local warning
generation occur directly on the Arduino.

### Fail-Safe Design Intent

Detected measurement faults generate
warning statuses rather than silently
being presented as normal measurements.

### Degraded Mode

The Arduino can continue local monitoring
during Raspberry Pi communication loss.

### Redundancy

Weight measurements and optical detection
provide two independent sensing methods.

Quantitative flow cross-validation requires
completion of optical calibration.

### Modularity

Each bed maintains independent sensor
channels and measurement structures.

### Scalability

Additional monitoring nodes could be
supported through further hardware
and software development.

### Signal Processing

EMA filtering and timing-based optical
event filtering improve measurement handling.

### Usability

OLED information, LEDs, buzzer warnings
and dashboard visualization provide
multiple forms of feedback.

### Cost Effectiveness

The prototype uses commonly available
development boards and sensor modules.

---

## 25. Testing and Evaluation

The evaluation follows the testing methodology
specified in Section 4 of the submitted
Project Artefact Report.

Implementation observations are distinguished
from formal quantitative validation.

All testing is restricted to the water-only
academic demonstration setup.

### 25.1 Responsiveness Testing

The original report specifies 20 repetitions
for each warning condition.

| Test ID | Condition | Planned Trials | Target |
|---|---|---:|---|
| T01 | Low-volume warning | 20 | Under 5 seconds |
| T02 | Empty warning | 20 | Under 5 seconds |
| T03 | Load cell fault warning | 20 | Under 5 seconds |
| T04 | No-drop warning | 20 | Under 5 seconds after timeout |

The no-drop timeout is configured as
30 seconds in the final firmware.

The timeout itself is separate from
the warning response-time target.

The original Raspberry Pi offline
indication target is also subject
to further timing validation.

Complete repeated-trial response-time
measurements have not yet been documented.

### 25.2 Robustness and Fault Injection

The original evaluation specifies three
repetitions for each condition.

| ID | Test Condition | Expected Behaviour |
|---|---|---|
| R01 | Load cell unavailable | SENSOR_FAULT when detected |
| R02 | Optical sensor obstruction | Optical counts cease; no-drop warning may occur |
| R03 | Water flow interruption | NO_DROPS after timeout |
| R04 | Low and empty bottle | Corresponding volume warning |
| R05 | Raspberry Pi unavailable | Local monitoring continues |
| R06 | Arduino restart | System reinitializes |
| R07 | Mechanical disturbance | Disturbance detection and flow invalidation |
| R08 | Bottle replacement | Measurement state reinitializes |

The implemented fault-handling logic
supports these evaluation scenarios.

Complete individual trial results remain
subject to further documented testing.

### 25.3 Flow Accuracy Testing

The submitted report specifies the following
reference flow rates:

| Reference Flow | Weight-Based Estimate | Optical Estimate |
|---|---|---|
| 60 mL/h | Pending controlled validation | Not calibrated |
| 125 mL/h | Pending controlled validation | Not calibrated |
| 250 mL/h | Pending controlled validation | Not calibrated |

Absolute Error =
Absolute Value of (Estimated Value - Reference Value)

Percentage Error =
(Absolute Error / Reference Value) × 100

Percentage error is undefined for a
zero reference value.

### 25.4 Optical Drop Count Accuracy

The proposed evaluation compares manually
observed drops with automatically accepted
optical events over the same period.

The intended recorded measurements are:

- Manual drop count.
- Arduino drop count.
- Measurement duration.
- Absolute count error.
- Percentage count error.

Controlled optical count accuracy
measurements remain outstanding.

---

## 26. Experimental Results and Observations

### 26.1 Calibration Results

| Measurement | Bed 1 | Bed 2 |
|---|---:|---:|
| Unloaded raw average | 243471.75 | -154616.50 |
| Empty bottle raw average | 254162.50 | -137512.25 |
| Full bottle raw average | 453046.00 | 40442.75 |
| Calibration factor | 397.767 | 355.910 |

These measurements were obtained during
prototype calibration.

### 26.2 Integrated Dashboard Observation

The following measurements were observed
during an integrated monitoring session.

| Measurement | Bed 1 | Bed 2 |
|---|---:|---:|
| Remaining volume | 257.2 mL | 365.3 mL |
| Remaining percentage | 51.4% | 73.1% |
| Total weight | 284.1 g | 413.3 g |
| Cumulative drop count | 92 | 9 |
| Drops per minute | 0.0 | 0.0 |
| Weight-based flow | 11.6 mL/h | 0.0 mL/h |
| Optical volumetric flow | Unavailable | Unavailable |
| Flow difference | Unavailable | Unavailable |
| Load cell status | Working | Working |
| Bottle status | Attached | Attached |
| Warning status | NO_DROPS | NO_DROPS |

These observations demonstrate successful
measurement acquisition, Bluetooth
transmission and dashboard visualization.

They are not controlled flow accuracy results.

The non-zero weight-based flow estimate
during a no-drop condition illustrates
the potential influence of measurement
noise and other estimation limitations.

### 26.3 Implementation Observations

| Function | Observed Result |
|---|---|
| Dual load cell acquisition | Measurements obtained |
| Individual calibration | Calibration factors established |
| Remaining volume estimation | Numerical measurements displayed |
| Optical drop detection | Drop counts obtained |
| OLED initialization | Working |
| LED indicators | Working |
| Buzzer integration | Integrated and reported working |
| Bluetooth communication | Measurement packets received |
| Bluetooth reconnection | Successful reconnection observed |
| Flask dashboard | Working |
| Weight-based flow | Numerical estimates displayed |
| Mobile monitoring | Access demonstrated |
| Headless operation | Configured and demonstrated |
| Optical volumetric flow | Calibration pending |
| Dual flow comparison | Awaiting optical calibration |
| Formal response-time validation | Pending |
| Controlled accuracy evaluation | Pending |
| Extended reliability evaluation | Pending |

### 26.4 Evaluation Status

The submitted report refers to responsiveness,
robustness and accuracy records on GitHub.

Available implementation observations,
calibration measurements and collected
supporting evidence are documented here.

Complete repeated-trial measurements and
controlled accuracy results remain outstanding.

No unmeasured experimental results are
represented as successful tests.

---

## 27. Project Evidence

Photographs, screenshots and available
experimental records are organized in
the Evidence directory.

### Evidence Categories

- Complete hardware assembly.
- Arduino wiring and sensor connections.
- Load cell and HX711 integration.
- Calibration measurements.
- Optical drop detection.
- Arduino Serial Monitor readings.
- OLED display operation.
- LED and buzzer integration.
- Bluetooth communication.
- Raspberry Pi integration.
- Web dashboard.
- Warning status observations.
- Mobile monitoring.
- Headless operation.
- Available testing records.

[View Project Evidence](Evidence/)

The evidence directory contains the
available supporting material collected
during development and demonstration.

Screenshots demonstrate observed functionality,
while quantitative performance claims require
corresponding experimental measurements.

---

## 28. Original Design and Final Implementation

Several changes were introduced during
prototype development.

| Original Proposed Design | Final Implementation |
|---|---|
| 16x2 LCD | SSD1306 OLED |
| USB serial data communication | Bluetooth Low Energy |
| IR sensors on D2 and D3 | IR sensors on D6 and D7 |
| Interrupt-based drop counting | Polling with timing-based filtering |
| Serial heartbeat | BLE notifications |
| 10-second heartbeat timeout | BLE connection and 8-second stale-data detection |
| PI OFFLINE LCD message | OLED BLE/LOCAL indication |
| Persistent measurement logging | In-memory history |
| Two calibrated flow methods | Weight-based flow and optical counting; optical calibration pending |
| Automatic flow discrepancy warning | Numerical comparison supported when both estimates are available |
| Independent IR fault diagnostics | Incomplete |
| Arduino watchdog | Not implemented |

The final implementation retains the
main two-bed monitoring architecture
while incorporating these design changes.

---

## 29. Repository Structure

```text
11.1PCDHD/
|
|-- readme.md
|
|-- Arduino/
|   |
|   |-- IV_Monitoring_Final/
|       |-- IV_Monitoring_Final.ino
|
|-- Raspberry_Pi/
|   |
|   |-- app.py
|   |-- requirements.txt
|   |-- iv-monitoring.service
|   |
|   |-- templates/
|       |-- index.html
|
|-- Evidence/
    |
    |-- README.md
    |-- Project photographs
    |-- Screenshots
    |-- Available experimental records
```


## 30. User Manual

### Arduino Software Setup

1. Open the Arduino firmware in Arduino IDE.
2. Install the required Arduino libraries.
3. Select Arduino Nano 33 IoT.
4. Select the appropriate serial port.
5. Verify the hardware configuration.
6. Compile and upload the firmware.
7. Open Serial Monitor at 115200 baud.
8. Observe the initialization and sensor readings.

### Required Arduino Libraries

- HX711
- ArduinoBLE
- Adafruit GFX Library
- Adafruit SSD1306

The Wire library is provided by the
Arduino platform.

### Raspberry Pi Software Setup

Place the Raspberry Pi source files
inside the IV_Monitoring project directory.

Open the project directory:

```bash
cd ~/IV_Monitoring
```

Create a virtual environment:

```bash
python3 -m venv .venv
```

Activate the virtual environment:

```bash
source .venv/bin/activate
```

Install the required dependencies:

```bash
pip install -r requirements.txt
```

Run the monitoring application:

```bash
python app.py
```

Open the dashboard:

```text
http://localhost:5000
```

### Mobile Dashboard Access

Obtain the Raspberry Pi's local IP address:

```bash
hostname -I
```

Open the corresponding local address
on a device connected to the same network:

```text
http://RASPBERRY_PI_IP:5000
```

### Automatic Startup Configuration

The supplied systemd configuration must
be adjusted to match the Raspberry Pi
username and project directory.

Install the service:

```bash
sudo cp iv-monitoring.service /etc/systemd/system/
```

Reload systemd:

```bash
sudo systemctl daemon-reload
```

Enable automatic startup:

```bash
sudo systemctl enable iv-monitoring.service
```

Start the service:

```bash
sudo systemctl start iv-monitoring.service
```

Check service status:

```bash
systemctl status iv-monitoring.service
```

View application logs:

```bash
journalctl -u iv-monitoring.service -n 50 --no-pager
```

Do not run a second manual application
instance while the automatic service
is already active.

### Running the Academic Demonstration

The demonstration presents:

1. Complete hardware assembly.
2. Independent bed measurements.
3. Remaining water volume and percentage.
4. Optical drop counting.
5. OLED information.
6. Local warning outputs.
7. Bluetooth communication.
8. Raspberry Pi dashboard.
9. Demonstration target comparison.
10. Mobile monitoring.
11. Headless operation.
12. Current limitations.

Only water is used in the demonstration.

---

## 31. Troubleshooting

| Problem | Possible Cause | Recommended Check |
|---|---|---|
| HX711 unavailable | Connection or measurement issue | Check sensor availability and documented wiring with power disconnected |
| Incorrect weight | Calibration mismatch | Review calibration parameters |
| Fluctuating weight | Mechanical movement or noise | Review mounting stability and filtering |
| No optical counts | Sensor alignment or detection issue | Review optical sensor conditions |
| Repeated optical counts | Repeated optical events | Review timing-based filtering |
| OLED blank | Incorrect driver or address | Check SSD1306 library and address 0x3C |
| LED remains active | Active bed warning | Inspect the corresponding status |
| Buzzer active | At least one active warning | Inspect both bed statuses |
| Arduino not discovered | BLE unavailable | Check controller power and Bluetooth availability |
| BLE disconnects | Communication interruption | Review logs and reconnection behaviour |
| Dashboard unavailable | Flask application not running | Check systemd service |
| Mobile dashboard unavailable | Network or IP mismatch | Check local network and current IP |
| Optical flow unavailable | Drop factor not calibrated | Optical calibration remains required |
| Flow difference unavailable | Missing flow estimate | Review measurement availability |
| Stale data | No recent BLE measurements | Check communication status |
| Port already in use | Duplicate application process | Check automatic startup service |

### Bluetooth Status

```bash
bluetoothctl show
```

### Application Status

```bash
systemctl is-active iv-monitoring.service
```

### Live Application Logs

```bash
journalctl -u iv-monitoring.service -f
```

### Local API Verification

```bash
curl http://127.0.0.1:5000/api/data
```

This helps distinguish Flask server
availability from Bluetooth data availability.

---

## 32. Challenges and Solutions

### Load Cell Fluctuations

Initial raw measurements showed fluctuations
and inconsistent readings.

Individual calibration, EMA filtering and
disturbance detection were introduced to
improve measurement handling.

### Different Sensor Responses

Both load cells produced different raw
measurement responses.

Independent offsets and calibration factors
were established for each bed.

### Repeated Optical Detection

Repeated sensor events initially affected
drop counting.

Minimum event intervals and sensor
rearming conditions were introduced.

### OLED Initialization

The OLED initially failed to display
information.

An I2C scanner identified address 0x3C.

Independent testing established that the
SSD1306 library was compatible with
the display.

### Bluetooth Integration

Bluetooth communication was tested
independently before integrating the
complete monitoring firmware.

A binary packet structure was introduced
to transmit individual bed measurements.

### Bluetooth Disconnections

Intermittent communication interruptions
were observed during extended operation.

Automatic reconnection and stale-data
handling were implemented.

Long-term communication stability requires
further investigation.

### Unavailable Optical Flow

Optical volumetric flow remained unavailable
because the drop factor was not calibrated.

The dashboard explicitly identifies
unavailable measurements.

### Headless Operation

The Raspberry Pi initially required manual
application startup.

A systemd service was configured to
automatically launch the dashboard
during system boot.

---

## 33. Unit Learning Outcome Alignment

### ULO1 – Software Development

The project demonstrates software development
through Arduino firmware, Python programming
and web application implementation.

Concepts applied include:

- Structured programming.
- Functions and data structures.
- Conditional logic.
- Measurement processing.
- Binary encoding and decoding.
- Asynchronous programming.
- Multithreading.
- Exception handling.
- Web development.

### ULO2 – Embedded Systems

The project demonstrates embedded hardware
integration through:

- Load cell interfacing.
- HX711 communication.
- Optical sensor acquisition.
- Digital GPIO.
- I2C communication.
- OLED display integration.
- LED and buzzer control.
- Sensor calibration.
- Signal filtering.
- Embedded timing.
- Bluetooth Low Energy.

### ULO3 – System Design

The project integrates multiple hardware
and software components into a distributed
monitoring architecture.

System design considerations include:

- Modularity.
- Independent sensing channels.
- Local monitoring independence.
- Fault handling.
- Communication recovery.
- Data freshness.
- Responsiveness.
- Scalability.
- Usability.
- Reproducibility.

### Learning Summary

Through this project, I developed practical
experience in embedded hardware integration,
sensor calibration, signal processing,
wireless communication and IoT visualization.

The project improved my understanding of
how hardware and software components
interact within a complete embedded system.

It also demonstrated the importance of
experimental validation, clear documentation
and identifying implementation limitations.

---

## 34. Limitations and Future Improvements

The current prototype has the following
limitations and areas for further development:

- Optical drop factor calibration is pending.
- Dual flow comparison remains incomplete.
- Automatic flow discrepancy warning is absent.
- Independent optical sensor fault detection is incomplete.
- Optical counting accuracy requires validation.
- Polling may miss short optical events.
- Weight-based flow can be affected by noise.
- Bluetooth stability requires further investigation.
- Formal responsiveness testing remains outstanding.
- Repeated robustness testing remains outstanding.
- Controlled flow accuracy testing remains outstanding.
- Arduino hardware watchdog is not implemented.
- Measurement history is not persistent.
- Dashboard authentication is not implemented.
- Additional monitoring nodes have not been validated.
- Air bubble detection is outside the project scope.
- The prototype is not clinically validated.

Future improvements would focus on completing
calibration, improving measurement reliability,
strengthening fault diagnostics, implementing
persistent logging and validating system
performance through controlled experiments.

---

## 35. Demonstration Video

The demonstration video covers the project
architecture, hardware components, embedded
firmware, sensor measurements, local warnings,
Bluetooth communication, Raspberry Pi
integration and monitoring dashboard.

### YouTube Demonstration

[Watch Project Demonstration](INSERT_YOUTUBE_LINK_HERE)

---

## 36. Conclusion

The Low-Cost IoT-Based Two-Bed IV Drip
Monitoring System demonstrates the integration
of embedded sensing, wireless communication
and web-based visualization.

Two independent sensing channels were
implemented using load cells, HX711 modules
and optical sensors.

The Arduino Nano 33 IoT performs sensor
acquisition, measurement processing and
local warning generation.

The Raspberry Pi receives measurement
information through Bluetooth Low Energy
and provides a responsive monitoring
dashboard accessible through computers
and mobile devices.

The distributed architecture allows
local monitoring to continue during
communication interruptions, provided
the Arduino remains powered.

The project demonstrates practical
embedded systems development and the
integration of hardware, communication
protocols and software applications.

Further optical calibration, quantitative
accuracy evaluation and reliability
testing remain important areas for
future development.

---

## 37. References

1. Arun, P., Manuel, A., Fathima, D.,
   Philip, E. K., Shaju, M., & Suryadeep, P.
   (2023).

   Design and development of an automated
   electronic system for intravenous
   infusion control, monitoring and alerting.

   https://doi.org/10.21467/proceedings.160.25

2. Lyons, I., et al. (2018).

   Errors and discrepancies in the
   administration of intravenous infusions:
   A mixed methods multihospital
   observational study.

   BMJ Quality & Safety, 27(11), 892–901.

   https://doi.org/10.1136/bmjqs-2017-007476

### Technical Documentation

- Arduino Nano 33 IoT:
  https://docs.arduino.cc/hardware/nano-33-iot/

- ArduinoBLE:
  https://docs.arduino.cc/libraries/arduinoble/

- Raspberry Pi:
  https://www.raspberrypi.com/documentation/

- Flask:
  https://flask.palletsprojects.com/

- Bleak:
  https://bleak.readthedocs.io/

- Adafruit SSD1306:
  https://github.com/adafruit/Adafruit_SSD1306

---

## Academic Disclaimer

This project was developed exclusively
for academic and educational purposes.

The prototype uses water for demonstration.

It is not a certified medical device,
does not control medication delivery
and must not be used for patient monitoring,
treatment or clinical decision-making.

---

**SIT210 – Embedded Systems Development**

**Task 11.1P/C/D/HD – Final Project Artefact**

