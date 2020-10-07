from configuration import *


def round_float(value):
    value_int = int(value * 100)
    if value_int == 100:
        return ("1.00")
    else:
        return ("0.%d" % value_int)

def round_float_1(value):
    return ("%.1f" % value)

def expect(condition, error_message):
    if not condition:
        print(error_message)
        exit(1)

def get_gate_library(design, synthesizer):
    if (synthesizer, design) in special_gate_libraries:
        return special_gate_libraries[(synthesizer, design)]
    return default_gate_libraries[synthesizer]

# ---- EXECUTE HAL ----
def execute_hal(synthesizer, design, rebuild_debug, debug, sizes):
    input_design = path_to_core_collection + \
        "/" + netlists[synthesizer][design]

    expect(os.path.isfile(path_to_hal_bin),
           "could not find HAL binary in: " + path_to_hal_bin)


    if not os.path.isfile(input_design):
        print("could not find design: " + input_design)
        return False

    command = "{} -i {} --dataflow --layer 1 --path {} --gate-library {}.lib".format(
        path_to_hal_bin, input_design, path_dataflow_out, get_gate_library(design, synthesizer))

    if sizes != "":
        command = "{} -i {} --dataflow --layer 1 --path {} --gate-library {}.lib --sizes {}".format(
            path_to_hal_bin, input_design, path_dataflow_out, get_gate_library(design, synthesizer), sizes)

    print(command)

    if rebuild_debug or debug:
        os.system("gdb --args " + command)
    else:
        os.system(command)

    return True
