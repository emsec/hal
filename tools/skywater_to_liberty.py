#!/usr/bin/python3
import os
import sys
import json
from pathlib import Path


"""
---------------------------------------------------------------------------------------------------
SkyWater to Liberty Converter:
Converts the content of a SkyWater gate library to a single liberty file that only contains the 
information that are used by the HAL liberty parser.
---------------------------------------------------------------------------------------------------
"""

library_name = ""


def indent_block(s, amt=1):
    """
    Intend the lines of a string.

    :param s: The string
    :param amt: The amount, how deep the string block should be intended (default 1)
    :return: The indented string
    """
    indent_string = "  "
    res = s
    for i in range(0, amt):
        res = indent_string + indent_string.join(res.splitlines(True))
    return res


def find_boolean_function(lib_json_data, port_name):
    """
    Search for a boolean function for a certain port in a .lib.json file.

    :param lib_json_data: json data of a .lib.json file
    :param port_name: The port name
    :returns: The boolean function as a string. Empty string if no boolean function was found.
    """
    if lib_json_data is None:
        return ""
    if ("pin,"+port_name) in lib_json_data:
        if "function" in lib_json_data[("pin,"+port_name)]:
            return lib_json_data[("pin,"+port_name)]["function"]

    return ""


def convert_ff_block(lib_json_data):
    """
    Generate a string in liberty format that represents a flip-flop (ff) block of a certain cell,
    contained in a .lib.json file.

    :param lib_json_data: json data of a .lib.json file
    :returns: The ff block string. Empty string if no ff information were found.
    """
    if lib_json_data is None:
        return ""
    if not ("ff,IQ,IQ_N" in lib_json_data):
        return ""
    ff_data = lib_json_data["ff,IQ,IQ_N"]
    ff_inner = ""
    for k in ["next_state", "clocked_on", "preset", "clear"]:
        if k in ff_data:
            ff_inner += "{}: \"{}\";\n".format(k, ff_data[k])

    for k in ["clear_preset_var1", "clear_preset_var2"]:
        if k in ff_data:
            ff_inner += "{}: {};\n".format(k, ff_data[k])

    return "ff (\"IQ\", \"IQ_N\") {{\n{}}}\n".format(indent_block(ff_inner))


def convert_latch_block(lib_json_data):
    """
    Generate a string in liberty format that represents a latch block of a certain cell,
    contained in a .lib.json file.

    :param lib_json_data: json data of a .lib.json file
    :returns: The latch block string. Empty string if no latch information were found.
    """
    if lib_json_data is None:
        return ""
    if not ("latch,IQ,IQ_N" in lib_json_data):
        return ""
    latch_data = lib_json_data["latch,IQ,IQ_N"]
    latch_inner = ""
    for k in ["clear", "data_in", "preset", "enable"]:
        if k in latch_data:
            latch_inner += "{}: \"{}\";\n".format(k, latch_data[k])

    for k in ["clear_preset_var1", "clear_preset_var2"]:
        if k in latch_data:
            latch_inner += "{}: {};\n".format(k, latch_data[k])

    return "latch (\"IQ\", \"IQ_N\") {{\n{}}}\n".format(indent_block(latch_inner))


def convert_gate_type_to_liberty(path):
    """
    Generate a string that represents a cell type in liberty format. Take the information from the description.json
    and any .lib.json file in the cell folder.

    :param path: The path to the cell folder.
    :return: The string
    """
    global library_name
    gate_type_string = ""
    # Open any .lib.json file
    lib_json_data = None
    for file in os.scandir(path):
        if file.path.endswith(".lib.json"):
            with open(file) as lib_json_file:
                lib_json_data = json.load(lib_json_file)
                lib_json_file.close()
            break
    if not (os.path.isfile(path + "/definition.json")):
        return ""

    with open(path + "/definition.json") as def_json_file:
        # Parse a gate_type
        def_json_data = json.load(def_json_file)
        def_json_file.close()
        gate_type_string_inner = ""
        # Get the library name if not already available
        if library_name == "":
            library_name = def_json_data["library"]

        # Parse the flip-flop header (if given)
        gate_type_string_inner += convert_ff_block(lib_json_data)

        # Parse the latch header (if given)
        gate_type_string_inner += convert_latch_block(lib_json_data)

        for port_data in def_json_data["ports"]:
            # Parse a port
            port_type = port_data[0]
            if port_type != "signal":
                continue
            port_name = port_data[1]
            port_dir = port_data[2]
            port_str_inner = ""
            if port_dir == "input":
                port_str_inner += "direction: input;\n"
            else:
                port_str_inner += "direction: output;\n"
                bf = find_boolean_function(lib_json_data, port_name)
                # find the boolean function if available
                if bf != "":
                    port_str_inner += "function: \"{}\";\n".format(bf)
            gate_type_string_inner += "pin({}) {{\n{}}}\n".format(port_name, indent_block(port_str_inner))

        gate_type_string = "cell({}) {{\n{}}}\n".format(def_json_data["name"], indent_block(gate_type_string_inner))

    return gate_type_string


def convert_folder_to_liberty(src_folder_path, dst_folder_path, version_suffix):
    """
    Converts a cell folder to a single minimal liberty file (containing all information HAL uses).
    Returns the output path, if at least one valid cell was parsed.

    :param src_folder_path: The path to the 'cells'-directory of the library
    :param dst_folder_path: The path to the directory where the output file should be generated
    :param version_suffix: A suffix that descibes the version of the library (for example: "(latest)")
    :returns: The path where the output file was generated by this function. Empty string if no cells were found.
    """
    g_lib_content = ""

    cell_folders = sorted([f.path for f in os.scandir(src_folder_path) if f.is_dir()])

    for e in cell_folders:
        g_lib_content += convert_gate_type_to_liberty(e)

    if g_lib_content == "":
        return ""

    file_content = "library ({}) {{\n{}}}".format(library_name, indent_block(g_lib_content))

    dst_file_name = library_name+version_suffix+".lib"
    out_path = os.path.join(dst_folder_path, dst_file_name)
    dst_file = open(out_path, "w")
    dst_file.write(file_content)
    dst_file.close()

    return str(out_path)


def print_usage(prog_name):
    """
    Print the programs usage description in the console

    :param prog_name: The name of the program file
    :return: nothing
    """
    print("\n=== SkyWater-to-Liberty Converter ===\n\n"
          "Converts the content of a SkyWater gate library to a single liberty file "
          "that only contains the information that are used by the HAL liberty parser.\n")
    print("Usage: {} <source_directory_path> [<destination_directory_path>]\n".format(prog_name))

    print("<source_directory_path>: path to the SkyWaters 'cells' directory that contains folders for all cells.")
    print("<destination_directory_path>: directory path for the generated output file.\n")


def parse_cli_args(p_arg_0, p_arg_1, p_arg_2):
    """
    Parse the given cli arguments and get the necessary information from it. Also checks their validity.

    :param p_arg_0: The 0th cli argument (the program name)
    :param p_arg_1: The 1st cli argument (should be the path of the source directory)
    :param p_arg_2: The 2nd cli argument (should be the path of the destination directory (if given))
    :returns: A tuple (source_directory, dst_directory, version). If input is invalid returns (None, None, None).
    """
    src_folder_path_res = ""
    dst_folder_path_res = ""
    version_suffix_res = ""
    # Src Folder Arg
    if p_arg_1 is None:
        print_usage(p_arg_0)
        return None, None, None
    if p_arg_1.lower() in ["-h", "-help", "help"]:
        print_usage(p_arg_0)
        return None, None, None

    if not os.path.isdir(p_arg_1):
        print("Error: Source directory '{}' does not exist!".format(os.path.abspath(p_arg_1)))
        return None, None, None
    else:
        src_folder_path_res = p_arg_1

        # If not already in the cell directory go into it
        if os.path.basename(src_folder_path_res) != "cells":
            if os.path.isdir(os.path.join(src_folder_path_res, "cells/")):
                src_folder_path_res = os.path.join(src_folder_path_res, "cells/")
        # Get the version of the library
        version_dir_name = os.path.basename(str(Path(src_folder_path_res).parent))
        if version_dir_name.startswith('v') or version_dir_name == "latest":
            version_suffix_res = "({})".format(version_dir_name.replace(".", "_"))

    # Dst Folder Arg
    if p_arg_2 is None:
        dst_folder_path_res = "./"
    else:
        if not os.path.isdir(p_arg_2):
            print("Error: Destination directory '{}' does not exist!".format(os.path.abspath(p_arg_2)))
            return None, None, None
        else:
            dst_folder_path_res = p_arg_2

    return src_folder_path_res, dst_folder_path_res, version_suffix_res


""" === ENTRY POINT === """
if __name__ == "__main__":
    # Read in the command line arguments (cli args)
    arg_0 = None
    arg_1 = None
    arg_2 = None
    for arg_i in range(0, len(sys.argv)):
        if arg_i == 0:
            arg_0 = sys.argv[0]
        if arg_i == 1:
            arg_1 = sys.argv[1]
        if arg_i == 2:
            arg_2 = sys.argv[2]

    # cli args are parsed and checked
    src_folder_path, dst_folder_path, version_suffix = parse_cli_args(arg_0, arg_1, arg_2)

    if not (src_folder_path is None):
        # Generate the liberty file
        o_path = convert_folder_to_liberty(src_folder_path, dst_folder_path, version_suffix)
        if o_path != "":
            print("Liberty file successfully created: " + os.path.abspath(o_path))
        else:
            print("No cells were found! Have you selected the correct directory?")

