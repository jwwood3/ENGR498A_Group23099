#!/usr/bin/env python3
import serial
import random
import sys

def send(dat):
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.reset_input_buffer()
    ser.write("hi".encode('ascii'))
    while True:
        line = ser.readline().decode('utf-8',errors='ignore').rstrip()
        if 'START' in line:
            break
    out = "<"+str(len(dat))+","
    for i in dat:
        out += f"{i:.3f}"+","
    out += ">"
    ser.write((out).encode('ascii'))

if __name__ == '__main__':
    print(sys.argv[1])
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.reset_input_buffer()
    ser.write("hi".encode('ascii'))
    while True:
        line = ser.readline().decode('utf-8',errors='ignore').rstrip()
        if 'START' in line:
            break
    running=True
    if len(sys.argv) == 1:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8',errors='ignore').rstrip()
                print(line)
                if "UP" in line or "DOWN" in line:
                    running = True
                if "STOP" in line and running:
                    running=False
                    r = [0]*2
                    if len(sys.argv) == 1:
                        r = [random.randrange(1,10) for i in range(2)]
                    else:
                        r = [float(sys.argv[i]) for i in range(1,3)]
                    print("Writing: "+str(r))
                    ser.write(("<"+str("8.4")+">").encode('utf-8'))
    else:
        out = "<"+str(len(sys.argv)-1)+","
        for i in range(1,len(sys.argv)):
            out += str(sys.argv[i])
        out += ">"
        ser.write((out).encode('ascii'))
        #while True:
        #    if ser.in_waiting > 0:
        #        line = ser.readline().decode('utf-8',errors='ignore').rstrip()
        #        print(line)