# PinMonitor -- Arduino component

This is the Arduino half of the _PinMonitor_. It reads two analog and two
digital inputs, blinks the on-board LED and transmits the status of the system
over USB.

## Quick Start

-   Wire-up an Arduino with some pots and switches according to the _Fritzing_
    diagram `doc/circuit_sketch.fzz`.
-   Check the `Makefile`, set your `BOARD_TAG` and `MONITOR_PORT`.
-   Connect the Arduino via USB and `make upload`.
-   The on-board LED should blink. You can also `make monitor` to see the
    serial stream. Or you can fire up the other half of this example.

## Running the unit tests

The `TaskScheduler` class comes with unit tests that use _Google Test_ and
implement a fake Arduino library.

The tests can be executed by running `make` (or `make test`) in the `test/`
directory.

