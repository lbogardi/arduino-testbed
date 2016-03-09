#!/usr/bin/env python
'''
PinMonitor -- Binary communication via USB between Arduino and Python
'''

from binstruct import BinStruct

import serial
import time


sleep_duration = .5 # second
comm_warmup_duration = 2 # seconds


def serial_connection():
    '''Return a new unopened serial connection.'''
    ser = serial.Serial()
    ser.baudrate = 9600
    ser.port = '/dev/ttyUSB0'
    ser.timeout = .2
    return ser

def hexdump(message, buf):
    return ('%s[%s]' %
            (message, ','.join('{:02X}'.format(ord(b)) for b in buf)))

def format_measdata(measdata):
    md = measdata._asdict()
    return '  ' + '\n  '.join(
        ['%s = %s' % (field, md[field]) for field in measdata._fields]);

def loop(ser):
    '''Print data received over serial connection `ser` in a loop.'''
    iter = 0
    buffer = []
    raw_data = BinStruct('MeasData', '<', [
        ('current_millis',  'L'),
        ('current_micros',  'L'),
        ('leftAnalog',      'H'),
        ('rightAnalog',     'H'),
        ('leftDigital',     '?'),
        ('rightDigital',    '?'),
        ('led_state',       '?'),
    ])

    while 1:
        iter += 1
        print('\n### %s\n### Iteration #%i\n### %s' % ('-'*70, iter, '-'*70))

        n_incoming_bytes = ser.in_waiting
        if n_incoming_bytes > 0:
            incoming_bytes = ser.read(n_incoming_bytes)
            buffer += incoming_bytes

            print(hexdump('Received %i bytes: ' % n_incoming_bytes,
                          incoming_bytes))

            while len(buffer) >= raw_data.size:
                print(hexdump('Internal buffer: ', buffer))
                bits = buffer[0:raw_data.size]
                buffer = buffer[raw_data.size:]

                print(hexdump('Decoding: ', bits))
                measdata = raw_data.unpack(bits)

                print('Decoded measurement data:\n%s' %
                      format_measdata(measdata))

            print(hexdump('Internal buffer: ', buffer))
            if 0 < len(buffer) < raw_data.size:
                print('ERROR: Buffer smaller than one data packet; dropped')
                buffer = []
        else:
            print('Receive buffer empty')

        print('Going to sleep for %0.3fs...' % sleep_duration)
        time.sleep(sleep_duration)
        print('...woke up')

def main():
    try:
        print('Opening serial connection...')
        ser = serial_connection()
        ser.open()
        print('Serial connection open')

        print('Going to sleep for %0.3fs...' % comm_warmup_duration)
        time.sleep(comm_warmup_duration)

        print('Starting loop...')
        loop(ser)
    except serial.SerialException as e:
        print('SerialException: %s' % str(e))
    finally:
        if ser.is_open:
            print('Closing serial connection')
            ser.close()


if __name__ == '__main__':
    main()

