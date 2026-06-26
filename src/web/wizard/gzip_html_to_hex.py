##
# @file gzip_html_to_hex.py
# @brief Compresses HTML files with gzip and exports the binary output as a formatted C++ hex array.
# @details The generated C++ array is wrapped in curly braces and saved in a timestamped text file to prevent collisions.
# @author Douglas Quigg (dstroy0, dquigg123@gmail.com)
# @date June 2026
#

import argparse
import gzip
import os
from datetime import datetime

##
# @brief Reads an input file, gzips its contents, formats as a C++ hex byte array, and writes to a text file.
# @param input_path The absolute path of the input file to compress.
# @param output_dir The output directory. If None, uses the same directory as input_path.
# @return The path to the generated output .txt file containing the array.
#
def file_to_hex_array(input_path, output_dir=None):
    if not os.path.exists(input_path):
        print(f"Error: input file {input_path} does not exist.")
        return None
        
    with open(input_path, "rb") as f:
        file_data = f.read()
        
    # Compress using gzip
    compressed_data = gzip.compress(file_data)
    
    # Format as C++ array: {0x1f, 0x8b, 0x08, ...}
    hex_list = [f"0x{b:02x}" for b in compressed_data]
    cpp_array = "{\n  " + ", ".join(hex_list) + "\n}"
    
    # Generate timestamped filename to prevent naming collisions
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    orig_name = os.path.basename(input_path)
    clean_orig_name = orig_name.replace(".", "_")
    output_filename = f"{clean_orig_name}_{timestamp}.txt"
    
    # Determine output path
    if output_dir is None:
        output_dir = os.path.dirname(os.path.abspath(input_path))
    
    output_path = os.path.join(output_dir, output_filename)
    
    # Write output
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(cpp_array)
        
    print(f"Original size  : {len(file_data)} bytes")
    print(f"Compressed size: {len(compressed_data)} bytes")
    print(f"C++ Array generated at: {output_path}")
    return output_path

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Gzip an HTML file and output a C++ hex array.")
    parser.add_argument("--input", required=True, help="Input HTML file path")
    parser.add_argument("--output-dir", help="Output directory path (defaults to same directory as input)")
    
    args = parser.parse_args()
    
    file_to_hex_array(args.input, args.output_dir)
