
import asyncio
import re
import speech_recognition as sr
from bleak import BleakClient, BleakScanner


# ============================================================
# SIT210 TASK 8.1HD - AUDIO PROCESSING AND VOICE ACTIVATED
# LIGHTING SYSTEM
#
# Raspberry Pi Responsibilities:
# 1. Capture audio through the connected microphone.
# 2. Convert spoken commands into text using speech recognition.
# 3. Correct common speech recognition errors.
# 4. Identify and validate supported voice commands.
# 5. Communicate with the Arduino Nano 33 IoT using BLE.
# 6. Receive and display responses from the Arduino.
#
# The Arduino controls the bathroom light, hallway light,
# light sensor and exhaust fan indicator.
#
# ============================================================


# ============================================================
# 1. BLUETOOTH CONFIGURATION
# ============================================================

# Bluetooth MAC address of the Arduino Nano 33 IoT.
# The Raspberry Pi uses this address to locate the device.

DEVICE_ADDRESS = "94:B5:55:C0:7C:22"

# UUID of the Bluetooth service provided by the Arduino.

SERVICE_UUID = "7a1b0001-6d5a-4c4e-9a10-123456789001"

# UUID of the characteristic used to send commands
# from the Raspberry Pi to the Arduino.

COMMAND_UUID = "7a1b0002-6d5a-4c4e-9a10-123456789001"

# UUID of the characteristic used to receive responses
# and notifications from the Arduino.

RESPONSE_UUID = "7a1b0003-6d5a-4c4e-9a10-123456789001"


# ============================================================
# 2. MICROPHONE CONFIGURATION
# ============================================================

# Select the microphone device connected to the Raspberry Pi.
# The device index identifies the microphone used for recording.

MICROPHONE_INDEX = 2

# Create a speech recogniser object to process microphone audio.

recognizer = sr.Recognizer()

# Defines the pause duration used to determine when the
# user has finished speaking.

recognizer.pause_threshold = 0.8


# ============================================================
# 3. VALID VOICE COMMANDS
# ============================================================

# These are the commands supported by the system.
# After speech recognition, the processed text must match
# one of these commands before it is sent to the Arduino.

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
    "exit"
}


# ============================================================
# 4. COMMAND ALIASES
# ============================================================

# Different spoken expressions can have the same meaning.
#
# This dictionary converts alternative phrases into a
# standard command format that the Arduino understands.
#
# For example:
# "turn on bathroom" becomes "bathroom on".
#
# This allows users to speak naturally without needing
# to remember one exact sentence.

COMMAND_ALIASES = {

    # Bathroom light commands
    "bathroom on": "bathroom on",
    "turn bathroom on": "bathroom on",
    "turn on bathroom": "bathroom on",
    "bathroom light on": "bathroom on",
    "switch bathroom on": "bathroom on",

    "bathroom off": "bathroom off",
    "turn bathroom off": "bathroom off",
    "turn off bathroom": "bathroom off",
    "bathroom light off": "bathroom off",
    "switch bathroom off": "bathroom off",

    # Hallway light commands
    "hallway on": "hallway on",
    "turn hallway on": "hallway on",
    "turn on hallway": "hallway on",
    "hallway light on": "hallway on",
    "switch hallway on": "hallway on",

    "hallway off": "hallway off",
    "turn hallway off": "hallway off",
    "turn off hallway": "hallway off",
    "hallway light off": "hallway off",
    "switch hallway off": "hallway off",

    # Exhaust fan commands
    "fan on": "fan on",
    "turn fan on": "fan on",
    "turn on fan": "fan on",
    "switch fan on": "fan on",

    "fan off": "fan off",
    "turn fan off": "fan off",
    "turn off fan": "fan off",
    "switch fan off": "fan off",

    # Automatic lighting mode commands
    "automatic on": "automatic on",
    "turn automatic on": "automatic on",
    "enable automatic": "automatic on",

    "automatic off": "automatic off",
    "turn automatic off": "automatic off",
    "disable automatic": "automatic off",

    # Light sensor reading commands
    "light": "light",
    "check light": "light",
    "light level": "light",

    # Program termination commands
    "exit": "exit",
    "stop": "exit",
    "quit": "exit"
}


# ============================================================
# 5. PROCESS RECOGNISED SPEECH
# ============================================================

# Processes the text received from speech recognition.
#
# The function:
# 1. Converts the text into lowercase.
# 2. Removes punctuation.
# 3. Corrects common recognition mistakes.
# 4. Standardises spaces.
# 5. Matches the processed text with supported commands.
#
# Returns a valid Arduino command or None if no match exists.

def process_voice_text(text):

    # Standardise the recognised text.

    text = text.lower().strip()

    # Remove punctuation to simplify command comparison.

    text = re.sub(r"[^\w\s]", "", text)

    # Common corrections for incorrectly recognised words.
    # These help improve command matching when speech
    # recognition produces minor spelling variations.

    corrections = {
        "nbathroom": "bathroom",
        "n bathroom": "bathroom",
        "holiday": "hallway",
        "hollyway": "hallway",
        "hall way": "hallway"
    }

    # Split the recognised sentence into individual words.

    words = text.split()

    # Replace recognised words using the correction dictionary.

    corrected_words = [
        corrections.get(word, word)
        for word in words
    ]

    # Reconstruct the corrected sentence.

    text = " ".join(corrected_words)

    # Remove unnecessary spaces between words.

    text = re.sub(r"\s+", " ", text).strip()

    print("Corrected speech:", text)

    # Compare the corrected sentence against the command aliases.

    command = COMMAND_ALIASES.get(text)

    # Return the standard command only if it is supported.

    if command in VALID_COMMANDS:

        return command

    # Reject speech that does not match a supported command.

    print("Unrecognised command:", text)

    return None


# ============================================================
# 6. SPEECH RECOGNITION FUNCTION
# ============================================================

# Captures the user's voice through the microphone and
# converts the recorded audio into text.
#
# Google Speech Recognition is used to identify the spoken
# words. The resulting text is passed to the command
# processing function for validation.

def listen_for_command():

    try:

        # Open the selected microphone for audio input.

        with sr.Microphone(
            device_index=MICROPHONE_INDEX
        ) as source:

            print("\n================================")
            print("MICROPHONE ACTIVE")
            print("Speak your command now...")
            print("================================")

            # Measure background noise and adjust the
            # recogniser to the surrounding environment.

            recognizer.adjust_for_ambient_noise(
                source,
                duration=0.5
            )

            print("Listening...")

            # Record the user's speech.
            # timeout: maximum time to wait for speech to begin.
            # phrase_time_limit: maximum duration of one command.

            audio = recognizer.listen(
                source,
                timeout=8,
                phrase_time_limit=7
            )

        print("\nAudio captured successfully.")

        print("Sending audio for speech recognition...")

        # Convert the captured audio into text using
        # Google's speech recognition service.
        #
        # en-IN specifies Indian English recognition.
        # show_all=True returns alternative recognition results.

        results = recognizer.recognize_google(
            audio,
            language="en-IN",
            show_all=True
        )

        # Check whether the recognition result is valid.

        if not isinstance(results, dict):

            print("No speech could be recognised.")

            return None

        # Extract alternative speech recognition results.

        alternatives = results.get("alternative", [])

        if not alternatives:

            print("No speech detected. Please try again.")

            return None

        print("\n--- SPEECH DETECTION RESULTS ---")

        # Examine the recognised speech alternatives until
        # a supported command is identified.

        for alternative in alternatives:

            recognised_text = alternative.get(
                "transcript", ""
            ).strip()

            if recognised_text:

                print("You said:", recognised_text)

                # Correct and validate the recognised speech.

                command = process_voice_text(
                    recognised_text
                )

                # Return the first valid recognised command.

                if command:

                    print("\nRecognised command:", command)

                    print("Sending command to Arduino...")

                    return command

        # No alternative matched the supported commands.

        print("\nSpeech detected, but no valid command matched.")

        return None

    # Handle situations where the user does not speak in time.

    except sr.WaitTimeoutError:

        print("No speech detected within the listening time.")

        return None

    # Handle speech that cannot be understood.

    except sr.UnknownValueError:

        print("Speech was captured, but could not be understood.")

        return None

    # Handle problems with the Google recognition service.

    except sr.RequestError as error:

        print("Google Speech Recognition error:", error)

        return None

    # Handle unexpected microphone or recording errors.

    except Exception as error:

        print("Microphone error:", error)

        return None


# ============================================================
# 7. BLUETOOTH RESPONSE HANDLER
# ============================================================

# Receives notifications from the Arduino.
#
# The Arduino sends response messages after processing
# commands, allowing the Raspberry Pi to display the
# current operation or response.

def notification_handler(sender, data):

    try:

        # Convert the received Bluetooth data into readable text.

        response = data.decode("utf-8")

        print("\nArduino response:", response)

    except Exception as error:

        print("Response error:", error)


# ============================================================
# 8. BLUETOOTH CONNECTION AND VOICE CONTROL
# ============================================================

# Main asynchronous function responsible for connecting
# the Raspberry Pi to the Arduino and managing voice control.
#
# Overall operation:
# 1. Search for the Arduino using its Bluetooth address.
# 2. Establish a BLE connection.
# 3. Locate the required BLE service and characteristics.
# 4. Enable notifications for Arduino responses.
# 5. Capture and recognise voice commands continuously.
# 6. Send validated commands to the Arduino.
# 7. Stop communication when the user requests exit.

async def main():

    print("Searching for SmartLighting Arduino...")

    # Search for the Arduino using its Bluetooth MAC address.

    device = await BleakScanner.find_device_by_address(
        DEVICE_ADDRESS,
        timeout=15.0
    )

    # Stop if the Arduino cannot be discovered.

    if device is None:

        print("Arduino device not found.")

        print("Check Arduino power and BLE advertising.")

        return

    print("Arduino found:", device.name)

    try:

        # Establish a Bluetooth Low Energy connection
        # with the discovered Arduino device.

        async with BleakClient(device) as client:

            # Verify that the connection was successful.

            if not client.is_connected:

                print("Bluetooth connection failed.")

                return

            print("\nConnected to SmartLighting Arduino.")

            # Retrieve the available Bluetooth services.

            services = client.services

            # Locate the service responsible for lighting control.

            service = services.get_service(SERVICE_UUID)

            if service is None:

                print("SmartLighting BLE service not found.")

                return

            # Find the command characteristic used to send
            # recognised voice commands to the Arduino.

            command_characteristic = service.get_characteristic(
                COMMAND_UUID
            )

            # Find the response characteristic used to receive
            # Arduino status messages and notifications.

            response_characteristic = service.get_characteristic(
                RESPONSE_UUID
            )

            # Verify that both required characteristics exist.

            if command_characteristic is None:

                print("Command characteristic not found.")

                return

            if response_characteristic is None:

                print("Response characteristic not found.")

                return

            # Subscribe to Arduino notifications so that
            # responses can be received automatically.

            await client.start_notify(
                RESPONSE_UUID,
                notification_handler
            )

            print("\nVoice control system is ready.")

            print("Available commands:")

            # Display all supported commands to the user.

            for command in sorted(VALID_COMMANDS):

                print("-", command)

            print("\nSay 'exit' to stop voice control.")

            # --------------------------------------------------
            # CONTINUOUS VOICE-CONTROL LOOP
            # --------------------------------------------------

            # Keep listening for commands while the Bluetooth
            # connection with the Arduino remains active.

            while client.is_connected:

                # Run microphone recording and speech
                # recognition in a separate thread.
                #
                # This prevents the synchronous microphone
                # operation from blocking the asyncio event loop.

                command = await asyncio.to_thread(
                    listen_for_command
                )

                # Ignore empty or unrecognised commands.

                if command is None:

                    continue

                # Stop the voice-control program when the
                # user says exit, stop or quit.

                if command == "exit":

                    print("Stopping voice control...")

                    break

                try:

                    # Send the validated command to the Arduino
                    # using its BLE command characteristic.
                    #
                    # The Arduino receives the text and processes
                    # the corresponding lighting or fan operation.

                    await client.write_gatt_char(
                        COMMAND_UUID,
                        command.encode("utf-8"),
                        response=True
                    )

                    print(
                        "Command sent successfully:",
                        command
                    )

                except Exception as error:

                    # Handle Bluetooth communication errors
                    # that occur while sending commands.

                    print("Bluetooth command error:", error)

            # Stop receiving Arduino notifications when
            # the voice-control loop finishes.

            await client.stop_notify(RESPONSE_UUID)

    except Exception as error:

        # Handle errors related to Bluetooth discovery,
        # connection, services or communication.

        print("Bluetooth connection error:", error)


# ============================================================
# 9. PROGRAM ENTRY POINT
# ============================================================

# This section starts the asynchronous main function
# when the Python file is executed directly.
#
# asyncio.run() manages the event loop required for
# Bluetooth communication and voice-control operations.

if __name__ == "__main__":

    try:

        asyncio.run(main())

    # Allow the user to stop the program safely using
    # Ctrl+C from the Raspberry Pi terminal.

    except KeyboardInterrupt:

        print("\nVoice control stopped by user.")
