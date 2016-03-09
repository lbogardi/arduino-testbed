from collections import namedtuple
from struct import Struct

class BinStruct:
    """Unpack raw byte arrays into named tuples"""

    def __init__(self, name, endian, fields):
        self.define(name, endian, fields)

    def define(self, name, endian, fields):
        fields, field_types = zip(*fields)

        self._name = name
        self._endian = endian
        self._fields = list(fields)
        self._format = endian + ''.join(field_types)

        self._struct = Struct(self._format)
        self._namedtuple = namedtuple(self._name, self._fields)._make

    @property
    def size(self):
        return self._struct.size

    def unpack(self, buffer):
        return self._namedtuple(self._struct.unpack(bytearray(buffer)))

