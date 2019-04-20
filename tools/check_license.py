#!/usr/bin/env python3
import sys
import os

# color codes for nice output formatting
class colors:
    RED = '\033[91m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# analyze a single file
def analyze_file(file, license_lines):
    print("===== " + file)
    error = False

    lines = list()
    with open(file, "rt") as f:
        lines = [l.strip() for l in f.readlines()]

    if len(license_lines) > len(lines):
        print(colors.RED + "    the file does not contain the license."+ colors.ENDC)
        error = True

    if not error:
        wrong_lines = list()
        for i, l in enumerate(license_lines):
            if l != lines[i]:
                wrong_lines.append(i)
                error = True

        if len(wrong_lines) == len(license_lines):
            print(colors.RED + "    the file does not contain the license."+ colors.ENDC)
        else:
            for line in wrong_lines:
                print(colors.RED + "    line "+str(line)+" does not match the license."+ colors.ENDC)

    return not error

# recursively check all files in a directory except the ones that start with an element of the ignore list
def check_recursive(thing, ignore_list, license_lines):
    if any(thing.startswith(x) for x in ignore_list):
        return True
    if os.path.isfile(thing) and any(thing.endswith(x) for x in [".h", ".hpp", ".h.in"]):
        return analyze_file(thing, license_lines)
    else:
        if not os.path.isdir(thing):
            return True # Current file is not a directory. Skip it
        if (thing[-1] != "/"):
            thing += "/"
        result = True
        for file in os.listdir(thing):
            result = check_recursive(thing + file, ignore_list, license_lines) and result
        return result

##################################################################
##################################################################
##################################################################

def print_usage():
    print("License Checker")
    print("  (custom script, may contain errors)")
    print("")
    print("  Usage: python check_license.py <headerfile> <inputpath1> [<inputpath2> ...] [-i/-ignore <ignorepath1> [<ignorepath2>]]")
    print("")
    print("  You can always provide directories or single files.")
    print("  All files/directories will be ignored which start with any of the ignorepaths.")
    print("")

if __name__ == '__main__':
    # parameter check
    if len(sys.argv) < 3:
        print_usage()
        sys.exit(1)

    license_lines = list()
    with open(sys.argv[1], "rt") as file:
        license_lines = [l.strip() for l in file.readlines()]

    print("checking for the following license:")
    for line in license_lines:
        print(line)
    print("")

    # parse the command line arguments
    ignore_list = []
    directory_list = []
    do_ignore = False
    for i in range(2, len(sys.argv)):
        if sys.argv[i] == "-i" or sys.argv[i] == "-ignore":
            do_ignore = True
            continue
        if do_ignore:
            ignore_list.append(sys.argv[i])
        else:
            directory_list.append(sys.argv[i])

    # process the inputs
    all_ok = True
    for x in directory_list:
        all_ok = check_recursive(x, ignore_list, license_lines) and all_ok

    # print the results and exit
    print("=====")
    if all_ok:
        print("License is always correct.")
        sys.exit(0)
    else:
        print("License errors were found.")
        sys.exit(1)
