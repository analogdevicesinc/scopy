##
## This file is part of the libsigrokdecode project.
##
## Copyright (C) 2011-2014 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

import sigrokdecode as srd
from math import floor, ceil

'''
OUTPUT_PYTHON format:

Packet:
[<ptype>, <rxtx>, <pdata>]

This is the list of <ptype>s and their respective <pdata> values:
 - 'STARTBIT': The data is the (integer) value of the start bit (0/1).
 - 'DATA': This is always a tuple containing two items:
   - 1st item: the (integer) value of the UART data. Valid values
     range from 0 to 512 (as the data can be up to 9 bits in size).
   - 2nd item: the list of individual data bits and their ss/es numbers.
 - 'PARITYBIT': The data is the (integer) value of the parity bit (0/1).
 - 'STOPBIT': The data is the (integer) value of the stop bit (0 or 1).
 - 'INVALID STARTBIT': The data is the (integer) value of the start bit (0/1).
 - 'INVALID STOPBIT': The data is the (integer) value of the stop bit (0/1).
 - 'PARITY ERROR': The data is a tuple with two entries. The first one is
   the expected parity value, the second is the actual parity value.
 - TODO: Frame error?

The <rxtx> field is 0 for RX packets, 1 for TX packets.
'''

# Used for differentiating between the two data directions.
RX = 0
TX = 1

# Given a parity type to check (odd, even, zero, one), the value of the
# parity bit, the value of the data, and the length of the data (5-9 bits,
# usually 8 bits) return True if the parity is correct, False otherwise.
# 'none' is _not_ allowed as value for 'parity_type'.
def parity_ok(parity_type, parity_bit, data, num_data_bits):

    # Handle easy cases first (parity bit is always 1 or 0).
    if parity_type == 'zero':
        return parity_bit == 0
    elif parity_type == 'one':
        return parity_bit == 1

    # Count number of 1 (high) bits in the data (and the parity bit itself!).
    ones = bin(data).count('1') + parity_bit

    # Check for odd/even parity.
    if parity_type == 'odd':
        return (ones % 2) == 1
    elif parity_type == 'even':
        return (ones % 2) == 0

class SamplerateError(Exception):
    pass

class ChannelError(Exception):
    pass

class Decoder(srd.Decoder):
    api_version = 2
    id = 'uart'
    name = 'UART'
    longname = 'Universal Asynchronous Receiver/Transmitter'
    desc = 'Asynchronous, serial bus.'
    license = 'gplv2+'
    inputs = ['logic']
    outputs = ['uart']
    optional_channels = (
        # Allow specifying only one of the signals, e.g. if only one data
        # direction exists (or is relevant).
        {'id': 'rx', 'name': 'RX', 'desc': 'UART receive line'},
        {'id': 'tx', 'name': 'TX', 'desc': 'UART transmit line'},
    )
    options = (
        {'id': 'baudrate', 'desc': 'Baud rate', 'default': 115200},
        {'id': 'num_data_bits', 'desc': 'Data bits', 'default': 8,
            'values': (5, 6, 7, 8, 9)},
        {'id': 'parity_type', 'desc': 'Parity type', 'default': 'none',
            'values': ('none', 'odd', 'even', 'zero', 'one')},
        {'id': 'parity_check', 'desc': 'Check parity?', 'default': 'yes',
            'values': ('yes', 'no')},
        {'id': 'num_stop_bits', 'desc': 'Stop bits', 'default': 1.0,
            'values': (0.0, 0.5, 1.0, 1.5)},
        {'id': 'bit_order', 'desc': 'Bit order', 'default': 'lsb-first',
            'values': ('lsb-first', 'msb-first')},
        {'id': 'format', 'desc': 'Data format', 'default': 'ascii',
            'values': ('ascii', 'dec', 'hex', 'oct', 'bin')},
        {'id': 'invert_rx', 'desc': 'Invert RX?', 'default': 'no',
            'values': ('yes', 'no')},
        {'id': 'invert_tx', 'desc': 'Invert TX?', 'default': 'no',
            'values': ('yes', 'no')},
    )
    annotations = (
        ('rx-data', 'RX data'),
        ('tx-data', 'TX data'),
        ('rx-start', 'RX start bits'),
        ('tx-start', 'TX start bits'),
        ('rx-parity-ok', 'RX parity OK bits'),
        ('tx-parity-ok', 'TX parity OK bits'),
        ('rx-parity-err', 'RX parity error bits'),
        ('tx-parity-err', 'TX parity error bits'),
        ('rx-stop', 'RX stop bits'),
        ('tx-stop', 'TX stop bits'),
        ('rx-warnings', 'RX warnings'),
        ('tx-warnings', 'TX warnings'),
        ('rx-data-bits', 'RX data bits'),
        ('tx-data-bits', 'TX data bits'),
    )
    annotation_rows = (
        ('rx-data', 'RX', (0, 2, 4, 6, 8)),
        ('rx-data-bits', 'RX bits', (12,)),
        ('rx-warnings', 'RX warnings', (10,)),
        ('tx-data', 'TX', (1, 3, 5, 7, 9)),
        ('tx-data-bits', 'TX bits', (13,)),
        ('tx-warnings', 'TX warnings', (11,)),
    )
    binary = (
        ('rx', 'RX dump'),
        ('tx', 'TX dump'),
        ('rxtx', 'RX/TX dump'),
    )
    idle_state = ['WAIT FOR START BIT', 'WAIT FOR START BIT']

    def putx(self, rxtx, data):
        s, halfbit = self.startsample[rxtx], self.bit_width / 2.0
        self.put(s - floor(halfbit), self.samplenum + ceil(halfbit), self.out_ann, data)

    def putpx(self, rxtx, data):
        s, halfbit = self.startsample[rxtx], self.bit_width / 2.0
        self.put(s - floor(halfbit), self.samplenum + ceil(halfbit), self.out_python, data)

    def putg(self, data):
        s, halfbit = self.samplenum, self.bit_width / 2.0
        self.put(s - floor(halfbit), s + ceil(halfbit), self.out_ann, data)

    def putp(self, data):
        s, halfbit = self.samplenum, self.bit_width / 2.0
        self.put(s - floor(halfbit), s + ceil(halfbit), self.out_python, data)

    def putbin(self, rxtx, data):
        s, halfbit = self.startsample[rxtx], self.bit_width / 2.0
        self.put(s - floor(halfbit), self.samplenum + ceil(halfbit), self.out_binary, data)

    def __init__(self, **kwargs):
        self.samplerate = None
        self.samplenum = 0
        self.frame_start = [-1, -1]
        self.startbit = [-1, -1]
        self.cur_data_bit = [0, 0]
        self.databyte = [0, 0]
        self.paritybit = [-1, -1]
        self.stopbit1 = [-1, -1]
        self.startsample = [-1, -1]
        self.state = ['WAIT FOR START BIT', 'WAIT FOR START BIT']
        self.oldbit = [1, 1]
        self.oldpins = [-1, -1]
        self.databits = [[], []]

    def start(self):
        self.out_python = self.register(srd.OUTPUT_PYTHON)
        self.out_binary = self.register(srd.OUTPUT_BINARY)
        self.out_ann = self.register(srd.OUTPUT_ANN)

    def metadata(self, key, value):
        if key == srd.SRD_CONF_SAMPLERATE:
            self.samplerate = value
            # The width of one UART bit in number of samples.
            self.bit_width = float(self.samplerate) / float(self.options['baudrate'])

    # Return true if we reached the middle of the desired bit, false otherwise.
    def reached_bit(self, rxtx, bitnum):
        # bitpos is the samplenumber which is in the middle of the
        # specified UART bit (0 = start bit, 1..x = data, x+1 = parity bit
        # (if used) or the first stop bit, and so on).
        # The samples within bit are 0, 1, ..., (bit_width - 1), therefore
        # index of the middle sample within bit window is (bit_width - 1) / 2.
        bitpos = self.frame_start[rxtx] + (self.bit_width - 1) / 2.0
        bitpos += bitnum * self.bit_width
        if self.samplenum >= bitpos:
            return True
        return False

    def reached_bit_last(self, rxtx, bitnum):
        bitpos = self.frame_start[rxtx] + ((bitnum + 1) * self.bit_width)
        if self.samplenum >= bitpos:
            return True
        return False

    def wait_for_start_bit(self, rxtx, old_signal, signal):
        # The start bit is always 0 (low). As the idle UART (and the stop bit)
        # level is 1 (high), the beginning of a start bit is a falling edge.
        if not (old_signal == 1 and signal == 0):
            return

        # Save the sample number where the start bit begins.
        self.frame_start[rxtx] = self.samplenum

        self.state[rxtx] = 'GET START BIT'

    def get_start_bit(self, rxtx, signal):
        # Skip samples until we're in the middle of the start bit.
        if not self.reached_bit(rxtx, 0):
            return

        self.startbit[rxtx] = signal

        # The startbit must be 0. If not, we report an error.
        if self.startbit[rxtx] != 0:
            self.putp(['INVALID STARTBIT', rxtx, self.startbit[rxtx]])
            self.putg([rxtx + 10, ['Frame error', 'Frame err', 'FE']])
            # TODO: Abort? Ignore rest of the frame?

        self.cur_data_bit[rxtx] = 0
        self.databyte[rxtx] = 0
        self.startsample[rxtx] = -1

        self.state[rxtx] = 'GET DATA BITS'

        self.putp(['STARTBIT', rxtx, self.startbit[rxtx]])
        self.putg([rxtx + 2, ['Start bit', 'Start', 'S']])

    def get_data_bits(self, rxtx, signal):
        # Skip samples until we're in the middle of the desired data bit.
        if not self.reached_bit(rxtx, self.cur_data_bit[rxtx] + 1):
            return

        # Save the sample number of the middle of the first data bit.
        if self.startsample[rxtx] == -1:
            self.startsample[rxtx] = self.samplenum

        # Get the next data bit in LSB-first or MSB-first fashion.
        if self.options['bit_order'] == 'lsb-first':
            self.databyte[rxtx] >>= 1
            self.databyte[rxtx] |= \
                (signal << (self.options['num_data_bits'] - 1))
        else:
            self.databyte[rxtx] <<= 1
            self.databyte[rxtx] |= (signal << 0)

        self.putg([rxtx + 12, ['%d' % signal]])

        # Store individual data bits and their start/end samplenumbers.
        s, halfbit = self.samplenum, int(self.bit_width / 2)
        self.databits[rxtx].append([signal, s - halfbit, s + halfbit])

        # Return here, unless we already received all data bits.
        if self.cur_data_bit[rxtx] < self.options['num_data_bits'] - 1:
            self.cur_data_bit[rxtx] += 1
            return

        self.state[rxtx] = 'GET PARITY BIT'

        self.putpx(rxtx, ['DATA', rxtx,
            (self.databyte[rxtx], self.databits[rxtx])])

        b, f = self.databyte[rxtx], self.options['format']
        if f == 'ascii':
            c = chr(b) if b in range(30, 126 + 1) else '[%02X]' % b
            self.putx(rxtx, [rxtx, [c]])
        elif f == 'dec':
            self.putx(rxtx, [rxtx, [str(b)]])
        elif f == 'hex':
            self.putx(rxtx, [rxtx, [hex(b)[2:].zfill(2).upper()]])
        elif f == 'oct':
            self.putx(rxtx, [rxtx, [oct(b)[2:].zfill(3)]])
        elif f == 'bin':
            self.putx(rxtx, [rxtx, [bin(b)[2:].zfill(8)]])

        self.putbin(rxtx, [rxtx, bytes([b])])
        self.putbin(rxtx, [2, bytes([b])])

        self.databits = [[], []]

    def get_parity_bit(self, rxtx, signal):
        # If no parity is used/configured, skip to the next state immediately.
        if self.options['parity_type'] == 'none':
            self.state[rxtx] = 'GET STOP BITS'
            return

        # Skip samples until we're in the middle of the parity bit.
        if not self.reached_bit(rxtx, self.options['num_data_bits'] + 1):
            return

        self.paritybit[rxtx] = signal

        self.state[rxtx] = 'GET STOP BITS'

        if parity_ok(self.options['parity_type'], self.paritybit[rxtx],
                     self.databyte[rxtx], self.options['num_data_bits']):
            self.putp(['PARITYBIT', rxtx, self.paritybit[rxtx]])
            self.putg([rxtx + 4, ['Parity bit', 'Parity', 'P']])
        else:
            # TODO: Return expected/actual parity values.
            self.putp(['PARITY ERROR', rxtx, (0, 1)]) # FIXME: Dummy tuple...
            self.putg([rxtx + 6, ['Parity error', 'Parity err', 'PE']])

    # TODO: Currently only supports 1 stop bit.
    def get_stop_bits(self, rxtx, signal):
        # Skip samples until we're in the middle of the stop bit(s).
        skip_parity = 0 if self.options['parity_type'] == 'none' else 1
        b = self.options['num_data_bits'] + 1 + skip_parity
        if not self.reached_bit(rxtx, b):
            return

        self.stopbit1[rxtx] = signal

        # Stop bits must be 1. If not, we report an error.
        if self.stopbit1[rxtx] != 1:
            self.putp(['INVALID STOPBIT', rxtx, self.stopbit1[rxtx]])
            self.putg([rxtx + 10, ['Frame error', 'Frame err', 'FE']])
            # TODO: Abort? Ignore the frame? Other?

        self.state[rxtx] = 'WAIT FOR START BIT'

        self.putp(['STOPBIT', rxtx, self.stopbit1[rxtx]])
        self.putg([rxtx + 4, ['Stop bit', 'Stop', 'T']])

    def decode(self, ss, es, data):
        if not self.samplerate:
            raise SamplerateError('Cannot decode without samplerate.')
        for (self.samplenum, pins) in data:

            # We want to skip identical samples for performance reasons but,
            # for now, we can only do that when we are in the idle state
            # (meaning both channels are waiting for the start bit).
            if self.state == self.idle_state and self.oldpins == pins:
                continue

            self.oldpins, (rx, tx) = pins, pins

            if self.options['invert_rx'] == 'yes':
                rx = not rx
            if self.options['invert_tx'] == 'yes':
                tx = not tx

            # Either RX or TX (but not both) can be omitted.
            has_pin = [rx in (0, 1), tx in (0, 1)]
            if has_pin == [False, False]:
                raise ChannelError('Either TX or RX (or both) pins required.')

            # State machine.
            for rxtx in (RX, TX):
                # Don't try to handle RX (or TX) if not supplied.
                if not has_pin[rxtx]:
                    continue

                signal = rx if (rxtx == RX) else tx

                if self.state[rxtx] == 'WAIT FOR START BIT':
                    self.wait_for_start_bit(rxtx, self.oldbit[rxtx], signal)
                elif self.state[rxtx] == 'GET START BIT':
                    self.get_start_bit(rxtx, signal)
                elif self.state[rxtx] == 'GET DATA BITS':
                    self.get_data_bits(rxtx, signal)
                elif self.state[rxtx] == 'GET PARITY BIT':
                    self.get_parity_bit(rxtx, signal)
                elif self.state[rxtx] == 'GET STOP BITS':
                    self.get_stop_bits(rxtx, signal)

                # Save current RX/TX values for the next round.
                self.oldbit[rxtx] = signal
