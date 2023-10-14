**Important:** it should be clear that this project is not an official software to connect the SEVentilation control. It is also important to emphasize that the documentation of the protocol for the connection between the PC interface and the control unit is not available and all information about the connection had to be determined using measurements and logging.

**Use it at your own risk. Damage to the fan system cannot be ruled out.**

# Ventilation System

The connection between the "SEC-Touch Zentralregler" and the ESP8266 is made through a pluggable rising clamp. The following sketch shows the assignment of the connection points.

![sketch - connection between the esp8266 and sec-touch](/doc/sketch.jpg)

Communication with the PC interface takes place serially at 28800 baud. Further details about the connection can be found in the SEController.cpp class.

An MQTT bridge is currently implemented in the project. This is interchangeable and can be replaced or supplemented with a KNX connection, for example.

The final result should look like this:

![final result](/doc/final.jpg)
