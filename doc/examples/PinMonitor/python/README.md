# PinMonitor -- Python component

This is the Python half of the _PinMonitor_. It reads binary data from the USB
port and unpacks it into a Python `namedtuple`.

## Quick Start

**NOTE:** Using a Python virtual environment is highly recommended.

-   `pip install -r requirements.txt`
-   `python setup.py install`
-   (Re)start the Arduino.
-   `./pinmonitor/pinmonitor.py`

## Running the unit tests

The `binstruct` package has unit tests in the `binstruct/tests/` directory that
can be run with `python setup.py test` or `make test` for short.

