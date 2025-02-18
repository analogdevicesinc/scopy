import argparse
import configparser

def convert_ini_to_flat(ini_file_path, flat_file_path):
    config = configparser.ConfigParser()
    config.read(ini_file_path)

    flat_lines = []
    for section in config.sections():
        for key, value in config[section].items():
            flat_lines.append(f"{section}\\{key}={value}")

    with open(flat_file_path, "w") as outfile:
        outfile.write("[M2kPlugin]\n")
        outfile.write("\n".join(flat_lines))
    

def convert_flat_to_ini(flat_file_path, ini_file_path):
    config = configparser.ConfigParser()

    with open(flat_file_path, "r") as file:
        for line in file:
            if "=" in line:
                key, value = line.strip().split("=", 1)
                section, sub_key = key.split("\\", 1)

                if section not in config:
                    config[section] = {}

                config[section][sub_key] = value

    with open(ini_file_path, "w") as ini_file:
        config.write(ini_file)
        

def main():
    parser = argparse.ArgumentParser(description="Convert between v1 and v2 Scopy file formats.")
    parser.add_argument("--outmode", choices=["v1", "v2"], required=True, help="Conversion mode: 'v1' (v2 to v1) or 'v2' (v1 to v2).")
    parser.add_argument("--input", required=True, help="Input file path.")
    parser.add_argument("--output", required=True, help="Output file path.")

    args = parser.parse_args()

    if args.outmode == "v1":
        convert_flat_to_ini(args.input, args.output)
    elif args.outmode == "v2":
        convert_ini_to_flat(args.input, args.output)

if __name__ == "__main__":
    main()