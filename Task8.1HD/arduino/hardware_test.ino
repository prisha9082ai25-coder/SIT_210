
#include <ArduinoBLE.h>
#include <Wire.h>
#include <BH1750.h>

// ============================================================
// SIT210 TASK 8.1HD - AUDIO PROCESSING AND VOICE ACTIVATED
// LIGHTING SYSTEM
//
// Hardware:
// - Arduino Nano 33 IoT
// - BH1750 light intensity sensor
// - Bathroom LED
// - Hallway LED
// - Exhaust fan indicator LED
// - Raspberry Pi (communicates through Bluetooth)
//
// System functionality:
// 1. Automatically controls room lighting using light readings.
// 2. Receives voice commands from the Raspberry Pi through BLE.
// 3. Allows independent manual control of bathroom and hallway
//    lights.
// 4. Allows manual control of the exhaust fan indicator.
// 5. Supports switching between automatic and manual modes.
//
// ============================================================


// ============================================================
// 1. LED PIN CONFIGURATION
// ============================================================

// Each output is connected to a separate Arduino digital pin.

const int bathroomLED = 10;
const int hallwayLED = 11;
const int fanLED = 12;


// ============================================================
// 2. LIGHT SENSOR CONFIGURATION
// ============================================================

// Create a BH1750 sensor object to measure ambient light
// intensity in lux.

BH1750 lightSensor;

// Indicates whether the BH1750 sensor was successfully
// initialised during system startup.

bool sensorReady = false;


// ============================================================
// 3. AUTOMATIC LIGHTING SETTINGS
// ============================================================

// Determines whether automatic lighting control is enabled.
// When enabled, the Arduino controls the room lights using
// readings received from the light sensor.

bool automaticMode = true;

// Light thresholds used by the automatic lighting system.
//
// Below 20 lux: Turn the room lights ON.
// Above 40 lux: Turn the room lights OFF.
//
// The difference between the two thresholds prevents
// frequent switching when the light level fluctuates.

const float LIGHT_ON_LEVEL = 20.0;
const float LIGHT_OFF_LEVEL = 40.0;

// Tracks the current automatic lighting state.

bool lightsAreOn = false;


// ============================================================
// 4. SENSOR READING TIMING
// ============================================================

// Stores the time of the previous sensor reading.

unsigned long lastSensorRead = 0;

// The light sensor is checked every 2000 milliseconds
// (2 seconds) instead of continuously.

const unsigned long SENSOR_INTERVAL = 2000;


// ============================================================
// 5. BLUETOOTH LOW ENERGY CONFIGURATION
// ============================================================

// Define a BLE service to allow the Raspberry Pi to
// communicate with the Arduino.

BLEService controlService(
  "7A1B0001-6D5A-4C4E-9A10-123456789001"
);

// Characteristic used by the Raspberry Pi to send
// voice-recognised commands to the Arduino.

BLEStringCharacteristic commandCharacteristic(
  "7A1B0002-6D5A-4C4E-9A10-123456789001",
  BLEWrite,
  40
);

// Characteristic used by the Arduino to provide responses
// and status information to the Raspberry Pi.

BLEStringCharacteristic responseCharacteristic(
  "7A1B0003-6D5A-4C4E-9A10-123456789001",
  BLERead | BLENotify,
  40
);


// ============================================================
// 6. FUNCTION TO CONTROL BOTH ROOM LIGHTS
// ============================================================

// Controls both bathroom and hallway LEDs simultaneously.
//
// This function is used by the automatic lighting system
// when a shared light-level reading is below or above
// the configured thresholds.

void setLights(bool state) {

  digitalWrite(bathroomLED, state ? HIGH : LOW);
  digitalWrite(hallwayLED, state ? HIGH : LOW);

  // Update the stored lighting state.

  lightsAreOn = state;

  Serial.print("Both room lights: ");
  Serial.println(state ? "ON" : "OFF");
}


// ============================================================
// 7. AUTOMATIC LIGHT SENSOR CONTROL
// ============================================================

// Periodically reads the BH1750 sensor and controls the
// bathroom and hallway lights when automatic mode is enabled.
//
// The sensor reading is used to determine whether the
// environment is sufficiently dark or bright.

void updateAutomaticLighting() {

  // Check whether the sensor reading interval has elapsed.

  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {

    lastSensorRead = millis();

    // Continue only if the light sensor was initialised.

    if (sensorReady) {

      // Read the current ambient light intensity in lux.

      float lux = lightSensor.readLightLevel();

      Serial.print("Ambient light: ");
      Serial.print(lux);
      Serial.println(" lux");

      // Automatic lighting operates only when automatic
      // mode is enabled and the sensor returns a valid reading.

      if (automaticMode && lux >= 0) {

        // If the environment is darker than 20 lux,
        // switch ON both room lights.

        if (lux < LIGHT_ON_LEVEL && !lightsAreOn) {

          setLights(true);

          Serial.println(
            "Dark: bathroom and hallway lights ON."
          );
        }

        // If the environment becomes brighter than 40 lux,
        // switch OFF both room lights.

        else if (lux > LIGHT_OFF_LEVEL && lightsAreOn) {

          setLights(false);

          Serial.println(
            "Bright: bathroom and hallway lights OFF."
          );
        }
      }
    }
  }
}


// ============================================================
// 8. SETUP
// ============================================================

// Initialises the Arduino hardware, light sensor and
// Bluetooth communication when the system starts.

void setup() {

  Serial.begin(9600);

  // Configure the bathroom, hallway and fan output pins.

  pinMode(bathroomLED, OUTPUT);
  pinMode(hallwayLED, OUTPUT);
  pinMode(fanLED, OUTPUT);

  // Set all output indicators to their initial OFF state.

  setLights(false);
  digitalWrite(fanLED, LOW);

  // Initialise I2C communication for the BH1750 sensor.

  Wire.begin();

  Serial.println("Starting BH1750...");

  // Start the light sensor in continuous high-resolution mode.

  sensorReady = lightSensor.begin(
    BH1750::CONTINUOUS_HIGH_RES_MODE
  );

  // Check whether sensor initialisation was successful.

  if (sensorReady) {

    Serial.println("BH1750 ready.");

  } else {

    Serial.println("WARNING: BH1750 not detected.");
  }

  // Initialise Bluetooth Low Energy communication.

  Serial.println("Starting BLE...");

  if (!BLE.begin()) {

    Serial.println("ERROR: BLE failed.");

    // Stop execution if Bluetooth cannot initialise.

    while (true) {
      delay(1000);
    }
  }

  // Set the Bluetooth device name so that the Raspberry Pi
  // can identify the Arduino.

  BLE.setLocalName("SmartLighting");

  BLE.setAdvertisedService(controlService);

  // Register the command and response characteristics
  // with the Bluetooth service.

  controlService.addCharacteristic(
    commandCharacteristic
  );

  controlService.addCharacteristic(
    responseCharacteristic
  );

  BLE.addService(controlService);

  // Initialise Bluetooth communication values.

  commandCharacteristic.writeValue("");

  responseCharacteristic.writeValue("Ready");

  // Make the Arduino discoverable to the Raspberry Pi.

  BLE.advertise();

  Serial.println("SmartLighting ready.");
  Serial.println("Automatic mode is ON.");
}


// ============================================================
// 9. FUNCTION TO PROCESS BLUETOOTH COMMANDS
// ============================================================

// Receives text commands from the Raspberry Pi and performs
// the corresponding operation on the Arduino.
//
// The Raspberry Pi processes microphone input, converts
// speech into text and sends the recognised command through
// Bluetooth Low Energy.
//
// This function interprets the received command and controls
// the appropriate output.

void processCommand(String command) {

  // Remove unnecessary spaces and convert the command
  // into lowercase for consistent command comparison.

  command.trim();
  command.toLowerCase();

  Serial.print("Command received: ");
  Serial.println(command);


  // ----------------------------------------------------------
  // AUTOMATIC MODE ON
  // ----------------------------------------------------------

  // Enables automatic lighting based on the light sensor.

  if (command == "automatic on") {

    automaticMode = true;

    lightsAreOn =
      (digitalRead(bathroomLED) == HIGH &&
       digitalRead(hallwayLED) == HIGH);

    responseCharacteristic.writeValue(
      "Automatic mode on"
    );

    Serial.println("Automatic lighting enabled.");
  }


  // ----------------------------------------------------------
  // AUTOMATIC MODE OFF
  // ----------------------------------------------------------

  // Disables automatic lighting and allows manual control.

  else if (command == "automatic off") {

    automaticMode = false;

    responseCharacteristic.writeValue(
      "Automatic mode off"
    );

    Serial.println("Automatic lighting disabled.");
  }


  // ----------------------------------------------------------
  // BATHROOM LIGHT ON
  // ----------------------------------------------------------

  // Enables manual mode and switches ON only the bathroom LED.

  else if (command == "bathroom on") {

    automaticMode = false;

    digitalWrite(bathroomLED, HIGH);

    Serial.println("Bathroom LED pin 10 set HIGH.");

    responseCharacteristic.writeValue(
      "Bathroom on; manual mode"
    );
  }


  // ----------------------------------------------------------
  // BATHROOM LIGHT OFF
  // ----------------------------------------------------------

  // Switches OFF only the bathroom LED.

  else if (command == "bathroom off") {

    automaticMode = false;

    digitalWrite(bathroomLED, LOW);

    Serial.println("Bathroom LED pin 10 set LOW.");

    responseCharacteristic.writeValue(
      "Bathroom off; manual mode"
    );
  }


  // ----------------------------------------------------------
  // HALLWAY LIGHT ON
  // ----------------------------------------------------------

  // Switches ON only the hallway LED.

  else if (command == "hallway on") {

    automaticMode = false;

    digitalWrite(hallwayLED, HIGH);

    Serial.println("Hallway LED pin 11 set HIGH.");

    responseCharacteristic.writeValue(
      "Hallway on; manual mode"
    );
  }


  // ----------------------------------------------------------
  // HALLWAY LIGHT OFF
  // ----------------------------------------------------------

  // Switches OFF only the hallway LED.

  else if (command == "hallway off") {

    automaticMode = false;

    digitalWrite(hallwayLED, LOW);

    Serial.println("Hallway LED pin 11 set LOW.");

    responseCharacteristic.writeValue(
      "Hallway off; manual mode"
    );
  }


  // ----------------------------------------------------------
  // EXHAUST FAN ON
  // ----------------------------------------------------------

  // Switches ON the fan indicator LED connected to pin 12.
  // This output represents the exhaust fan activation.

  else if (command == "fan on") {

    digitalWrite(fanLED, HIGH);

    Serial.println("Fan indicator LED pin 12 set HIGH.");

    responseCharacteristic.writeValue(
      "Fan indicator on"
    );
  }


  // ----------------------------------------------------------
  // EXHAUST FAN OFF
  // ----------------------------------------------------------

  // Switches OFF the fan indicator LED.

  else if (command == "fan off") {

    digitalWrite(fanLED, LOW);

    Serial.println("Fan indicator LED pin 12 set LOW.");

    responseCharacteristic.writeValue(
      "Fan indicator off"
    );
  }


  // ----------------------------------------------------------
  // LIGHT SENSOR READING
  // ----------------------------------------------------------

  // Allows the Raspberry Pi to request the current
  // ambient light intensity from the BH1750 sensor.

  else if (command == "light") {

    if (sensorReady) {

      float lux = lightSensor.readLightLevel();

      if (lux >= 0) {

        // Convert the sensor reading into a text value
        // before sending it through Bluetooth.

        String result = String(lux, 1);

        responseCharacteristic.writeValue(result);

        Serial.print("Light level: ");
        Serial.print(result);
        Serial.println(" lux");

      } else {

        responseCharacteristic.writeValue(
          "Sensor error"
        );
      }

    } else {

      responseCharacteristic.writeValue(
        "Sensor unavailable"
      );
    }
  }


  // ----------------------------------------------------------
  // UNKNOWN COMMAND
  // ----------------------------------------------------------

  // Handle commands that do not match any recognised option.

  else {

    responseCharacteristic.writeValue(
      "Unknown command"
    );

    Serial.println("Unknown command.");
  }
}


// ============================================================
// 10. MAIN LOOP
// ============================================================

// Continuously manages Bluetooth communication, sensor
// monitoring and incoming commands from the Raspberry Pi.

void loop() {

  // Process pending Bluetooth Low Energy events.

  BLE.poll();

  // Check the ambient light level and update automatic
  // lighting when required.

  updateAutomaticLighting();

  // Check whether a central device (Raspberry Pi) is connected.

  BLEDevice central = BLE.central();

  if (central) {

    Serial.println("Raspberry Pi connected.");

    // Continue communication while the Raspberry Pi
    // remains connected to the Arduino.

    while (central.connected()) {

      // Process Bluetooth communication events.

      BLE.poll();

      // Continue automatic sensor monitoring even while
      // the Raspberry Pi is connected.

      updateAutomaticLighting();

      // Check whether a new command has been received
      // through the BLE command characteristic.

      if (commandCharacteristic.written()) {

        String receivedCommand =
          commandCharacteristic.value();

        // Pass the received command to the command
        // processing function.

        processCommand(receivedCommand);
      }
    }

    Serial.println("Raspberry Pi disconnected.");
  }
}
