import pygame
import serial
import time

# Connect to Arduino
arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)

# Initialize Xbox controller
pygame.init()
pygame.joystick.init()

joystick = pygame.joystick.Joystick(0)
joystick.init()

print("Controller connected:", joystick.get_name())

def map_axis_to_speed(value):
    """Convert joystick axis (-1 to 1) to PWM (0 to 255)."""
    return int(abs(value) * 255)

try:
    while True:
        pygame.event.pump()

        y_axis = joystick.get_axis(1)  # Forward/back
        x_axis = joystick.get_axis(3)  # Turn

        cmd = 'S'
        speed = 0

        if abs(y_axis) > 0.2:  # Forward/back control
            speed = map_axis_to_speed(y_axis)
            if y_axis < 0:
                cmd = 'F'
            else:
                cmd = 'B'
        elif abs(x_axis) > 0.2:  # Left/right turn
            speed = map_axis_to_speed(x_axis)
            if x_axis < 0:
                cmd = 'L'
            else:
                cmd = 'R'
        else:
            cmd = 'S'
            speed = 0

        message = f"{cmd},{speed}\n"
        arduino.write(message.encode())
        print(f"Sent: {message.strip()}")

        time.sleep(0.05)

except KeyboardInterrupt:
    print("Exiting...")
    arduino.write(b"S,0\n")
    arduino.close()
    pygame.quit()
