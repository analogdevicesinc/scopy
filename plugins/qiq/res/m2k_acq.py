import libm2k
import numpy as np
import os
import argparse

def write_shared_file(file, data):
    print("Received data length:", len(data))
    with open("read.lock", "w") as f:
        f.write("locked")
    with open(file, "w+b") as f:
        f.write(data)  # Save data as binary
        f.flush()
        os.fsync(f.fileno())
    os.remove("read.lock")
    print("Data written and lock released.")
    print("Data saved to", file)

parser = argparse.ArgumentParser(description="Generate and transmit waveform using M2k.")
parser.add_argument("-p", "--path", help="Path to shared file")
parser.add_argument("--init", action="store_true", help="Path to shared file")

args = parser.parse_args()

if not args.path:
    print("Error: No path provided for the shared file.")
    exit("Please provide a path to the shared file using -p or --path argument.")

NB_OUT_SAMPLES = 1024

ctx=libm2k.m2kOpen()
if ctx is None:
	print("Connection Error: No ADALM2000 device available/connected to your PC.")
	exit(1)

ain=ctx.getAnalogIn()
aout=ctx.getAnalogOut()
trig=ain.getTrigger()

# Prevent bad initial config for ADC and DAC
ain.reset()
aout.reset()

ctx.calibrateADC()
ctx.calibrateDAC()

ain.enableChannel(0,True)
ain.enableChannel(1,True)
ain.setSampleRate(100000)
ain.setRange(0,-10,10)
ain.setKernelBuffersCount(1)

### uncomment the following block to enable triggering
#trig.setAnalogSource(0) # Channel 0 as source
#trig.setAnalogCondition(0,libm2k.RISING_EDGE_ANALOG)
#trig.setAnalogLevel(0,0.5)  # Set trigger level at 0.5
#trig.setAnalogDelay(0) # Trigger is centered
#trig.setAnalogMode(1, libm2k.ANALOG)

aout.setSampleRate(0, 750000)
aout.setSampleRate(1, 750000)
aout.enableChannel(0, True)
aout.enableChannel(1, True)

x=np.linspace(-np.pi, np.pi, NB_OUT_SAMPLES)
buffer1=np.linspace(-3.0, 3.00, NB_OUT_SAMPLES)
buffer2=np.sin(x)

aout.setCyclic(True)

# Convert every voltage sample from the signal to its raw value.
# The library method expects a short* (C++), but in Python a bytearray is accepted.
# Change the type of the array from 'int64' to 'int16'.
buffer1_raw = np.array([aout.convertVoltsToRaw(0, item) for item in buffer1])
buffer1_raw = buffer1_raw.astype('int16')
buffer1_raw = bytearray(buffer1_raw)
buffer2_raw = np.array([aout.convertVoltsToRaw(1, item) for item in buffer2])
buffer2_raw = buffer2_raw.astype('int16')
buffer2_raw = bytearray(buffer2_raw)

aout.pushRawBytes(0, buffer1_raw, NB_OUT_SAMPLES)
aout.pushRawBytes(1, buffer2_raw, NB_OUT_SAMPLES)

shared_file_path = args.path  # Path to shared file
if args.init:
    ain.stopAcquisition()
    data = ain.getSamplesRawInterleaved(8000)	# allows a memory view
    data = data.tobytes()				# convert the memory view to a readable bytearray
    write_shared_file(shared_file_path, data)
else:
    while True:
        if not os.path.exists("write.lock"):
            ain.stopAcquisition()
            data = ain.getSamplesRawInterleaved(8000)	# allows a memory view
            data = data.tobytes()				# convert the memory view to a readable bytearray
            write_shared_file(shared_file_path, data)

libm2k.contextClose(ctx)
