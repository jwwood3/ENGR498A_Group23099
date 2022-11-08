# File Descriptions

## autoProcess.py

Defines function processAll(dir) which processes valid files in the given directory and sends the result to the arduino. Meant to be run on the raspberry pi. Currently stops after the first file for testing purposes.

## detectUSB.py

Detects plugging in a USB drive and automatically runs the processAll function from autoProcess.py. Meant to run on startup on the raspberry pi.

## graphProcess.py

Processes image and .xyz files into NxN resolution xyz files.

## imageConvert.py

Convert input image into .xyz file representation

## sendData.py

Initial test file for sending data to the arduino from the raspberry pi. Not usable code.

## sendDataMulti.py

Defines the send(z) function which sends all of the numbers in the array z to the arduino. Meant to be run from the raspberry pi.

## show.py

An initial attempt into gathering some of the data processing code into a cleaner file without excess functions that were written for testing other ideas
