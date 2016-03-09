# PinMonitor

The main topic of this example is communication with the Arduino from a Python
script using a binary protocol.

It demonstrates the following concepts:

-   A task scheduler for the Arduino tasks.
-   Unit tests for the task scheduler using _Google Test_ and a fake
    implementation of the Arduino library.
-   Example usage of the task scheduler for reading digital and analog inputs
    and transmitting them over serial while blinking the on-board LED.
-   Example usage of the _PySerial_ Python package to read the USB port for the
    serial data sent by the Arduino.
-   A Python package to define a binary structure and unpack it from an array
    of bytes.

More details in the other READMEs...

