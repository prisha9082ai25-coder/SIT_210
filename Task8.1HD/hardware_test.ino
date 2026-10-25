#include <ArduinoBLE.h>  // Provides Bluetooth Low Energy (BLE) communication.
#include <Wire.h>        // Provides I2C communication for connecting the light sensor.
#include <BH1750.h>      // Provides functions for reading light intensity from the BH1750 sensor.

// ============================================================
// 1. LED PIN CONFIGURATION
// ============================================================

// Define the Arduino digital pins connected to each LED.
// The LEDs represent the bathroom light, hallway light and fan status.
// Each LED should have an appropriate series resistor.
const int bathroomLED = 10;
const int hallwayLED = 11;
const int fanLED = 12;

// ============================================================
// 2. LIGHT SENSOR CONFIGURATION
// ============================================================

// Create an object to communicate with the BH1750 light sensor.
// The sensor measures ambient light intensity in lux.
BH1750 lightSensor;

// Tracks whether the sensor was detected successfully during setup.
// If initialisation fails, automatic lighting and sensor readings
// will not be performed as if the sensor were working.
bool sensorReady = false;

// ============================================================
// 3. AUTOMATIC LIGHTING SETTINGS
// ============================================================

// Controls whether automatic lighting is enabled.
// When true, the two room LEDs respond to the measured light level.
// When false, automatic light-level control is disabled.
bool automaticMode = true;

// Lux threshold below which the system switches both room lights ON.
// A low lux reading indicates that the surrounding area is dark.
const float LIGHT_ON_LEVEL = 20.0;

// Lux threshold above which the system switches both room lights OFF.
// This threshold is higher than the ON threshold to reduce rapid
// switching when the measured light level fluctuates near a threshold.
const float LIGHT_OFF_LEVEL = 40.0;

// Stores the last state set by the automatic lighting function.
// true means both bathroom and hallway LEDs were set ON.
// false means both were set OFF.
// Manual commands can control the two LEDs separately, so this
// variable represents the automatic system's shared light state.
bool lightsAreOn = false;

// ============================================================
// 4. SENSOR READING TIMING
// ============================================================

// Stores the time, in milliseconds, when the sensor was last read.
// millis() measures elapsed time since the Arduino started running.
unsigned long lastSensorRead = 0;

// The sensor reading interval is 2000 milliseconds, or 2 seconds.
// Using millis() avoids adding a delay that would block other tasks.
const unsigned long SENSOR_INTERVAL = 2000;

// ============================================================
// 5. BLUETOOTH LOW ENERGY (BLE) CONFIGURATION
// ============================================================

// Create the BLE service that groups the characteristics used
// for communication between the Raspberry Pi and Arduino.
//
// Keep this UUID unchanged because the Raspberry Pi program uses it
// to identify the Arduino's smart-lighting service.
BLEService controlService(
  "7A1B0001-6D5A-4C4E-9A10-123456789001"
);

// Create a BLE characteristic through which the Raspberry Pi
// writes commands to the Arduino, such as "bathroom on".
//
// BLEWrite allows the central device (Raspberry Pi) to write data.
// The characteristic can hold a string of up to 40 characters.
BLEStringCharacteristic commandCharacteristic(
  "7A1B0002-6D5A-4C4E-9A10-123456789001",
  BLEWrite,
  40
);

// Create a BLE characteristic for sending information back
// to the Raspberry Pi.
//
// BLERead allows the Raspberry Pi to read the latest response.
// BLENotify allows the Arduino to notify subscribed clients when
// the characteristic value changes.
BLEStringCharacteristic responseCharacteristic(
  "7A1B0003-6D5A-4C4E-9A10-123456789001",
  BLERead | BLENotify,
  40
);

// ============================================================
// 6. FUNCTION TO CONTROL BOTH ROOM LIGHTS
// ============================================================

// This function switches the bathroom and hallway LEDs together.
// It is used by automatic lighting when both lights should have
// the same state.
//
// The function also updates lightsAreOn so the automatic controller
// knows the state it last applied.
void setLights(bool state) {
  digitalWrite(bathroomLED, state ? HIGH : LOW);
  digitalWrite(hallwayLED, state ? HIGH : LOW);
  lightsAreOn = state;
}

// ============================================================
// 7. SETUP: RUNS ONCE WHEN THE ARDUINO STARTS
// ============================================================

void setup() {
  // Start serial communication so status messages and sensor
  // readings can be viewed in the Arduino IDE Serial Monitor.
  Serial.begin(9600);

  // Configure all three LED pins as outputs.
  pinMode(bathroomLED, OUTPUT);
  pinMode(hallwayLED, OUTPUT);
  pinMode(fanLED, OUTPUT);

  // Start with both room lights OFF.
  // Also initialise the fan-status LED to OFF.
  setLights(false);
  digitalWrite(fanLED, LOW);

  // Start I2C communication, which the BH1750 uses to exchange data.
  Wire.begin();
  Serial.println("Starting BH1750...");

  // Initialise the BH1750 in continuous high-resolution mode.
  // In this mode, the sensor repeatedly measures ambient light.
  // The return value indicates whether initialisation succeeded.
  sensorReady = lightSensor.begin(
    BH1750::CONTINUOUS_HIGH_RES_MODE
  );

  // Report the sensor's initialisation result.
  if (sensorReady) {
    Serial.println("BH1750 ready.");
  } else {
    // Automatic light control is skipped if initialisation failed.
    Serial.println("WARNING: BH1750 not detected.");
  }

  Serial.println("Starting BLE...");

  // Initialise Bluetooth Low Energy.
  // If BLE cannot start, report the problem and stop normal execution.
  if (!BLE.begin()) {
    Serial.println("ERROR: BLE failed.");

    // Remain here because BLE communication is essential
    // for receiving commands from the Raspberry Pi.
    while (true) {
      delay(1000);
    }
  }

  // Give the Arduino a recognisable BLE device name.
  // This name can be displayed when searching for BLE devices.
  BLE.setLocalName("SmartLighting");

  // Associate the control service with the device advertisement.
  // This helps a central device identify the service it needs.
  BLE.setAdvertisedService(controlService);

  // Add the command and response characteristics to the service.
  controlService.addCharacteristic(commandCharacteristic);
  controlService.addCharacteristic(responseCharacteristic);

  // Register the configured service with the BLE system.
  BLE.addService(controlService);

  // Set the initial values of the BLE characteristics.
  // An empty command value means no command has been received yet.
  // "Ready" tells a client reading the response that the system
  // has completed its initial setup.
  commandCharacteristic.writeValue("");
  responseCharacteristic.writeValue("Ready");

  // Start advertising so the Raspberry Pi can discover the Arduino.
  BLE.advertise();

  // Print startup information for debugging and demonstration.
  Serial.println("SmartLighting ready.");
  Serial.println("Automatic mode is ON.");
}

// ============================================================
// 8. FUNCTION TO PROCESS COMMANDS RECEIVED OVER BLE
// ============================================================

// This function receives a text command from the Raspberry Pi,
// checks which command was sent, and performs the corresponding action.
//
// Supported commands include:
// "automatic on" / "automatic off"
// "bathroom on" / "bathroom off"
// "hallway on" / "hallway off"
// "fan on" / "fan off"
// "light" to request a current light-level reading.
void processCommand(String command) {
  // Remove unnecessary spaces from the beginning and end.
  command.trim();

  // Convert the command to lowercase so that command matching
  // does not depend on the capitalisation of the received text.
  command.toLowerCase();

  // Display the received command in the Serial Monitor.
  Serial.print("Command received: ");
  Serial.println(command);

  // Enable automatic lighting.
  // The sensor will control both room lights when automaticMode
  // is true and the required light thresholds are crossed.
  if (command == "automatic on") {
    automaticMode = true;
    responseCharacteristic.writeValue("Automatic mode on");
    Serial.println("Automatic lighting enabled.");
  }

  // Disable automatic lighting without directly switching the
  // room LEDs ON or OFF. Their current states are retained.
  else if (command == "automatic off") {
    automaticMode = false;
    responseCharacteristic.writeValue("Automatic mode off");
    Serial.println("Automatic lighting disabled.");
  }

  // Turn the bathroom LED ON manually.
  // Manual room-light commands disable automatic mode so that
  // the sensor controller does not immediately override the request.
  else if (command == "bathroom on") {
    automaticMode = false;
    digitalWrite(bathroomLED, HIGH);
    responseCharacteristic.writeValue("Bathroom on; manual mode");
  }

  // Turn the bathroom LED OFF manually.
  else if (command == "bathroom off") {
    automaticMode = false;
    digitalWrite(bathroomLED, LOW);
    responseCharacteristic.writeValue("Bathroom off; manual mode");
  }

  // Turn the hallway LED ON manually.
  else if (command == "hallway on") {
    automaticMode = false;
    digitalWrite(hallwayLED, HIGH);
    responseCharacteristic.writeValue("Hallway on; manual mode");
  }

  // Turn the hallway LED OFF manually.
  else if (command == "hallway off") {
    automaticMode = false;
    digitalWrite(hallwayLED, LOW);
    responseCharacteristic.writeValue("Hallway off; manual mode");
  }

  // Turn the fan-status LED ON.
  // This is only an indicator LED; it does not control a real fan.
  else if (command == "fan on") {
    digitalWrite(fanLED, HIGH);
    responseCharacteristic.writeValue("Fan indicator on");
  }

  // Turn the fan-status LED OFF.
  else if (command == "fan off") {
    digitalWrite(fanLED, LOW);
    responseCharacteristic.writeValue("Fan indicator off");
  }

  // Return the current measured light level to the Raspberry Pi.
  else if (command == "light") {

    // Only attempt a sensor reading if initialisation succeeded.
    if (sensorReady) {
      float lux = lightSensor.readLightLevel();

      // A negative reading is treated as a sensor-reading error.
      if (lux >= 0) {
        // Convert the numerical reading to a string with one
        // decimal place so it can be transmitted through BLE.
        String result = String(lux, 1);

        // Send the light-level value to the Raspberry Pi.
        responseCharacteristic.writeValue(result);

        // Also display the reading locally for debugging.
        Serial.print("Light level: ");
        Serial.print(result);
        Serial.println(" lux");
      } else {
        // Report a failed reading instead of returning a value
        // that could be mistaken for a valid light measurement.
        responseCharacteristic.writeValue("Sensor error");
      }
    } else {
      // Tell the Raspberry Pi that the sensor was not detected
      // during setup and cannot provide a reading.
      responseCharacteristic.writeValue("Sensor unavailable");
    }
  }

  // Handle unsupported commands safely.
  // No LED state is deliberately changed in this branch.
  else {
    responseCharacteristic.writeValue("Unknown command");
    Serial.println("Unknown command.");
  }
}

// ============================================================
// 9. MAIN LOOP: REPEATS WHILE THE ARDUINO IS RUNNING
// ============================================================

void loop() {
  // Process BLE communication events, such as connection activity
  // and incoming or outgoing characteristic data.
  BLE.poll();

  // ----------------------------------------------------------
  // 9A. READ THE AMBIENT LIGHT SENSOR
  // ----------------------------------------------------------

  // Check whether two seconds have elapsed since the previous
  // scheduled sensor reading. This avoids using delay(2000),
  // which would pause other loop operations for two seconds.
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();

    // Read the sensor only if it was successfully initialised.
    if (sensorReady) {
      float lux = lightSensor.readLightLevel();

      // Print the current ambient light level in lux.
      Serial.print("Ambient light: ");
      Serial.print(lux);
      Serial.println(" lux");

      // Automatic control is performed only when automatic mode
      // is enabled and the sensor reading is not negative.
      if (automaticMode && lux >= 0) {

        // If the area is darker than 20 lux and the lights are
        // currently recorded as OFF, turn both lights ON.
        if (lux < LIGHT_ON_LEVEL && !lightsAreOn) {
          setLights(true);
          Serial.println("Dark: bathroom and hallway lights ON.");
        }

        // If the area is brighter than 40 lux and the lights are
        // currently recorded as ON, turn both lights OFF.
        //
        // The gap between 20 and 40 lux is hysteresis. It helps
        // prevent rapid ON/OFF switching near a single threshold.
        else if (lux > LIGHT_OFF_LEVEL && lightsAreOn) {
          setLights(false);
          Serial.println("Bright: bathroom and hallway lights OFF.");
        }
      }
    }
  }

  // ----------------------------------------------------------
  // 9B. CHECK FOR A RASPBERRY PI BLE CONNECTION
  // ----------------------------------------------------------

  // Check whether a central BLE device, such as the Raspberry Pi,
  // has connected to this Arduino.
  BLEDevice central = BLE.central();

  if (central) {
    Serial.println("Raspberry Pi connected.");

    // Continue handling communication while the central device
    // remains connected. BLE.poll() keeps BLE events processing.
    while (central.connected()) {
      BLE.poll();

      // Check whether the Raspberry Pi has written a new command
      // to the command characteristic.
      if (commandCharacteristic.written()) {

        // Retrieve the received text and pass it to the command
        // processing function for validation and execution.
        processCommand(commandCharacteristic.value());
      }
    }

    // This message is printed when the central device disconnects.
    Serial.println("Raspberry Pi disconnected.");
  }
}
