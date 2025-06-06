import argparse
import mmap
import struct
import os

def read_interleaved_doubles(filename):
    file_size = os.path.getsize(filename)
    total_values = file_size // 8  # 8 bytes per double

    data = []

    with open(filename, "rb") as f:
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)

        for i in range(total_values):
            offset = i * 8
            value = struct.unpack_from('d', mm, offset)[0]
            data.append(value)

        mm.close()

    return data

def process_data(data):
    # Example processing: add 2 to each value
    return [x + 2 for x in data] 

def write_interleaved_doubles(filename, data):
    with open(filename, "wb") as f:
        for value in data:
            f.write(struct.pack('d', value))

def main():
    parser = argparse.ArgumentParser(description="Read, process, and write interleaved binary data.")
    parser.add_argument('--input', required=True, help="Path to input binary file")
    parser.add_argument('--output', required=True, help="Path to output binary file")

    args = parser.parse_args()

    # Step 1: Read the binary file
    data = read_interleaved_doubles(args.input)
    print(f"Read {len(data)} samples from {args.input}")

    # Step 2: Process the data
    processed = process_data(data)

    # Step 3: Write processed data to output file
    write_interleaved_doubles(args.output, processed)
    print(f"Processed data saved to: {args.output}")

if __name__ == "__main__":
    main()
