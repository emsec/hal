#!/usr/bin/python3

import os
import argparse
import getpass
import sys

from configuration import *
from utils import *



parser = argparse.ArgumentParser(description='Call the HAL dataflow plugin.')
parser.add_argument('design', metavar='design', type=str, help='name of the design')
parser.add_argument('synthesizer', metavar='synthesizer', type=str, help='name of the synthesizer')
parser.add_argument('--rebuild', action='store_true', help='if set will clear build folder and rebuild cmake')
parser.add_argument('--rebuild-debug', action='store_true', help='if set will clear build folder and rebuild cmake with debug and execute with gdb')
parser.add_argument('--debug', action='store_true', help='if set will execute with gdb')
parser.add_argument('--sizes', metavar='sizes', type=str, help='define allowed sizes')
parser.add_argument('--hal-file', action='store_true', help='use the .hal file instead of the netlist')
parser.add_argument('--create-hal-file', action='store_true', help='run hal without the dataflow plugin to create a .hal file for the netlist')

##################################################################
##################################################################
##################################################################
# ---- ARG CHECKS ----

def expect(condition, error_message):
    if not condition:
        print(error_message)
        exit(1)

args = parser.parse_args()

synthesizers = set(x for x in netlists)
designs = set(x for y in netlists for x in netlists[y])

expect(os.path.isdir(path_to_core_collection), "core collection needs to be placed in: " + path_to_core_collection)
expect(os.path.isdir(path_to_core_collection), "hal needs to be placed in: " + path_to_core_collection)
expect(args.design in designs, "available designs: " +", ".join(designs))
expect(args.synthesizer in synthesizers, "available synthesizers: " +", ".join(synthesizers))

##################################################################
##################################################################
##################################################################
# ---- BUILD HAL ----

if args.rebuild or args.rebuild_debug:
    print("will recreate cmake")
    #delete everything in build folder
    os.system('rm -rf ' + path_to_hal_build + "/*")
    os.chdir(path_to_hal_build)
    if args.rebuild:
        os.system("cmake ../ -GNinja -DPL_DATAFLOW=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_GUI=OFF")
    else:
        os.system("cmake ../ -GNinja -DPL_DATAFLOW=ON -DCMAKE_BUILD_TYPE=Debug -DWITH_GUI=OFF")

expect(os.path.isdir(path_to_hal_build), "path to hal build '{}' does not exist".format(path_to_hal_build))

os.chdir(path_to_hal_build)

return_ninja = os.system('ninja')
expect(return_ninja == 0, "error in build: return code: " + str(return_ninja))

##################################################################
##################################################################
##################################################################
# ---- EXECUTE PLUGIN ----

expect(os.path.isfile(path_to_hal_bin), "could not find HAL binary in: " + path_to_hal_bin)

input_design = path_to_core_collection + "/" + netlists[args.synthesizer][args.design]


if args.create_hal_file:
    command = "{} -i {} --gate-library {}.lib".format(path_to_hal_bin, input_design, get_gate_library(args.design, args.synthesizer))
    os.system(command)
else:
    if args.hal_file:
        input_design = input_design[:input_design.rfind(".")] + ".hal"

    expect(os.path.isfile(input_design), "could not find design: " + input_design)

    command = "{} -i {} --dataflow --path {} --gate-library {}.lib".format(path_to_hal_bin, input_design, path_dataflow_out, get_gate_library(args.design, args.synthesizer))

    if args.sizes != None:
        command += " --sizes {}".format(args.sizes)

    print(command)

    if args.rebuild_debug or args.debug:
        os.system("gdb --args " + command)
    else:
        os.system(command)
