// HX711 library reads the amplified digital output of each load cell.
#include "HX711.h"
// Math functions provide absolute differences, finite-value checks and rounding.
#include <math.h>
// ArduinoBLE provides the BLE peripheral, service and notification characteristic.
#include <ArduinoBLE.h>
// String utilities compare status labels using strcmp.
#include <string.h>
// Wire handles I2C communication with the OLED.
#include <Wire.h>
// Adafruit GFX provides text and drawing primitives.
#include <Adafruit_GFX.h>
// SSD1306 driver manages the 128 x 64 display buffer.
#include <Adafruit_SSD1306.h>

// ============================================================
// TWO-BED IV MONITORING SYSTEM
// Arduino Nano 33 IoT
// Water-only academic demonstration prototype
// ============================================================


// ============================================================
// 1. HARDWARE CONFIGURATION
// ============================================================

// Array index 0 represents Bed 1; index 1 represents Bed 2. DT carries HX711 data.
const int DT_PINS[2]  = {2, 4};
// Each HX711 has its own clock output from the Arduino.
const int SCK_PINS[2] = {3, 5};

// Digital IR inputs are active LOW when an obstruction is detected.
const int IR_PINS[2]  = {6, 7};
// Each bed has an independent warning LED.
const int LED_PINS[2] = {8, 9};

// Separate HX711 objects keep the two sensor channels independent.
HX711 scales[2];

// Local display and audible bench-demo indicator.
// The shared buzzer uses D10; the polarity setting controls its on/off logic.
const int BUZZER_PIN = 10;
const bool BUZZER_ACTIVE_HIGH = true;
// The tested I2C address is 0x3C; -1 means no dedicated OLED reset pin.
const uint8_t OLED_ADDRESS = 0x3C;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool oledReady = false;
unsigned long lastOLED = 0;
// Refresh the OLED once per second rather than on every loop iteration.
const unsigned long OLED_INTERVAL = 1000;


// false = Human-readable Serial Monitor
// true  = JSON output for Raspberry Pi

const bool JSON_MODE = false;


// ============================================================
// 2. ACTUAL CALIBRATION PARAMETERS
// ============================================================

// Raw ADC baselines with no bottle attached; subtract these to remove the zero offset.
const float ZERO_OFFSET[2] = {
  243471.75,
  -154616.50
};

// Raw readings with an empty bottle attached; used to estimate bottle mass.
const float EMPTY_OFFSET[2] = {
  254162.50,
  -137512.25
};

// Calibration factors in ADC counts per gram, independently measured for each bed.
const float CAL_FACTOR[2] = {
  397.767,
  355.910
};

// Nominal water capacity in mL for the percentage calculation.
const float CAPACITY = 500.0;


// ============================================================
// 3. DEMONSTRATION SETTINGS
// ============================================================

// Demo warning thresholds in mL, not clinical alarm settings.
const float LOW_VOLUME = 100.0;
const float EMPTY_VOLUME = 5.0;

// EMA coefficient: 20% new reading and 80% previous filtered reading.
const float ALPHA = 0.20;

// A change exceeding 30 g is treated as a possible disturbance.
const float DISTURBANCE_LIMIT = 30.0;

// Reject closely spaced triggers; 250 ms limits accepted events to about 240/min.
const unsigned long MIN_DROP_INTERVAL = 250;
// Require 30 ms of clear HIGH signal before arming the next detection.
const unsigned long CLEAR_TIME = 30;

// Report both beds every 2 seconds; calculate flow over at least 15 seconds.
const unsigned long REPORT_INTERVAL = 2000;
const unsigned long FLOW_INTERVAL = 15000;
// No accepted drops for over 30 seconds triggers a demo warning after startup grace.
const unsigned long STOP_TIMEOUT = 30000;
// Mark the HX711 unavailable if it supplies no ready sample for over 3 seconds.
const unsigned long SENSOR_TIMEOUT = 3000;

// Enter experimentally measured drops per mL later.
// Zero means volumetric drop flow is unavailable.

// Uncalibrated: retain unavailable output until independently measured.
const float DROP_FACTOR[2] = {0.0, 0.0};


// ============================================================
// 4. BED DATA STRUCTURE
// ============================================================

// Group all measurements, timestamps and state flags for one bed.
struct BedData {

  // Most recent unconverted signed ADC sample.
  long rawReading = 0;

  // Mass values are in grams; total includes the bottle and water.
  float totalWeight = 0;
  float bottleWeight = 0;
  float waterWeight = 0;

  // Estimated water volume in mL and percentage of nominal capacity.
  float volume = 0;
  float percentage = 0;

  // Flow values are in mL/h; optical frequency is in drops/min.
  float weightFlow = 0;
  float dropFlow = 0;
  float dropsPerMinute = 0;

  // Cumulative accepted events and timestamp of the last accepted event.
  unsigned long dropCount = 0;
  unsigned long lastDropTime = 0;

  // Timestamp marking the start of a clear optical path.
  unsigned long clearStart = 0;

  // Arming prevents counting the same continuous LOW signal repeatedly.
  bool armed = false;
  bool clearTracking = false;

  // Flags describe load-cell validity, bottle presence and filter state.
  bool sensorOK = false;
  bool bottlePresent = false;
  bool initialized = false;
  bool disturbance = false;

  // Distinguish a completed flow estimate from a window still collecting data.
  bool flowValid = false;
  bool flowWindowStarted = false;

  // Candidate mass and consecutive samples used to accept sustained changes.
  float candidateWeight = 0;
  int stableSamples = 0;

  // Store the starting volume, time and count for the next flow calculation.
  float flowStartVolume = 0;

  unsigned long flowStartTime = 0;
  unsigned long flowStartDrops = 0;

  // Track attachment time and most recent HX711 sample.
  unsigned long bottleStartTime = 0;
  unsigned long lastSample = 0;

  const char* status = "INITIALIZING";
};

// Allocate separate state for each bed.
BedData beds[2];

unsigned long lastReport = 0;


// ============================================================
// 5. RESET FLOW MEASUREMENTS
// ============================================================

// Invalidate previous flow estimates after a fault, removal or disturbance.
void resetFlow(int bed) {

  // A reference modifies the original bed record rather than copying it.
  BedData &b = beds[bed];

  b.flowValid = false;
  b.flowWindowStarted = false;

  b.weightFlow = 0;
  b.dropFlow = 0;
  b.dropsPerMinute = 0;
}


// ============================================================
// 6. ARDUINO SETUP
// ============================================================

// Bluetooth configuration (compatible with iv_receiver.py)
// Service UUID identifies this application to a BLE central.
BLEService ivService("7a2b0001-6d5a-4c4e-9a10-123456789001");
// Characteristic UUID identifies the 20-byte measurement channel; notifications push updates.
BLECharacteristic dataCharacteristic(
  "7a2b0002-6d5a-4c4e-9a10-123456789001",
  BLERead | BLENotify, 20);
bool bluetoothReady = false;
bool wasConnected = false;

// Initialize BLE without preventing local operation if initialization fails.
void setupBluetooth() {
  if (!BLE.begin()) {
    if (!JSON_MODE) Serial.println("BLE initialization failed");
    return;
  }
  // Advertise the name used by the Raspberry Pi scanner.
  BLE.setLocalName("IV_Monitoring");
  BLE.setAdvertisedService(ivService);
  ivService.addCharacteristic(dataCharacteristic);
  BLE.addService(ivService);
  BLE.advertise();
  bluetoothReady = true;
  if (!JSON_MODE) Serial.println("Bluetooth ready: IV_Monitoring");
}

// Service BLE events and resume advertising after a connection is lost.
void serviceBluetooth() {
  if (!bluetoothReady) return;
  BLE.poll();
  bool connected = BLE.connected();
  if (wasConnected && !connected) BLE.advertise();
  wasConnected = connected;
}

// Initialize local outputs independently of Bluetooth.
void setupLocalOutputs() {
  // Set the inactive buzzer level before configuring its output pin.
  digitalWrite(BUZZER_PIN, BUZZER_ACTIVE_HIGH ? LOW : HIGH);
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin();
  delay(1000); // Match the successful standalone OLED test.
  // Probe the OLED address and check for an I2C acknowledgement.
  Wire.beginTransmission(OLED_ADDRESS);
  uint8_t oledError = Wire.endTransmission();
  if (!JSON_MODE) {
    Serial.print("OLED I2C address: 0x");
    Serial.println(OLED_ADDRESS, HEX);
    Serial.print("OLED I2C result (0 = ACK): ");
    Serial.println(oledError);
  }
  // Initialize the display only if the I2C probe succeeds.
  oledReady = (oledError == 0) && oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (oledReady) {
    if (!JSON_MODE) Serial.println("SSD1306 initialized successfully");
    oled.invertDisplay(false);
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("WATER DEMO");
    oled.println("Initializing...");
    oled.display();
    delay(1000); // Make startup screen visible before BLE starts.
  } else if (!JSON_MODE) {
    Serial.println("OLED initialization failed; continuing without display");
  }
}

// Run once at power-up or reset.
void setup() {

  Serial.begin(115200);

  // Configure both beds using their respective pin arrays.
  for (int i = 0; i < 2; i++) {

    scales[i].begin(
      DT_PINS[i],
      SCK_PINS[i]
    );

    pinMode(IR_PINS[i], INPUT);

    pinMode(LED_PINS[i], OUTPUT);

    digitalWrite(LED_PINS[i], LOW);

    // Initialize the timeout timestamp before normal sampling starts.
    beds[i].lastSample = millis();

    // Empty bottle mass = (empty raw reading - unloaded raw reading) / counts per gram.
    beds[i].bottleWeight =
      (EMPTY_OFFSET[i] - ZERO_OFFSET[i])
      / CAL_FACTOR[i];
  }

  if (!JSON_MODE) {

    Serial.println(
      "TWO-BED IV MONITORING SYSTEM"
    );

    Serial.println(
      "System Initializing..."
    );

    Serial.println(
      "=============================="
    );
  }
  setupLocalOutputs();
  setupBluetooth();
}


// ============================================================
// 7. FILTERED IR DROP DETECTION
// ============================================================

// Poll one optical input and count only accepted clear-to-blocked events.
void detectDrop(int bed) {

  BedData &b = beds[bed];

  // millis() gives elapsed milliseconds since boot; subtraction supports timer rollover.
  unsigned long now = millis();

  int state = digitalRead(IR_PINS[bed]);

  // HIGH = clear detection area

  if (state == HIGH) {

    // Start timing a newly clear optical path.
    if (!b.clearTracking) {

      b.clearStart = now;
      b.clearTracking = true;
    }

    // Arm only after the path remains clear long enough.
    if (now - b.clearStart >= CLEAR_TIME) {

      b.armed = true;
    }
  }

  // LOW = drop detected

  else {

    b.clearTracking = false;

    // A LOW transition is accepted only if the detector was armed.
    if (b.armed) {

      b.armed = false;

      // Accept the first event immediately; later events must meet the minimum interval.
      if (b.dropCount == 0 ||
          now - b.lastDropTime
          >= MIN_DROP_INTERVAL) {

        b.dropCount++;

        b.lastDropTime = now;

        // Display every accepted drop

        if (!JSON_MODE) {

          Serial.print("Bed ");

          Serial.print(bed + 1);

          Serial.print(" Drop Count: ");

          Serial.println(b.dropCount);
        }
      }
    }
  }
}


// ============================================================
// 8. LOAD CELL MEASUREMENT
// ============================================================

// Acquire, validate, filter and convert one load-cell sample.
void readLoadCell(int bed) {

  BedData &b = beds[bed];

  unsigned long now = millis();

  // Non-blocking sensor readiness check

  // Return promptly if no ADC sample is ready; do not wait for conversion.
  if (!scales[bed].is_ready()) {

    if (now - b.lastSample > SENSOR_TIMEOUT) {

      b.sensorOK = false;
      b.initialized = false;

      resetFlow(bed);
    }

    return;
  }

  long raw = scales[bed].read();

  b.rawReading = raw;
  b.lastSample = now;

  // Detect ADC saturation

  // Signed 24-bit ADC endpoints indicate possible saturation.
  if (raw == 8388607 ||
      raw == -8388608) {

    b.sensorOK = false;
    b.initialized = false;

    resetFlow(bed);

    return;
  }

  b.sensorOK = true;

  // Convert ADC reading to grams

  // Subtract the unloaded baseline and divide by counts per gram.
  float measuredWeight =
    (raw - ZERO_OFFSET[bed])
    / CAL_FACTOR[bed];

  // Detect bottle removal

  // A reading below half the empty bottle mass is treated as removal.
  if (measuredWeight <
      b.bottleWeight * 0.5) {

    b.bottlePresent = false;
    b.initialized = false;

    b.totalWeight = measuredWeight;

    b.waterWeight = 0;
    b.volume = 0;
    b.percentage = 0;

    b.disturbance = false;
    b.stableSamples = 0;

    resetFlow(bed);

    return;
  }

  // Detect newly attached bottle

  // Record the attachment time for the no-drop grace period.
  if (!b.bottlePresent) {

    b.bottleStartTime = now;
  }

  b.bottlePresent = true;

  // Initialize first valid measurement

  // Seed the filter with the first valid measurement rather than zero.
  if (!b.initialized) {

    b.totalWeight = measuredWeight;

    b.initialized = true;
    b.disturbance = false;

    b.stableSamples = 0;

    resetFlow(bed);
  }

  // Detect sudden weight disturbance

  // Large changes bypass normal smoothing until their stability is checked.
  else if (
    fabs(measuredWeight - b.totalWeight)
    > DISTURBANCE_LIMIT
  ) {

    b.disturbance = true;

    resetFlow(bed);

    // Restart the candidate sequence if the new reading differs by more than 5 g.
    if (b.stableSamples == 0 ||
        fabs(measuredWeight -
             b.candidateWeight) > 5.0) {

      b.candidateWeight = measuredWeight;

      b.stableSamples = 1;
    }

    else {

      b.stableSamples++;
    }

    // Accept sustained weight change

    // Ten consistent samples permit a sustained mass change to become the new baseline.
    if (b.stableSamples >= 10) {

      b.totalWeight = measuredWeight;

      b.disturbance = false;
      b.stableSamples = 0;

      b.bottleStartTime = now;
    }

    else {

      return;
    }
  }

  // Exponential weight smoothing

  else {

    b.disturbance = false;
    b.stableSamples = 0;

    // Exponential moving average reduces short-term noise without storing a sample array.
    b.totalWeight =
      ALPHA * measuredWeight +
      (1.0 - ALPHA) * b.totalWeight;
  }

  // Subtract empty bottle mass

  // Remove container mass so only water contributes to the volume estimate.
  b.waterWeight =
    b.totalWeight - b.bottleWeight;

  // Prevent negative water mass due to measurement noise.
  if (b.waterWeight < 0) {

    b.waterWeight = 0;
  }

  // Approximate water density: 1 g/mL

  // For this water-only demonstration, approximate 1 gram as 1 mL.
  b.volume = b.waterWeight;

  b.percentage =
    (b.volume / CAPACITY) * 100.0;

  // Clamp the displayed percentage; the volume itself is not capped here.
  b.percentage =
    constrain(b.percentage, 0.0, 100.0);
}


// ============================================================
// 9. FLOW RATE CALCULATION
// ============================================================

// Estimate average flow over a timed window, not instantaneous flow.
void calculateFlow(int bed) {

  BedData &b = beds[bed];

  // Do not calculate flow using invalid or disturbed measurements.
  if (!b.sensorOK ||
      !b.bottlePresent ||
      !b.initialized ||
      b.disturbance) {

    resetFlow(bed);

    return;
  }

  unsigned long now = millis();

  // Initialize flow measurement window

  // Capture starting values and wait for enough elapsed time.
  if (!b.flowWindowStarted) {

    b.flowStartTime = now;

    b.flowStartVolume = b.volume;

    b.flowStartDrops = b.dropCount;

    b.flowWindowStarted = true;

    return;
  }

  unsigned long elapsed =
    now - b.flowStartTime;

  if (elapsed < FLOW_INTERVAL) {

    return;
  }

  // Convert milliseconds to hours for mL/h and minutes for drops/min.
  float elapsedHours =
    elapsed / 3600000.0;

  float elapsedMinutes =
    elapsed / 60000.0;

  // Weight-based flow rate

  // Volume lost is the starting volume minus the current filtered volume.
  float volumeLost =
    b.flowStartVolume - b.volume;

  // Ignore apparent volume gains when calculating outward flow.
  if (volumeLost < 0) {

    volumeLost = 0;
  }

  // Divide volume lost by elapsed hours to estimate average weight-based flow.
  b.weightFlow =
    volumeLost / elapsedHours;

  // Number of drops during measurement window

  // Subtract the starting cumulative count to isolate events in this window.
  unsigned long newDrops =
    b.dropCount - b.flowStartDrops;

  b.dropsPerMinute =
    newDrops / elapsedMinutes;

  // Drop-based volumetric flow

  // Optical volumetric flow requires a measured drops-per-mL factor; zero disables it.
  if (DROP_FACTOR[bed] > 0) {

    float dropVolume =
      newDrops / DROP_FACTOR[bed];

    b.dropFlow =
      dropVolume / elapsedHours;
  }

  // A valid window is complete even when optical volumetric calibration is missing.
  b.flowValid = true;

  // Start next measurement window

  b.flowStartTime = now;

  b.flowStartVolume = b.volume;

  b.flowStartDrops = b.dropCount;
}


// ============================================================
// 10. STATUS AND ALERT MANAGEMENT
// ============================================================

// Select one status in priority order; the first matching condition wins.
void updateStatus(int bed) {

  BedData &b = beds[bed];

  unsigned long now = millis();

  // Load-cell failure has priority over all other statuses.
  if (!b.sensorOK) {

    b.status = "SENSOR_FAULT";
  }

  else if (!b.bottlePresent) {

    b.status = "BOTTLE_REMOVED";
  }

  else if (b.disturbance) {

    b.status = "DISTURBANCE";
  }

  // Empty and low-volume thresholds apply only after sensor and bottle checks.
  else if (b.volume <= EMPTY_VOLUME) {

    b.status = "EMPTY";
  }

  else if (b.volume <= LOW_VOLUME) {

    b.status = "LOW_VOLUME";
  }

  // Both attachment age and time since the last accepted drop must exceed the timeout.
  else if (
    now - b.bottleStartTime > STOP_TIMEOUT &&
    now - b.lastDropTime > STOP_TIMEOUT
  ) {

    b.status = "NO_DROPS";
  }

  else {

    b.status = "NORMAL";
  }

  // Independent LED alarm for each bed

  // Any status other than NORMAL turns on this bed warning LED.
  bool alert =
    strcmp(b.status, "NORMAL") != 0;

  digitalWrite(
    LED_PINS[bed],
    alert ? HIGH : LOW
  );
}


// ============================================================
// 11. MEASUREMENT VALIDITY
// ============================================================

// Central validity check reused by serial, BLE and OLED output.
bool measurementValid(int bed) {

  BedData &b = beds[bed];

  return b.sensorOK &&
         b.bottlePresent &&
         b.initialized &&
         !b.disturbance;
}


// ============================================================
// 12. HUMAN-READABLE SERIAL OUTPUT
// ============================================================

// Print diagnostic measurements and explicit unavailable states for one bed.
void printBed(int bed) {

  BedData &b = beds[bed];

  Serial.println();

  Serial.print("========== BED ");
  Serial.print(bed + 1);
  Serial.println(" ==========");

  Serial.print("Raw Reading      : ");
  Serial.println(b.rawReading);

  if (measurementValid(bed)) {

    Serial.print("Total Weight     : ");
    Serial.print(b.totalWeight, 2);
    Serial.println(" g");

    Serial.print("Bottle Weight    : ");
    Serial.print(b.bottleWeight, 2);
    Serial.println(" g");

    Serial.print("Water Weight     : ");
    Serial.print(b.waterWeight, 2);
    Serial.println(" g");

    Serial.print("Remaining Volume : ");
    Serial.print(b.volume, 2);
    Serial.println(" mL");

    Serial.print("Remaining Level  : ");
    Serial.print(b.percentage, 2);
    Serial.println(" %");
  }

  else {

    Serial.println(
      "Weight/Volume    : UNAVAILABLE"
    );
  }

  Serial.print("Total Drop Count : ");
  Serial.println(b.dropCount);

  // Display flow only after a valid measurement window.
  if (b.flowValid &&
      measurementValid(bed)) {

    Serial.print("Drops Per Minute : ");
    Serial.println(b.dropsPerMinute, 2);

    Serial.print("Weight Flow Rate : ");
    Serial.print(b.weightFlow, 2);
    Serial.println(" mL/h");

    if (DROP_FACTOR[bed] > 0) {

      Serial.print("Drop Flow Rate   : ");
      Serial.print(b.dropFlow, 2);
      Serial.println(" mL/h");

      // Absolute difference compares the two estimates when optical calibration exists.
      Serial.print("Flow Difference  : ");

      Serial.print(
        fabs(b.weightFlow - b.dropFlow),
        2
      );

      Serial.println(" mL/h");
    }

    else {

      Serial.println(
        "Drop Flow Rate   : NOT CALIBRATED"
      );
    }
  }

  else {

    Serial.println(
      "Flow Rate        : WAITING FOR VALID WINDOW"
    );
  }

  // Report window age and validity to help diagnose missing flow values.
  Serial.print("Flow Window (ms) : ");
  Serial.println(b.flowWindowStarted ? millis() - b.flowStartTime : 0UL);
  Serial.print("Flow Valid       : ");
  Serial.println(b.flowValid ? "YES" : "NO");
  // The BLE fixed-point format cannot represent values above 6553.4.
  if (b.flowValid && b.weightFlow > 6553.4f) {
    Serial.println("BLE flow value out of range");
  }

  Serial.print("Sensor Status    : ");

  Serial.println(
    b.sensorOK ? "WORKING" : "FAULT"
  );

  Serial.print("Bottle Status    : ");

  Serial.println(
    b.bottlePresent ? "ATTACHED" : "REMOVED"
  );

  Serial.print("System Status    : ");
  Serial.println(b.status);

  Serial.println("==============================");
}


// ============================================================
// 13. JSON OUTPUT HELPERS
// ============================================================

// Use JSON null rather than misleading numeric values for invalid measurements.
void printValue(float value, bool valid) {

  if (valid && isfinite(value)) {

    Serial.print(value, 2);
  }

  else {

    Serial.print("null");
  }
}

// Emit lowercase JSON boolean literals.
void printBoolean(bool value) {

  Serial.print(value ? "true" : "false");
}


// ============================================================
// 14. JSON DATA FOR RASPBERRY PI
// ============================================================

// Serialize one bed as a JSON object; this is optional serial output, not the BLE packet.
void sendBedJSON(int bed) {

  BedData &b = beds[bed];

  bool valid = measurementValid(bed);

  bool flowOK = valid && b.flowValid;

  // Require both a valid window and a positive optical calibration factor.
  bool dropFlowOK =
    flowOK && DROP_FACTOR[bed] > 0;

  Serial.print("{\"bed\":");
  Serial.print(bed + 1);

  Serial.print(",\"raw\":");
  Serial.print(b.rawReading);

  Serial.print(",\"total_weight_g\":");
  printValue(b.totalWeight, valid);

  Serial.print(",\"water_weight_g\":");
  printValue(b.waterWeight, valid);

  Serial.print(",\"volume_ml\":");
  printValue(b.volume, valid);

  Serial.print(",\"percentage\":");
  printValue(b.percentage, valid);

  Serial.print(",\"drop_count\":");
  Serial.print(b.dropCount);

  Serial.print(",\"drops_per_minute\":");
  printValue(b.dropsPerMinute, flowOK);

  Serial.print(",\"weight_flow_mlh\":");
  printValue(b.weightFlow, flowOK);

  Serial.print(",\"drop_flow_mlh\":");
  printValue(b.dropFlow, dropFlowOK);

  Serial.print(",\"flow_difference_mlh\":");

  printValue(
    fabs(b.weightFlow - b.dropFlow),
    dropFlowOK
  );

  Serial.print(",\"sensor_ok\":");
  printBoolean(b.sensorOK);

  Serial.print(",\"bottle_present\":");
  printBoolean(b.bottlePresent);

  // This flag indicates the IR feature is enabled, not that the IR sensor has passed a fault test.
  Serial.print(",\"ir_enabled\":true");

  Serial.print(",\"status\":\"");
  Serial.print(b.status);
  Serial.print("\"}");
}


// ============================================================
// 15. COMPLETE SYSTEM JSON OUTPUT
// ============================================================

// Wrap both bed objects in a single JSON record with Arduino uptime.
void sendSystemJSON() {

  Serial.print("{\"uptime_ms\":");

  Serial.print(millis());

  Serial.print(",\"beds\":[");

  sendBedJSON(0);

  Serial.print(",");

  sendBedJSON(1);

  Serial.println("]}");
}


// ============================================================
// 16. MAIN LOOP
// ============================================================

// BLE packet encoding: little-endian, 20 bytes per bed.
// Map text status to a compact byte understood by the Python receiver.
uint8_t getStatusCode(int bed) {
  const char* status = beds[bed].status;
  const char* names[] = {"NORMAL", "LOW_VOLUME", "EMPTY", "NO_DROPS",
                         "BOTTLE_REMOVED", "SENSOR_FAULT", "DISTURBANCE"};
  for (uint8_t i = 0; i < 7; i++) {
    if (strcmp(status, names[i]) == 0) return i;
  }
  // Code 7 represents initialization or an unmatched status.
  return 7;
}

// Write a 16-bit integer least-significant byte first (little endian).
void put16(uint8_t* packet, int position, uint16_t value) {
  packet[position] = value & 0xFF;
  packet[position + 1] = (value >> 8) & 0xFF;
}

// Write a 32-bit integer as four little-endian bytes.
void put32(uint8_t* packet, int position, uint32_t value) {
  for (int i = 0; i < 4; i++) {
    packet[position + i] = (value >> (8 * i)) & 0xFF;
  }
}

// Encode one decimal place using an unsigned 16-bit integer; 65535 means unavailable.
uint16_t encodeValue(float value, bool valid) {
  if (!valid || !isfinite(value) || value < 0 || value > 6553.4)
    return 65535;
  return (uint16_t)round(value * 10);
}

// Send one bed only when BLE is ready, connected and notifications are subscribed.
void sendBedBluetooth(int bed) {
  if (!bluetoothReady || !BLE.connected() ||
      !dataCharacteristic.subscribed()) return;
  BedData &b = beds[bed];
  bool valid = measurementValid(bed);
  bool flowOK = valid && b.flowValid;
  bool dropFlowOK = flowOK && DROP_FACTOR[bed] > 0;
  // Packet layout matches Python struct format <BBBBHHIHHHH (20 bytes).
  uint8_t packet[20] = {0};
  // Bytes 0-3: bed number, status code, load-cell flag and bottle flag.
  packet[0] = bed + 1;
  packet[1] = getStatusCode(bed);
  packet[2] = b.sensorOK ? 1 : 0;
  packet[3] = b.bottlePresent ? 1 : 0;
  // Bytes 4-7: volume and percentage, scaled by ten.
  put16(packet, 4, encodeValue(b.volume, valid));
  put16(packet, 6, encodeValue(b.percentage, valid));
  // Bytes 8-11: cumulative drop count as a 32-bit unsigned integer.
  put32(packet, 8, b.dropCount);
  // Bytes 12-19: drops/min, weight flow, optical flow and total mass, each scaled by ten.
  put16(packet, 12, encodeValue(b.dropsPerMinute, flowOK));
  put16(packet, 14, encodeValue(b.weightFlow, flowOK));
  put16(packet, 16, encodeValue(b.dropFlow, dropFlowOK));
  put16(packet, 18, encodeValue(b.totalWeight, valid));
  // Update the characteristic so the subscribed Raspberry Pi can receive a notification.
  dataCharacteristic.writeValue(packet, sizeof(packet));
}

// ============================================================
// LOCAL OLED AND BUZZER (water-only bench demonstration)
// ============================================================
// Draw one bed at the supplied vertical position in the display buffer.
void drawBedOLED(int bed, int y) {
  BedData &b = beds[bed];
  oled.setCursor(0, y);
  oled.print("B");
  oled.print(bed + 1);
  oled.print(": ");
  // Avoid presenting invalid volume or percentage as a real measurement.
  if (measurementValid(bed)) {
    oled.print(b.volume, 0);
    oled.print("mL ");
    oled.print(b.percentage, 0);
    oled.print("%");
  } else {
    oled.print("Unavailable");
  }
  oled.setCursor(0, y + 9);
  oled.print(b.status);
  // Draw a 128-pixel outline and fill its interior according to remaining percentage.
  oled.drawRect(0, y + 18, 128, 5, SSD1306_WHITE);
  if (measurementValid(bed)) {
    int width = (int)(constrain(b.percentage, 0.0f, 100.0f) * 1.26f);
    if (width > 0) oled.fillRect(1, y + 19, width, 3, SSD1306_WHITE);
  }
}

// Update the OLED at a timed interval; skip if initialization failed.
void updateOLED() {
  if (!oledReady) return;
  unsigned long now = millis();
  if (now - lastOLED < OLED_INTERVAL) return;
  lastOLED = now;
  // Clear the memory buffer before drawing the next complete frame.
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextWrap(false);
  oled.setCursor(0, 0);
  oled.print("WATER DEMO");
  oled.setCursor(90, 0);
  // Show whether BLE is connected or local-only operation is active.
  oled.print(bluetoothReady && BLE.connected() ? "BLE" : "LOCAL");
  oled.drawFastHLine(0, 9, 128, SSD1306_WHITE);
  drawBedOLED(0, 12);
  drawBedOLED(1, 39);
  // Transfer the completed display buffer over I2C.
  oled.display();
}

// Drive one shared buzzer if either bed has an active demo warning.
void updateBuzzer() {
  // Shared audible indicator; independent LEDs and OLED identify each bed.
  // Suppress startup chirps for the first three seconds.
  bool alert = false;
  if (millis() >= 3000UL) {
    for (int i = 0; i < 2; i++) {
      if (strcmp(beds[i].status, "NORMAL") != 0 &&
          strcmp(beds[i].status, "INITIALIZING") != 0) alert = true;
    }
  }
  // Use a 150 ms pulse every 2 seconds without delay() in the loop.
  bool on = alert && (millis() % 2000UL < 150UL);
  // Translate the logical buzzer state into the configured electrical polarity.
  digitalWrite(BUZZER_PIN, (on == BUZZER_ACTIVE_HIGH) ? HIGH : LOW);
}

// Repeatedly service communication, sensors, calculations and outputs.
void loop() {

  // Process pending BLE events at the beginning of each iteration.
  serviceBluetooth();

  // Continuously monitor both IR sensors

  // Poll both optical inputs frequently; short pulses can still be missed during other operations.
  detectDrop(0);
  detectDrop(1);

  // Process Bed 1

  // Process Bed 1 weight independently, then poll both IR inputs again.
  readLoadCell(0);

  detectDrop(0);
  detectDrop(1);

  calculateFlow(0);
  updateStatus(0);

  // Process Bed 2

  // Repeat the same processing sequence for Bed 2.
  readLoadCell(1);

  detectDrop(0);
  detectDrop(1);

  calculateFlow(1);
  updateStatus(1);

  // Refresh local indicators regardless of BLE connection state.
  updateBuzzer();
  updateOLED();
  detectDrop(0);
  detectDrop(1);

  // Periodic system reporting

  unsigned long now = millis();

  // Use elapsed-time scheduling rather than a blocking reporting delay.
  if (now - lastReport >= REPORT_INTERVAL) {

    lastReport = now;

    // Send separate 20-byte notifications for the two beds.
    sendBedBluetooth(0);
    detectDrop(0);
    detectDrop(1);
    sendBedBluetooth(1);

    // Select optional JSON serial output or human-readable diagnostics.
    if (JSON_MODE) {

      sendSystemJSON();
    }

    else {

      printBed(0);

      printBed(1);
    }
  }
}

