**Important:** it should be clear that this project is not an official software to connect the SEVentilation control. It is also important to emphasize that the documentation of the protocol for the connection between the PC interface and the control unit is not available and all information about the connection had to be determined using measurements and logging.

**Use it at your own risk. Damage to the fan system cannot be ruled out.**

# Ventilation System

The connection between the "SEC-Touch Zentralregler" and the ESP8266 is made through a pluggable rising clamp. The following sketch shows the assignment of the connection points.

![sketch - connection between the esp8266 and sec-touch](/doc/sketch.jpg)

*** Important *** The PC Rx and Tx labels on the SEC-Touch diagram show which ESP32 pin is connected, There is no to connect TX to RX and RX to TX if you just follow the diagram. 

Communication with the PC interface takes place serially at 28800 baud. Further details about the connection can be found in the SEController.cpp class.

An MQTT bridge is currently implemented in the project. This is interchangeable and can be replaced or supplemented with a KNX connection, for example.

The final result should look like this:

![final result](/doc/final.jpg)


# Switching from MQTT to a Web Interface

To use a web-based interface for fan control instead of the MQTT bridge, you can replace the MQTT->Poll(); line in the loop() function with WebUI->loop();. This allows switching from MQTT to a webpage for controlling the fan system.

Note: It is not possible to enable both MQTT and the web interface simultaneously. Doing so would require adjustments to the control logic within the SEController class to handle multiple communication modes at the same time.