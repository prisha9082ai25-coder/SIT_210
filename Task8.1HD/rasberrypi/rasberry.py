import asyncio
import speech_recognition as sr
from bleak import BleakClient, BleakScanner


# ---------------------------------------------------------
# 1. BLE CONFIGURATION
# ---------------------------------------------------------

# Bluetooth address of the Arduino Nano 33 IoT.
# Replace this if your Arduino's BLE address changes.
ARDUINO_ADDRESS = "94:B5:55:C0:7C:22"

# These UUIDs must match the Arduino firmware.
SERVICE_UUID = "7a1b0001-6d5a-4c4e-9a10-123456789001"
COMMAND_UUID = "7a1b0002-6d5a-4c4e-9a10-123456789001"
RESPONSE_UUID = "7a1b0003-6d5a-4c4e-9a10-123456789001"


# ---------------------------------------------------------
# 2. VOICE RECOGNITION CONFIGURATION
# ---------------------------------------------------------

# SpeechRecognition manages microphone input and speech recognition.
recognizer = sr.Recognizer()

# Allow a short pause before listening for the next command.
recognizer.pause_threshold = 0.8

# These are the commands this Python program understands.
# Keep these consistent with the Arduino's processCommand()
# function.
VALID_COMMANDS = {
    "bathroom on",
    "bathroom off",
    "hallway on",
    "hallway off",
    "fan on",
    "fan off",
    "automatic on",
    "automatic off",
    "light",
}


# ---------------------------------------------------------
# 3. NORMALISE AND CHECK A SPOKEN COMMAND
# ---------------------------------------------------------

def process_voice_text(text):
    """
    Convert recognised speech into a supported command.

    For example:
        "Bathroom On" becomes "bathroom on".

    Return None when the recognised words do not match a
    supported command.
    """

    # Remove unnecessary spaces and use lowercase text.
    command = text.strip().lower()

    # Accept only commands listed in VALID_COMMANDS.
    if command in VALID_COMMANDS:
        return command

    return None


# ---------------------------------------------------------
# 4. DISPLAY ARDUINO RESPONSES
# ---------------------------------------------------------

def handle_response(sender, data):
    """
    Display a notification received from the Arduino.

    The Arduino response characteristic sends text messages.
    """

    try:
        message = bytes(data).decode("utf-8")
        print(f"\nArduino response: {message}")

    except UnicodeDecodeError:
        print("\nReceived a response that could not be decoded.")


# ---------------------------------------------------------
# 5. LISTEN TO THE MICROPHONE
# ---------------------------------------------------------

def listen_for_command():
    """
    Record speech from the default microphone and convert it
    into text using Google's speech-recognition service.

    Internet access is required for recognize_google().
    """

    try:
        with sr.Microphone() as source:
            print("\nListening for a command...")

            # Adjust for background noise before listening.
            recognizer.adjust_for_ambient_noise(
                source,
                duration=0.5
            )

            # Listen until the user finishes speaking.
            audio = recognizer.listen(
                source,
                timeout=5,
                phrase_time_limit=5
            )

        print("Recognising speech...")

        # Convert the recorded speech into text.
        recognised_text = recognizer.recognize_google(audio)

        print(f"You said: {recognised_text}")

        # Check whether the recognised words are supported.
        command = process_voice_text(recognised_text)

        if command is None:
            print("Command not recognised by this program.")
            return None

        return command

    except sr.WaitTimeoutError:
        print("No speech detected. Please try again.")
        return None

    except sr.UnknownValueError:
        print("Sorry, I could not understand the speech.")
        return None

    except sr.RequestError as error:
        print(f"Speech recognition service error: {error}")
        return None

    except OSError as error:
        print(f"Microphone error: {error}")
        return None


# ---------------------------------------------------------
# 6. CONNECT TO THE ARDUINO AND SEND COMMANDS
# ---------------------------------------------------------

async def run_smart_lighting():
    """
    Scan for the Arduino, connect over BLE, and repeatedly
    listen for voice commands.

    The program remains active until the user says 'exit'
    or presses Ctrl+C.
    """

    print("Voice-Controlled Smart Lighting")
    print("--------------------------------")
    print("Looking for the Arduino over Bluetooth...")

    # Discover nearby BLE devices.
    # The address is checked to find the intended Arduino.
    device = await BleakScanner.find_device_by_address(
        ARDUINO_ADDRESS,
        timeout=10.0
    )

    if device is None:
        print("Arduino not found.")
        print("Check that it is powered on and advertising BLE.")
        return

    print(f"Found Arduino: {device.address}")

    try:
        # Establish a BLE connection with the Arduino.
        async with BleakClient(device) as client:
            print("Connected to Arduino successfully.")

            # Check that the Arduino provides the expected
            # command and response characteristics.
            command_char = client.services.get_characteristic(
                COMMAND_UUID
            )

            response_char = client.services.get_characteristic(
                RESPONSE_UUID
            )

            if command_char is None or response_char is None:
                print("Required BLE characteristics were not found.")
                print("Check the UUIDs in the Arduino firmware.")
                return

            # Subscribe to Arduino messages, if supported.
            try:
                await client.start_notify(
                    RESPONSE_UUID,
                    handle_response
                )
                print("Arduino response notifications enabled.")

            except Exception as error:
                print(f"Notifications unavailable: {error}")
                print("Commands may still be sent.")

            print("\nReady for voice commands.")
            print("Say 'exit' to disconnect and stop.")

            try:
                while client.is_connected:
                    # Speech recognition is synchronous, so run
                    # it in a worker thread to keep BLE responsive.
                    command = await asyncio.to_thread(
                        listen_for_command
                    )

                    # Nothing was recognised, so listen again.
                    if command is None:
                        continue

                    # Stop locally without sending 'exit' to Arduino.
                    if command == "exit":
                        break

                    print(f"Sending command: {command}")

                    try:
                        # Send the command as UTF-8 text.
                        # This matches the Arduino's BLE handler.
                        await client.write_gatt_char(
                            COMMAND_UUID,
                            command.encode("utf-8"),
                            response=True
                        )

                        print("Command sent successfully.")

                    except Exception as error:
                        print(f"Could not send command: {error}")

            except KeyboardInterrupt:
                print("\nStopping voice control...")

            finally:
                # Stop notifications when possible.
                try:
                    await client.stop_notify(RESPONSE_UUID)
                except Exception:
                    pass

    except Exception as error:
        print(f"BLE connection error: {error}")

    print("Smart lighting program stopped.")


# ---------------------------------------------------------
# 7. START THE PROGRAM
# ---------------------------------------------------------

if __name__ == "__main__":
    try:
        # Run the asynchronous BLE control program.
        asyncio.run(run_smart_lighting())

    except KeyboardInterrupt:
        print("\nProgram stopped by the user.")
