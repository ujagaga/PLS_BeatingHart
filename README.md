# PLS Beating Hart #

This is a computer casing for our medical clinic. An Orange Py 5 is used as a desktop computer and placed in this plastic casing with two servo motors to make it look like it is beating. There is also an OLED display on the power button to show when it is working.

I soldered a couple of wires to the OrangePi5 built in power button, so I could attach an external one. The 3D model is built using FreeCad and printed on a 3D printer.

The servos used are SG90.

The OLED LCD is a 128x64 pixel 0.96" I2C display.

A wire is connected between UART1 on GPIO header of the RPI5 and pin 0 of the AtTiny85 board. To set the beating speed, send a message at baud 9600, starting with "0xA5 0xA5" and then the speed you want between 0 and 100:

       0xA5 0xA5 5

To make it simpler, I included a Python script: "hart_speed.py". Before you run it, you need to have TkInter installed.

       sudo apt install python3-tk



## Contact ##

* web: http://www.radinaradionica.com
* email: ujagaga@gmail.com

