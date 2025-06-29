def txt_to_bin(input_txt_path, output_bin_path):
    with open(input_txt_path, 'r') as txt_file, open(output_bin_path, 'wb') as bin_file:
        for line in txt_file:
            # Skip the offset and colon (first 5 characters: "000: ")
            hex_bytes = line[5:].strip().split()
            # Convert hex to bytes and write to binary file
            bin_file.write(bytes(int(byte, 16) for byte in hex_bytes))

# Example usage
txt_to_bin('input.txt', 'output.bin')