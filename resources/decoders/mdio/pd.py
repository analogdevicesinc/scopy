##
## This file is part of the libsigrokdecode project.
##
## Copyright (C) 2014 Aurelien Jacobs <aurel@gnuage.org>
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

class Decoder(srd.Decoder):
    api_version = 2
    id = 'mdio'
    name = 'MDIO'
    longname = 'Management Data Input/Output'
    desc = 'Half-duplex sync serial bus for MII management between MAC and PHY.'
    license = 'gplv2+'
    inputs = ['logic']
    outputs = ['mdio']
    channels = (
        {'id': 'mdc', 'name': 'MDC', 'desc': 'Clock'},
        {'id': 'mdio', 'name': 'MDIO', 'desc': 'Data'},
    )
    annotations = (
        ('mdio-data', 'MDIO data'),
        ('mdio-bits', 'MDIO bits'),
        ('errors', 'Human-readable errors'),
    )
    annotation_rows = (
        ('mdio-data', 'MDIO data', (0,)),
        ('mdio-bits', 'MDIO bits', (1,)),
        ('other', 'Other', (2,)),
    )

    def __init__(self):
        self.oldmdc = 0
        self.ss_block = -1
        self.samplenum = -1
        self.oldpins = None
        self.reset_decoder_state()

    def start(self):
        self.out_python = self.register(srd.OUTPUT_PYTHON)
        self.out_ann = self.register(srd.OUTPUT_ANN)

    def putw(self, data):
        self.put(self.ss_block, self.samplenum, self.out_ann, data)

    def putbit(self, mdio, start, stop):
        # Bit annotations.
        self.put(start, stop, self.out_ann, [1, ['%d' % mdio]])

    def putdata(self):
        # FIXME: Only pass data, no bits.
        # Pass MDIO bits and then data to the next PD up the stack.
        ss, es = self.mdiobits[-1][1], self.mdiobits[0][2]

        # self.put(ss, es, self.out_python, ['BITS', self.mdiobits])
        self.put(ss, es, self.out_python, ['DATA', self.mdiodata])

        # Bit annotations.
        for bit in self.mdiobits:
            self.put(bit[1], bit[2], self.out_ann, [1, ['%d' % bit[0]]])

        # Error annotation if an error happened.
        if self.error:
            self.put(self.ss_bit, self.es_error, self.out_ann, [2, [self.error]])
            return

        op = 'READ' if self.operation else 'WRITE'

        # Dataword annotations.
        if self.ss_preamble != -1:
            self.put(self.ss_preamble, self.ss_start, self.out_ann, [0, ['PREAMBLE']])
        self.put(self.ss_start, self.ss_operation, self.out_ann, [0, ['START']])
        self.put(self.ss_operation, self.ss_phy, self.out_ann, [0, [op]])
        self.put(self.ss_phy, self.ss_reg, self.out_ann, [0, ['PHY: %d' % self.phy]])
        self.put(self.ss_reg, self.ss_turnaround, self.out_ann, [0, ['REG: %d' % self.reg]])
        self.put(self.ss_turnaround, self.ss_data, self.out_ann, [0, ['TURNAROUND']])
        self.put(self.ss_data, self.es_data, self.out_ann, [0, ['DATA: %04X' % self.data]])

    def reset_decoder_state(self):
        self.mdiodata = 0
        self.mdiobits = []
        self.bitcount = 0
        self.ss_preamble = -1
        self.ss_start = -1
        self.ss_operation = -1
        self.ss_phy = -1
        self.ss_reg = -1
        self.ss_turnaround = -1
        self.ss_data = -1
        self.phy = 0
        self.phy_bits = 0
        self.reg = 0
        self.reg_bits = 0
        self.data = 0
        self.data_bits = 0
        self.state = 'PREAMBLE'
        self.error = None

    def parse_preamble(self, mdio):
        if self.ss_preamble == -1:
            self.ss_preamble = self.samplenum
        if mdio != 1:
            self.error = 'Invalid preamble: could not find 32 consecutive bits set to 1'
            self.state = 'ERROR'
        elif self.bitcount == 31:
            self.state = 'START'

    def parse_start(self, mdio):
        if self.ss_start == -1:
            if mdio != 0:
                self.error = 'Invalid start bits: should be 01'
                self.state = 'ERROR'
            else:
                self.ss_start = self.samplenum
        else:
            if mdio != 1:
                self.error = 'Invalid start bits: should be 01'
                self.state = 'ERROR'
            else:
                self.state = 'OPERATION'

    def parse_operation(self, mdio):
        if self.ss_operation == -1:
            self.ss_operation = self.samplenum
            self.operation = mdio
        else:
            if mdio == self.operation:
                self.error = 'Invalid operation bits'
                self.state = 'ERROR'
            else:
                self.state = 'PHY'

    def parse_phy(self, mdio):
        if self.ss_phy == -1:
            self.ss_phy = self.samplenum
        self.phy_bits += 1
        self.phy |= mdio << (5 - self.phy_bits)
        if self.phy_bits == 5:
            self.state = 'REG'

    def parse_reg(self, mdio):
        if self.ss_reg == -1:
            self.ss_reg = self.samplenum
        self.reg_bits += 1
        self.reg |= mdio << (5 - self.reg_bits)
        if self.reg_bits == 5:
            self.state = 'TURNAROUND'

    def parse_turnaround(self, mdio):
        if self.ss_turnaround == -1:
            if self.operation == 0 and mdio != 1:
                self.error = 'Invalid turnaround bits'
                self.state = 'ERROR'
            else:
                self.ss_turnaround = self.samplenum
        else:
            if mdio != 0:
                self.error = 'Invalid turnaround bits'
                self.state = 'ERROR'
            else:
                self.state = 'DATA'

    def parse_data(self, mdio):
        if self.ss_data == -1:
            self.ss_data = self.samplenum
        self.data_bits += 1
        self.data |= mdio << (16 - self.data_bits)
        if self.data_bits == 16:
            self.es_data = self.samplenum + int((self.samplenum - self.ss_data) / 15)
            self.state = 'DONE'

    def parse_error(self, mdio):
        if self.bitcount == 63:
            self.es_error = self.samplenum + int((self.samplenum - self.ss_bit) / 63)
            self.state = 'DONE'

    def handle_bit(self, mdio):
        # If this is the first bit of a command, save its sample number.
        if self.bitcount == 0:
            self.ss_bit = self.samplenum
            # No preamble?
            if mdio == 0:
                self.state = 'START'

        # Guesstimate the endsample for this bit (can be overridden below).
        es = self.samplenum
        if self.bitcount > 0:
            es += self.samplenum - self.mdiobits[0][1]

        self.mdiobits.insert(0, [mdio, self.samplenum, es])

        if self.bitcount > 0:
            self.bitsamples = (self.samplenum - self.ss_bit) / self.bitcount
            self.mdiobits[1][2] = self.samplenum

        if self.state == 'PREAMBLE':
            self.parse_preamble(mdio)
        elif self.state == 'START':
            self.parse_start(mdio)
        elif self.state == 'OPERATION':
            self.parse_operation(mdio)
        elif self.state == 'PHY':
            self.parse_phy(mdio)
        elif self.state == 'REG':
            self.parse_reg(mdio)
        elif self.state == 'TURNAROUND':
            self.parse_turnaround(mdio)
        elif self.state == 'DATA':
            self.parse_data(mdio)
        elif self.state == 'ERROR':
            self.parse_error(mdio)

        self.bitcount += 1
        if self.state == 'DONE':
            self.putdata()
            self.reset_decoder_state()

    def find_mdc_edge(self, mdc, mdio):
        # Output the current error annotation if the clock stopped running
        if self.state == 'ERROR' and self.samplenum - self.clocksample > (1.5 * self.bitsamples):
            self.es_error = self.clocksample + int((self.clocksample - self.ss_bit) / self.bitcount)
            self.putdata()
            self.reset_decoder_state()

        # Ignore sample if the clock pin hasn't changed.
        if mdc == self.oldmdc:
            return

        self.oldmdc = mdc

        if mdc == 0:   # Sample on rising clock edge.
            return

        # Found the correct clock edge, now get/handle the bit(s).
        self.clocksample = self.samplenum
        self.handle_bit(mdio)

    def decode(self, ss, es, data):
        for (self.samplenum, pins) in data:
            # Ignore identical samples early on (for performance reasons).
            if self.oldpins == pins:
                continue
            self.oldpins, (mdc, mdio) = pins, pins

            self.find_mdc_edge(mdc, mdio)
