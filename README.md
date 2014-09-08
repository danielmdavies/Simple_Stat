This is the Simple Stat. It is based off of Dan Steingart's Ardustat.

It is a tool that can be used for electrochemical testing using only using an arduino, a resistor and a capacitor. It is capable of both potentiostatic and galvanostatic methods.

Pre-requisites:

    Hardware:
    1. An Arduino (We use UNO)
    2. A 1000 Ohm Resistor
    3. A 100 uF Capacitor
    
    Software:
    1. Node.Js
    sudo apt-get install node.js
    sudo apt-get install npm
    2. Node packages - serialport, express, sleep
    npm install serialport
    npm install express
    npm install sleep
    3. Python - We recommend pithy (https://github.com/dansteingart/pithy)
    4. Arduino Interface
    (download from Arduino website)

Instructions:
1. Download Repository
2. Upload firmware code to arduino
3. Navigate to where simple_forwarder has been placed
4. Run node simple_forwarder.js with specifying the http port to use (8001), the serial port (/dev/ttyACM0), baud rate (57600) and buffer length (300).
5. Go nuts - Example Python code for electrochemical testing uploaded soon.


Contributors - Dan Davies, Dan Zirkel, Advisor - Dan Steingart
