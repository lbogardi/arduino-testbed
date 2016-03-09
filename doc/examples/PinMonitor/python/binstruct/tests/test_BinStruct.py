from binstruct import BinStruct
import struct

from collections import namedtuple


test_inputs = [
    {
        'name': 'little_endian_ulong_ulong_bool',
        'endian': '<',
        'fields': [('startup_ms',   'L',    1),
                   ('current_ms',   'L',    2),
                   ('led_state',    '?',    True)],
        'size': 9,
    },
    {
        'name': 'big_endian_ulong_ulong_bool',
        'endian': '>',
        'fields': [('startup_ms',   'L',    1),
                   ('current_ms',   'L',    2),
                   ('led_state',    '?',    True)],
        'size': 9,
    },
]


def split_test_input(test_input):
    """Split the test input into several useful parts and representations"""
    fields, field_names, values, format = [], [], [], test_input['endian']

    for (field_name, format_char, value) in test_input['fields']:
        fields.append((field_name, format_char))
        field_names.append(field_name)
        values.append(value)
        format += format_char

    return (fields, field_names, values, format)


def test_define():
    for test_input in test_inputs:
        #   Prepare the input
        (fields, field_names, _, format) = split_test_input(test_input)

        #   Create a new object
        meas_data = BinStruct(test_input['name'],
                              test_input['endian'],
                              fields)

        #   Check the hidden attributes
        assert(meas_data._name == test_input['name'])
        assert(meas_data._endian == test_input['endian'])
        assert(meas_data._fields == field_names)
        assert(meas_data._format == format)


def test_size():
    for test_input in test_inputs:
        #   Prepare the input
        (fields, _, _, _) = split_test_input(test_input)

        #   Create a new object
        meas_data = BinStruct(test_input['name'],
                              test_input['endian'],
                              fields)

        assert(meas_data.size == test_input['size'])


def test_unpack():
    for test_input in test_inputs:
        #   Prepare the input and the expected result
        (fields, field_names, values, format) = split_test_input(test_input)
        buffer = bytearray(struct.pack(format, *values))
        ReturnType = namedtuple(test_input['name'], field_names)
        expected_value = ReturnType(*values)

        #   Create a new object
        meas_data = BinStruct(test_input['name'],
                              test_input['endian'],
                              fields)

        #   Unpack a buffer into a namedtuple
        unpacked_data = meas_data.unpack(buffer)
        assert(unpacked_data == expected_value)

