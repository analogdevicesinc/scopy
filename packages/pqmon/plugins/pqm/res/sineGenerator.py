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

import numpy as np
import time
import os

start_time = 0
end_time = 100  # number of buffers
sample_rate = 5100
acquisition_time = np.arange(start_time, end_time, 1 / sample_rate)
frequency = 50
offset = 0
voltage_amplitude = 330
current_amplitude = 2
voltage_sinewave0 = (
    voltage_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 0) + offset
)
voltage_sinewave1 = (
    voltage_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 2 * np.pi / 3)
    + offset
)
voltage_sinewave2 = (
    voltage_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 4 * np.pi / 3)
    + offset
)
current_sinewave0 = (
    current_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + np.pi / 6) + offset
)
current_sinewave1 = (
    current_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 5 * np.pi / 6)
    + offset
)
current_sinewave2 = (
    current_amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 9 * np.pi / 6)
    + offset
)
straightLine = 0 * acquisition_time
bufferValues = np.ravel(
    [
        voltage_sinewave0,
        voltage_sinewave1,
        voltage_sinewave2,
        current_sinewave0,
        current_sinewave1,
        current_sinewave2,
        straightLine,
    ],
    "F",
)
output_file = open("data.bin", "ab")
safety_guard = 0
while True:
    file_size = os.path.getsize(output_file.name)
    if file_size <= 0:
        safety_guard += 1
    if file_size <= 0 and safety_guard >= 2:
        print(safety_guard)
        safety_guard = 0
        bufferValues = bufferValues.astype("int16")
        bufferValues.tofile(output_file, "")
        output_file.flush()
    time.sleep(3)
