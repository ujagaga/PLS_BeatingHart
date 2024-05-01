#!/usr/bin/python3

import serial
import tkinter as tk


def set_speed(speed):
    ser = serial.Serial('/dev/ttyUSB0', 9600, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE)

    values = bytearray([0xa5, 0xa5, speed])
    ser.write(values)
    ser.close()


root = tk.Tk()
root.minsize(200, 50)
root.title("PLS Hart Speed")


def apply():
    speed = int(current_value.get())
    set_speed(speed)


def validate(inStr, acttyp):
    if acttyp == '1': #insert
        if not inStr.isdigit():
            return False
        try:
            value = int(inStr)
            if value < 0 or value > 100:
                return False
        except:
            return False
    return True


tk.Label(root, text="Brzina od 0 do 100:").pack()
current_value = tk.StringVar(value=0)
speed_entry = tk.Spinbox(root, from_=0, to=100,  textvariable=current_value, wrap=True, validate="key")
speed_entry['validatecommand'] = (speed_entry.register(validate), '%P', '%d')

sub_btn = tk.Button(root, text='Apply', command=apply)

speed_entry.pack()
sub_btn.pack()

root.mainloop()
