```python
# Import the tkinter library to create the Graphical User Interface (GUI).
# We use 'tk' as a short name to make it easier to write tkinter commands.
import tkinter as tk

# Import PWMLED to control LED brightness using PWM.
# Import LED to turn normal LEDs ON and OFF.
from gpiozero import PWMLED, LED


# -----------------------------------------
# LED SETUP
# -----------------------------------------

# Create a PWM-controlled LED for the Living Room.
# GPIO pin 18 is connected to the Living Room LED.
# PWMLED allows us to adjust the brightness of the LED.
living_room = PWMLED(18)

# Create a normal LED for the Bathroom.
# GPIO pin 27 is connected to the Bathroom LED.
# LED allows us to switch the LED ON or OFF.
bathroom = LED(27)

# Create a normal LED for the Closet.
# GPIO pin 22 is connected to the Closet LED.
closet = LED(22)


# -----------------------------------------
# FUNCTION TO SELECT A ROOM
# -----------------------------------------

def select_room():

    # Get the room selected by the user through the radio buttons.
    # room.get() returns the currently selected room's name.
    selected = room.get()

    # First, turn OFF all three LEDs.
    # This ensures that only the selected room's light remains ON.
    living_room.off()
    bathroom.off()
    closet.off()

    # Check whether the user selected the Living Room.
    if selected == "Living Room":

        # Get the brightness value from the slider.
        # The slider value is between 0 and 100.
        # PWMLED requires a value between 0 and 1.
        # Dividing by 100 converts the percentage into this range.
        # For example, 50 / 100 = 0.5, which means 50% brightness.
        living_room.value = brightness.get() / 100

    # Otherwise, check whether the Bathroom was selected.
    elif selected == "Bathroom":

        # Turn ON the Bathroom LED at full brightness.
        bathroom.on()

    # Otherwise, check whether the Closet was selected.
    elif selected == "Closet":

        # Turn ON the Closet LED at full brightness.
        closet.on()


# -----------------------------------------
# FUNCTION TO CHANGE LIVING ROOM BRIGHTNESS
# -----------------------------------------

# This function is called whenever the brightness slider changes.
# The slider automatically passes its current value to this function.
def change_brightness(value):

    # Convert the slider value into a floating-point number.
    # A float can contain decimal values, such as 50.0.
    brightness_value = float(value)

    # Convert the percentage into a value between 0 and 1.
    # For example:
    # 0   becomes 0.0 (OFF)
    # 50  becomes 0.5 (50% brightness)
    # 100 becomes 1.0 (full brightness)
    living_room.value = brightness_value / 100


# -----------------------------------------
# FUNCTION TO EXIT THE PROGRAM
# -----------------------------------------

def exit_program():

    # Turn OFF the Living Room LED before closing the program.
    living_room.off()

    # Turn OFF the Bathroom LED.
    bathroom.off()

    # Turn OFF the Closet LED.
    closet.off()

    # Close the GUI window.
    window.destroy()


# -----------------------------------------
# CREATE THE MAIN GUI WINDOW
# -----------------------------------------

# Create the main application window.
window = tk.Tk()

# Set the title displayed at the top of the window.
window.title("House Light Control")

# Set the window size to 400 pixels wide and 400 pixels high.
window.geometry("400x400")


# -----------------------------------------
# ADD THE HEADING
# -----------------------------------------

# Create a label to display the application heading.
title = tk.Label(
    window,                         # The window in which the label appears.
    text="House Light Control",     # Text displayed on the label.
    font=("Arial", 18, "bold")      # Font family, size, and style.
)

# Display the heading in the window.
# pady=20 adds 20 pixels of vertical space around the label.
title.pack(pady=20)


# -----------------------------------------
# CREATE THE RADIO BUTTON VARIABLE
# -----------------------------------------

# Create a StringVar to store the selected room's name.
# Radio buttons that share this variable belong to the same group.
# Only one room can be selected at a time.
room = tk.StringVar()


# -----------------------------------------
# CREATE THE LIVING ROOM RADIO BUTTON
# -----------------------------------------

living_button = tk.Radiobutton(
    window,                         # Parent window.
    text="Living Room",             # Text shown beside the radio button.
    variable=room,                  # Variable storing the selected option.
    value="Living Room",            # Value stored when this option is selected.
    command=select_room,             # Function called when selected.
    font=("Arial", 12)              # Font style and size.
)

# Display the Living Room radio button.
# anchor="w" aligns it to the left.
# padx=100 adds horizontal spacing.
# pady=5 adds vertical spacing.
living_button.pack(anchor="w", padx=100, pady=5)


# -----------------------------------------
# CREATE THE BATHROOM RADIO BUTTON
# -----------------------------------------

bathroom_button = tk.Radiobutton(
    window,
    text="Bathroom",
    variable=room,                  # Shares the same variable as other buttons.
    value="Bathroom",               # Value stored when selected.
    command=select_room,             # Calls the room selection function.
    font=("Arial", 12)
)

# Display the Bathroom radio button.
bathroom_button.pack(anchor="w", padx=100, pady=5)


# -----------------------------------------
# CREATE THE CLOSET RADIO BUTTON
# -----------------------------------------

closet_button = tk.Radiobutton(
    window,
    text="Closet",
    variable=room,                  # Shares the same selection variable.
    value="Closet",                 # Value stored when selected.
    command=select_room,             # Calls the room selection function.
    font=("Arial", 12)
)

# Display the Closet radio button.
closet_button.pack(anchor="w", padx=100, pady=5)


# -----------------------------------------
# ADD THE BRIGHTNESS SLIDER LABEL
# -----------------------------------------

# Create a label explaining what the slider controls.
brightness_label = tk.Label(
    window,
    text="Living Room Brightness",
    font=("Arial", 12)
)

# Display the label.
# The tuple (20, 5) adds 20 pixels above and 5 pixels below.
brightness_label.pack(pady=(20, 5))


# -----------------------------------------
# CREATE THE BRIGHTNESS SLIDER
# -----------------------------------------

brightness = tk.Scale(
    window,                         # Parent window.
    from_=0,                        # Minimum slider value: 0%.
    to=100,                         # Maximum slider value: 100%.
    orient=tk.HORIZONTAL,           # Display the slider horizontally.
    length=250,                     # Set the slider length to 250 pixels.
    command=change_brightness       # Call this function when the slider moves.
)

# Set the initial brightness slider value to 100%.
brightness.set(100)

# Display the brightness slider.
brightness.pack()


# -----------------------------------------
# CREATE THE EXIT BUTTON
# -----------------------------------------

exit_button = tk.Button(
    window,
    text="Exit",                    # Text displayed on the button.
    command=exit_program,           # Function called when clicked.
    width=12                       # Button width.
)

# Display the Exit button.
# pady=25 adds vertical spacing around the button.
exit_button.pack(pady=25)


# -----------------------------------------
# START THE GUI APPLICATION
# -----------------------------------------

# Start Tkinter's event loop.
# This keeps the window open and waits for user actions,
# such as selecting a room, moving the slider, or clicking Exit.
window.mainloop()
```
