#!/usr/bin/python3
from machine import Pin
import time
import random

## INITIALISATION
# NE PAS MODIFIER
out1 = Pin(33, Pin.OUT)    # GPIO33
out2 = Pin(32, Pin.OUT)    # GPIO32
out3 = Pin(26, Pin.OUT)    # GPIO26
out4 = Pin(27, Pin.OUT)    # GPIO27

# AJUSTER
step_sleep = 0.008 # Temps entre pas
max_steps = 150 # Nombre de pas maximum (156 hardlimit, 150 softlimit)

# DEBUT
out1.off()
out2.off()
out3.off()
out4.off()


def cleanup():
    out1.off()
    out2.off()
    out3.off()
    out4.off()

def stepOne():
    out1.off()
    out2.off()
    out3.off()
    out4.on()

def stepTwo():
    out1.off()
    out2.on()
    out3.off()
    out4.off()

def stepThree():
    out1.off()
    out2.off()
    out3.on()
    out4.off()

def stepFour():
    out1.on()
    out2.off()
    out3.off()
    out4.off()




try:
    while True:
        time.sleep(1)
        step_count = random.randint(10, max_steps)

        i = 0
        for i in range(step_count):
            if i%4==0:
                stepOne()
            elif i%4==1:
                stepTwo()
            elif i%4==2:
                stepThree()
            elif i%4==3:
                stepFour()

            time.sleep( step_sleep )

        i = 0
        for i in range(step_count):
            if i%4==1:
                stepFour()
            elif i%4==0:
                stepThree()
            elif i%4==3:
                stepTwo()
            elif i%4==2:
                stepOne()

            time.sleep( step_sleep )

except KeyboardInterrupt:
    cleanup()

cleanup()