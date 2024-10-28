#
# Copyright (c) 2024 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

import iio
import random
import time

context = iio.Context("ip:127.0.0.1")
device = context.find_device("pqm")
channelsList = device.channels

i = 0
while True:
    for ch in channelsList:
        rms = random.uniform(0.0, 100.0)
        angle = random.uniform(0.0, 360.0)
        harmonics_list = (
            [100]
            + random.sample(range(10, 80), 5)
            + random.sample(range(5, 40), 5)
            + random.sample(range(0, 5), 36, counts=[8, 8, 8, 8, 4])
            + [0, 0, 0, 0]
        )
        harmonics = map(str, harmonics_list)
        interharmonics_list = (
            [100]
            + random.sample(range(10, 80), 5)
            + random.sample(range(5, 40), 5)
            + random.sample(range(0, 5), 36, counts=[8, 8, 8, 8, 4])
            + [0, 0, 0, 0]
        )
        inter_harmonics = map(str, interharmonics_list)
        strHarmonics = " ".join(harmonics)
        strInterHarmonics = " ".join(inter_harmonics)
        ch.attrs["rms"]._write(str(rms))
        ch.attrs["angle"]._write(str(angle))
        if "harmonics" in ch.attrs:
            ch.attrs["harmonics"]._write(strHarmonics)
        if "inter_harmonics" in ch.attrs:
            ch.attrs["inter_harmonics"]._write(strInterHarmonics)
    time.sleep(3)
    