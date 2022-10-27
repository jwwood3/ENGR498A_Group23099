#!/usr/bin/env python3
import serial
import random
import sys
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
                    r = 0
                    if len(sys.argv) == 1:
                        r = random.randrange(1,10)
                    else:
                        r = float(sys.argv[1])
                    print("Writing: "+str(r))
                    ser.write(("<1,"+str("8.4")+">").encode('utf-8'))
    else:
        ser.write(("<1,"+str(sys.argv[1])+">").encode('ascii'))
        #while True:
        #    if ser.in_waiting > 0:
        #        line = ser.readline().decode('utf-8',errors='ignore').rstrip()
        #        print(line)