
# ============================================================
# SIT210 - EMBEDDED SYSTEMS DEVELOPMENT
# TASK 11.1P
# TWO-BED IoT IV MONITORING SYSTEM
#
# Raspberry Pi 4 - Backend Application
# Water-only academic demonstration prototype
#
# Main responsibilities:
# 1. Discover and connect to Arduino using Bluetooth LE.
# 2. Receive and decode binary sensor measurements.
# 3. Maintain independent data for both beds.
# 4. Host a Flask-based web dashboard.
# 5. Provide live measurements and historical data through APIs.
# 6. Automatically attempt reconnection after Bluetooth loss.
# ============================================================


# ============================================================
# 1. IMPORT REQUIRED LIBRARIES
# ============================================================

# Allows asynchronous Bluetooth operations without blocking
# the entire application.
import asyncio

# Converts binary Bluetooth packets into Python values.
import struct

# Allows Bluetooth reception and the Flask server to run
# concurrently in separate threads.
import threading

# Provides timing functions for detecting outdated measurements.
import time

# A fixed-length queue used to store recent sensor measurements.
from collections import deque

# Provides readable timestamps for dashboard measurements.
from datetime import datetime

# BleakScanner discovers nearby Bluetooth LE devices.
# BleakClient establishes connections and receives notifications.
from bleak import BleakClient, BleakScanner

# Flask creates the web server.
# jsonify converts Python data into JSON responses.
# render_template loads the HTML dashboard.
from flask import Flask, jsonify, render_template


# ============================================================
# 2. BLUETOOTH CONFIGURATION
# ============================================================

# Bluetooth device name advertised by the Arduino Nano 33 IoT.
DEVICE_NAME = "IV_Monitoring"

# UUID of the Arduino BLE characteristic containing sensor data.
# This must match the UUID defined in the Arduino firmware.
DATA_UUID = "7a2b0002-6d5a-4c4e-9a10-123456789001"

# Binary packet format matching the Arduino's 20-byte packet.
#
# < = Little-endian byte order
# B = Unsigned 8-bit integer (1 byte)
# H = Unsigned 16-bit integer (2 bytes)
# I = Unsigned 32-bit integer (4 bytes)
#
# Four B values  = 4 bytes
# Two H values   = 4 bytes
# One I value    = 4 bytes
# Four H values  = 8 bytes
#
# Total packet size = 20 bytes.

PACKET_FORMAT = "<BBBBHHIHHHH"

# Automatically calculate the expected packet size.
PACKET_SIZE = struct.calcsize(PACKET_FORMAT)

# Measurements older than eight seconds are considered stale.
# This prevents outdated readings from being presented as live data.
STALE_TIMEOUT = 8


# ============================================================
# 3. FLASK CONFIGURATION AND SHARED DATA
# ============================================================

# Create the Flask web application.
app = Flask(__name__)

# A thread lock protects shared data from simultaneous access
# by the Bluetooth receiver and Flask request handlers.
lock = threading.Lock()

# Tracks whether the Bluetooth receiver is connected.
connected = False

# Store the most recent measurement for each bed.
# None indicates that no measurement is currently available.
bed_data = {
    1: None,
    2: None
}

# Store a maximum of 150 historical measurements per bed.
# Once the limit is reached, the oldest entry is removed
# automatically when a new entry is added.
#
# This prevents unlimited memory consumption.
history = {
    1: deque(maxlen=150),
    2: deque(maxlen=150)
}

# Convert numerical status codes received from Arduino
# into readable messages for the dashboard.
STATUS_CODES = {
    0: "NORMAL",
    1: "LOW_VOLUME",
    2: "EMPTY",
    3: "NO_DROPS",
    4: "BOTTLE_REMOVED",
    5: "SENSOR_FAULT",
    6: "DISTURBANCE",
    7: "INITIALIZING"
}


# ============================================================
# 4. SENSOR VALUE DECODING
# ============================================================

def decode_value(value):

    # Arduino uses 65535 as a special marker for unavailable,
    # invalid or out-of-range measurements.
    #
    # Convert this marker into Python's None value.
    if value == 65535:
        return None

    # Arduino multiplies floating-point measurements by ten
    # before transmitting them as unsigned integers.
    #
    # Dividing by ten restores the transmitted decimal value.
    #
    # Example:
    # Received value = 2572
    # Decoded value  = 257.2
    return value / 10.0


# ============================================================
# 5. BLUETOOTH NOTIFICATION HANDLER
# ============================================================

# This callback executes whenever Arduino sends a BLE notification.
#
# sender identifies the characteristic responsible for the data.
# data contains the received binary packet.
def receive_data(sender, data):

    # Reject packets that do not match the expected size.
    # This prevents incorrect binary decoding.
    if len(data) != PACKET_SIZE:
        return

    # Extract individual measurements from the binary packet.
    #
    # The order must exactly match the Arduino firmware.
    (
        bed,
        status,
        sensor_ok,
        bottle_present,
        volume,
        percentage,
        drop_count,
        drops_per_minute,
        weight_flow,
        drop_flow,
        total_weight

    ) = struct.unpack(PACKET_FORMAT, data)

    # Only accept measurements belonging to Bed 1 or Bed 2.
    # Unexpected bed identifiers are ignored.
    if bed not in (1, 2):
        return

    # Restore the original decimal representation of flow values.
    weight_flow = decode_value(weight_flow)
    drop_flow = decode_value(drop_flow)

    # Initially mark the difference between flow methods
    # as unavailable.
    difference = None

    # Calculate the absolute difference only when both
    # flow measurements are available.
    #
    # If the optical drop factor has not been calibrated,
    # drop_flow remains None and no difference is calculated.
    if weight_flow is not None and drop_flow is not None:

        difference = abs(weight_flow - drop_flow)

    # Organize all decoded measurements into a dictionary.
    # Each key corresponds to a field used by the web dashboard.
    reading = {

        # Identifies the bed responsible for the measurement.
        "bed": bed,

        # Estimated remaining water volume in millilitres.
        "volume_ml": decode_value(volume),

        # Estimated remaining liquid percentage.
        "percentage": decode_value(percentage),

        # Cumulative number of accepted optical drops.
        "drop_count": drop_count,

        # Number of accepted drops per minute.
        "drops_per_minute": decode_value(drops_per_minute),

        # Flow estimated from weight changes over time.
        "weight_flow_mlh": weight_flow,

        # Optical volumetric flow, available when calibrated.
        "drop_flow_mlh": drop_flow,

        # Absolute difference between both flow methods.
        "flow_difference_mlh": difference,

        # Combined measured weight of the bottle and water.
        "total_weight_g": decode_value(total_weight),

        # Convert numerical sensor flags into Boolean values.
        "sensor_ok": bool(sensor_ok),

        # Indicates whether the bottle is detected as attached.
        "bottle_present": bool(bottle_present),

        # Convert the numerical status into readable text.
        # Unknown codes are displayed as UNKNOWN.
        "status": STATUS_CODES.get(status, "UNKNOWN"),

        # Human-readable time displayed on the dashboard.
        "last_updated": datetime.now().strftime("%H:%M:%S"),

        # Internal timestamp used for stale data detection.
        #
        # monotonic() is suitable for elapsed-time calculations
        # because system clock adjustments do not affect it.
        "timestamp": time.monotonic()
    }

    # Acquire the thread lock before modifying shared data.
    # The lock is automatically released after the block finishes.
    with lock:

        # Replace the previous reading for the corresponding bed.
        bed_data[bed] = reading

        # Add the latest measurement to the historical queue.
        # These values are used by the dashboard history graph.
        history[bed].append({
            "time": reading["last_updated"],
            "volume": reading["volume_ml"],
            "flow": weight_flow
        })

    # Print a short summary to the Raspberry Pi terminal.
    # This helps verify that Bluetooth measurements are arriving.
    print(
        f"Bed {bed} | "
        f"Volume: {reading['volume_ml']} | "
        f"Drops: {drop_count} | "
        f"Status: {reading['status']}"
    )


# ============================================================
# 6. AUTOMATIC BLUETOOTH CONNECTION
# ============================================================

# Asynchronous function responsible for Bluetooth discovery,
# connection, notification subscription and reconnection.
async def bluetooth_receiver():

    # Allows this function to modify the global connection flag.
    global connected

    # Keep attempting to establish a Bluetooth connection
    # throughout the lifetime of the application.
    while True:

        try:

            print("Searching for Arduino...")

            # Search for the Arduino using its advertised BLE name.
            #
            # await allows the asynchronous operation to complete
            # without blocking the Flask server's separate thread.
            device = await BleakScanner.find_device_by_name(
                DEVICE_NAME,
                timeout=20.0
            )

            # If Arduino is not discovered within 20 seconds,
            # wait briefly before attempting discovery again.
            if device is None:

                print("Arduino not found. Retrying...")

                await asyncio.sleep(3)

                continue

            # Establish a Bluetooth LE connection.
            #
            # The asynchronous context manager handles connection
            # setup and cleanup when this block exits.
            async with BleakClient(device) as client:

                # Subscribe to the Arduino data characteristic.
                #
                # Whenever a notification arrives, receive_data()
                # is automatically called to decode the packet.
                await client.start_notify(
                    DATA_UUID,
                    receive_data
                )

                # Update the shared connection flag only after
                # the notification subscription succeeds.
                with lock:
                    connected = True

                print("Arduino connected successfully")

                # Keep the Bluetooth connection context active
                # while the client remains connected.
                #
                # Check the connection approximately once per second.
                while client.is_connected:

                    await asyncio.sleep(1)

        # Handle Bluetooth-related exceptions without permanently
        # terminating the entire dashboard application.
        except Exception as error:

            print("Bluetooth error:", error)

        # This block executes when the connection attempt ends,
        # including when an exception or disconnection occurs.
        finally:

            with lock:

                # Inform the dashboard that BLE is disconnected.
                connected = False

                # Remove the previous live measurements.
                # This prevents old readings from appearing current.
                bed_data[1] = None
                bed_data[2] = None

        # Wait three seconds before attempting reconnection.
        #
        # Automatic reconnection improves recovery but does not
        # guarantee uninterrupted Bluetooth communication.
        await asyncio.sleep(3)


# ============================================================
# 7. BACKGROUND BLUETOOTH THREAD
# ============================================================

def start_bluetooth():

    # Create and run an asyncio event loop for the asynchronous
    # Bluetooth receiver.
    #
    # This function is executed in a separate background thread
    # so the Flask server can continue handling web requests.
    asyncio.run(bluetooth_receiver())


# ============================================================
# 8. DASHBOARD PAGE
# ============================================================

# The root URL displays the main monitoring dashboard.
#
# Example:
# http://localhost:5000/
@app.route("/")
def dashboard():

    # Load index.html from the templates directory.
    # Flask returns the rendered HTML to the browser.
    return render_template("index.html")


# ============================================================
# 9. LIVE SENSOR DATA API
# ============================================================

# This endpoint provides the latest measurements for both beds
# in JSON format.
#
# The dashboard JavaScript periodically requests this endpoint
# to update its displayed measurements.
@app.route("/api/data")
def get_data():

    # Record the current monotonic time for freshness checking.
    now = time.monotonic()

    # Protect shared data while preparing the API response.
    with lock:

        # Create the main response dictionary.
        result = {

            # Current Bluetooth connection state.
            "connected": connected,

            # Measurements and availability for both beds.
            "beds": []
        }

        # Process both beds independently.
        for bed in (1, 2):

            # Retrieve the most recent reading for this bed.
            reading = bed_data[bed]

            # If no measurement has arrived, return an
            # unavailable status instead of fabricated values.
            if reading is None:

                result["beds"].append({
                    "bed": bed,
                    "available": False,
                    "status": "WAITING_FOR_DATA"
                })

                continue

            # Check whether the latest measurement is older
            # than the configured eight-second timeout.
            #
            # Stale readings are not presented as live data.
            if now - reading["timestamp"] > STALE_TIMEOUT:

                result["beds"].append({
                    "bed": bed,
                    "available": False,
                    "status": "STALE_DATA"
                })

                continue

            # Copy the dictionary to avoid modifying the original
            # measurement stored in shared memory.
            current = reading.copy()

            # Mark the measurement as recently received.
            #
            # Individual sensor values may still be unavailable
            # if the Arduino reported invalid measurements.
            current["available"] = True

            # Remove the internal timing value because the
            # browser does not require it.
            current.pop("timestamp", None)

            # Add the processed bed measurement to the response.
            result["beds"].append(current)

    # Convert the completed Python dictionary into JSON
    # and return it as an HTTP response.
    return jsonify(result)


# ============================================================
# 10. MEASUREMENT HISTORY API
# ============================================================

# Provides recent historical measurements for both beds.
#
# The dashboard uses this endpoint to draw its history graphs.
@app.route("/api/history")
def get_history():

    # Protect the historical queues during access.
    with lock:

        # Convert each deque into a standard Python list
        # because deque objects are not directly JSON serializable.
        #
        # History is stored temporarily in memory and is lost
        # when the Python application restarts.
        return jsonify({
            "1": list(history[1]),
            "2": list(history[2])
        })


# ============================================================
# 11. MAIN PROGRAM
# ============================================================

# This block executes when app.py is launched directly.
if __name__ == "__main__":

    # Create a background thread for Bluetooth communication.
    #
    # daemon=True allows the background thread to terminate
    # when the main Python application exits.
    bluetooth_thread = threading.Thread(
        target=start_bluetooth,
        daemon=True
    )

    # Start Bluetooth discovery and reception in the background.
    bluetooth_thread.start()

    print("Starting IV Monitoring Dashboard")

    # Start the Flask web server.
    app.run(

        # Listen on all available network interfaces.
        #
        # This allows computers and mobile phones on the same
        # trusted local network to access the dashboard.
        host="0.0.0.0",

        # HTTP port used by the dashboard.
        port=5000,

        # Disable Flask debugging features.
        debug=False,

        # Disable automatic code reloading.
        #
        # This avoids creating duplicate application instances
        # and competing Bluetooth receiver threads.
        use_reloader=False
    )

# ============================================================
# END OF PROGRAM
#
# This application is intended for local academic demonstration.
# It is not a production or clinical monitoring application.
# ============================================================
