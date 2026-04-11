import pygame
import serial
import serial.tools.list_ports
import time
import sys

BAUD_RATE = 9600
DEADZONE = 0.15
PREFERRED_PORT = "/dev/serial/by-id/usb-Arduino__www.arduino.cc__0042_43439353536351505042-if00"

def find_arduino():
    try:
        ser = serial.Serial(PREFERRED_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print(f"Connected to Arduino on {PREFERRED_PORT}")
        return ser
    except:
        pass

    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "Arduino" in port.description or "ttyACM" in port.device:
            try:
                ser = serial.Serial(port.device, BAUD_RATE, timeout=1)
                time.sleep(2)
                print(f"Connected to Arduino on {port.device}")
                return ser
            except:
                continue

    return None

def wait_for_arduino():
    while True:
        arduino = find_arduino()
        if arduino is not None:
            return arduino
        print("Waiting for Arduino...")
        time.sleep(1)

def wait_for_controller():
    pygame.init()
    pygame.joystick.init()

    while pygame.joystick.get_count() == 0:
        print("Waiting for Xbox controller...")
        pygame.event.pump()
        time.sleep(1)

    js = pygame.joystick.Joystick(0)
    js.init()
    print("Controller connected:", js.get_name())
    return js

def apply_deadzone(value, deadzone=DEADZONE):
    if abs(value) < deadzone:
        return 0.0
    return value

def safe_write(arduino, msg):
    try:
        arduino.write(msg.encode())
    except serial.SerialException as e:
        print("Serial write failed:", e)
        raise

arduino = wait_for_arduino()
joystick = wait_for_controller()

try:
    while True:
        pygame.event.pump()

        left_input = -joystick.get_axis(1)
        right_input = -joystick.get_axis(3)

        left_input = apply_deadzone(left_input)
        right_input = apply_deadzone(right_input)

        left_speed = int(left_input * 255)
        right_speed = int(right_input * 255)

        safe_write(arduino, f"DRIVE,{left_speed},{right_speed}\n")

        if joystick.get_button(0):       # A
            safe_write(arduino, "BOOM,255\n")
        elif joystick.get_button(1):     # B
            safe_write(arduino, "BOOM,-255\n")
        else:
            safe_write(arduino, "BOOM,0\n")

        if joystick.get_button(2):       # X
            safe_write(arduino, "DIPPER,255\n")
        elif joystick.get_button(3):     # Y
            safe_write(arduino, "DIPPER,-255\n")
        else:
            safe_write(arduino, "DIPPER,0\n")

        hat = joystick.get_hat(0)

        if hat == (0, 1):
            safe_write(arduino, "BUCKET,255\n")
        elif hat == (0, -1):
            safe_write(arduino, "BUCKET,-255\n")
        else:
            safe_write(arduino, "BUCKET,0\n")

        if hat == (-1, 0):
            safe_write(arduino, "BASE,-1\n")
        elif hat == (1, 0):
            safe_write(arduino, "BASE,1\n")
        else:
            safe_write(arduino, "BASE,0\n")

        time.sleep(0.05)

except KeyboardInterrupt:
    print("Stopping...")
    try:
        arduino.write(b"STOP_ALL\n")
    except:
        pass
    arduino.close()
    pygame.quit()
    sys.exit(0)

except Exception as e:
    print("Unexpected error:", e)
    try:
        arduino.write(b"STOP_ALL\n")
    except:
        pass
    try:
        arduino.close()
    except:
        pass
    pygame.quit()
    sys.exit(1)