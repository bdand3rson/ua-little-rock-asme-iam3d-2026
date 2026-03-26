import pygame
import serial
import time
import sys

SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 9600
DEADZONE = 0.15

def connect_arduino():
    try:
        arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print(f"Connected to Arduino on {SERIAL_PORT}")
        return arduino
    except serial.SerialException as e:
        print("ERROR: Could not connect to Arduino.")
        print(e)
        return None

def connect_controller():
    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        print("ERROR: No controller detected.")
        return None

    joystick = pygame.joystick.Joystick(0)
    joystick.init()
    print(f"Connected to controller: {joystick.get_name()}")
    return joystick

def apply_deadzone(value, deadzone=DEADZONE):
    if abs(value) < deadzone:
        return 0.0
    return value

def clamp(value, minimum, maximum):
    return max(minimum, min(maximum, value))

def safe_write(arduino, message):
    try:
        arduino.write(message.encode())
    except serial.SerialException as e:
        print("ERROR: Serial write failed.")
        print(e)
        raise

arduino = connect_arduino()
if arduino is None:
    sys.exit(1)

joystick = connect_controller()
if joystick is None:
    arduino.close()
    sys.exit(1)

try:
    while True:
        pygame.event.pump()

        if pygame.joystick.get_count() == 0:
            print("ERROR: Controller disconnected.")
            safe_write(arduino, "S\n")
            break

        # Read joystick axes
        throttle = -joystick.get_axis(1)   # left stick Y; invert so up = positive
        steer = joystick.get_axis(3)       # right stick X

        # Apply deadzone
        throttle = apply_deadzone(throttle)
        steer = apply_deadzone(steer)

        # Arcade drive mixing
        left = throttle + steer
        right = throttle - steer

        # Clamp to [-1, 1]
        left = clamp(left, -1.0, 1.0)
        right = clamp(right, -1.0, 1.0)

        # Convert to PWM range [-255, 255]
        left_speed = int(left * 255)
        right_speed = int(right * 255)

        # Send stop if centered
        if left_speed == 0 and right_speed == 0:
            safe_write(arduino, "S\n")
        else:
            command = f"D,{left_speed},{right_speed}\n"
            safe_write(arduino, command)

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nProgram stopped by user.")

except serial.SerialException:
    print("Serial communication error.")

except pygame.error as e:
    print("Pygame/controller error:")
    print(e)

except Exception as e:
    print("Unexpected error:")
    print(e)

finally:
    try:
        if arduino and arduino.is_open:
            arduino.write(b"S\n")
            arduino.close()
    except:
        pass

    pygame.quit()
    print("Clean shutdown complete.")