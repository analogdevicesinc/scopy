FILES:

1. sineGenerator.py - Generates samples for the pqm device channels in `data.bin`. The file is created when the script runs and is not included in this folder. This script is needed only with libiio v0.

2. pqmDataWriter.py - Generates random values for the attributes of the channels (rms, angle, harmonics and inter_harmonics). 

3. pqm1.xml - Contains the context structure of the pqm device.

Python dependencies:

- Python 3
- `numpy` for `sineGenerator.py` (libiio v0 only)
- `pylibiio` for `pqmDataWriter.py` (provides the `iio` module)

We recommend using a virtual environment. From this folder, create and activate it, then install the dependencies:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install numpy pylibiio
```

Steps:

1. Open a terminal in this folder.
2. Configure the emulator:
	 - With libiio v0, run `iio-emu generic pqm1.xml iio:device0@data.bin`, or use the Emu tab in Scopy with:
		 - XML Path: `/absolutePathTo/pqm1.xml`
		 - RX/TX devices: `iio:device0@/absolutePathTo/data.bin`
	 - With libiio v1, the emulator loops over buffer data from `.bin` files. Name each file `<dev_name/id>_buf<buf_index>.bin`; `iiod-emu` discovers these files automatically, so no RX/TX path is needed.
3. With libiio v0 only, run `./sineGenerator.py [output-file]` (defaults to `data.bin`).
4. With the virtual environment activated, run `./pqmDataWriter.py`.
5. Connect to the device through Scopy.

