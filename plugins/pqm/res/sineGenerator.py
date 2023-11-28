import numpy as np
import time
import os

start_time = 0
end_time = 100 #number of buffers
sample_rate = 1000
acquisition_time = np.arange(start_time, end_time, 1 / sample_rate)
frequency = 10 
amplitude = 100
offset = 100 #for positive values
sinewave = amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 0) + offset
sinewave2 = amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 2 * np.pi / 3) + offset
sinewave3 = amplitude * np.sin(2 * np.pi * frequency * acquisition_time + 4 * np.pi / 3) + offset
straightLine = 0 * acquisition_time
bufferValues = np.ravel(
    [sinewave, sinewave2, sinewave3, sinewave, sinewave2, sinewave3, straightLine], "F"
)
output_file = open('data.bin', 'ab')
while True: 
    file_size = os.path.getsize(output_file.name)
    if file_size <= 0: 
        bufferValues = bufferValues.astype("uint32")
        bufferValues.tofile(output_file, "")
        output_file.flush()
    time.sleep(5)
