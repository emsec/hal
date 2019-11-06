import os
import sys
import json


def process(file):
    print("processing '{}'".format(file))

    with open(file, "r") as f:
        json_data = json.load(f)

    json_data = json_data["library"]
    name = json_data["library_name"]

    elements = list()
    inputs = dict()
    outputs = dict()

    if "element_types" in json_data:
        for gate_type in json_data["element_types"]:
            elements.append(gate_type)
            inputs[gate_type] = list()
            outputs[gate_type] = list()

        for gate_type in json_data["elements_input_types"]:
            inputs[gate_type] = json_data["elements_input_types"][gate_type]

        for gate_type in json_data["elements_output_types"]:
            outputs[gate_type] = json_data["elements_output_types"][gate_type]

    if "elements" in json_data:
        for gate_type in json_data["elements"]:
            elements.append(gate_type)
            gate_inputs, _, gate_outputs = json_data["elements"][gate_type]
            inputs[gate_type] = gate_inputs
            outputs[gate_type] = gate_outputs

    lines = list()

    lines.append("library ({})".format(name) + " {")
    lines.append("define(cell);")

    for typ in elements:
        lines.append("cell({})".format(typ) + " {")
        if ("FF" in typ and "BUFF" not in typ) or "FD" in typ:
            insert_point = len(lines)
            clock_pin = "null"
            for pin in inputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: input;")
                if pin in ["C", "CK", "CKB", "CLK"]:
                    clock_pin = pin
                    lines.append("clock: true;")
                lines.append("}")
            for pin in outputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: output;")
                if "N" in pin:
                    lines.append("function: \"IQN\";")
                else:
                    lines.append("function: \"IQ\";")
                lines.append("}")
            lines.insert(insert_point, "ff (\"IQ\" , \"IQN\") {")
            lines.insert(insert_point+1, "next_state          : \"D\";")
            lines.insert(insert_point+2,
                         "clocked_on          : \"{}\";".format(clock_pin))
            lines.insert(insert_point+3, "preset              : \"!SN\";")
            lines.insert(insert_point+4, "clear               : \"!RN\";")
            lines.insert(insert_point+5, "clear_preset_var1   : L;")
            lines.insert(insert_point+6, "clear_preset_var2   : L;")
            lines.insert(insert_point+7, "}")

        elif "LUT" in typ:
            lines.append("lut (\"lut_out\") {")
            lines.append("data_category     : \"generic\";")
            lines.append("data_identifier   : \"INIT\";")
            lines.append("bit_order         : \"ascending\";")
            lines.append("}")
            for pin in inputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: input;")
                lines.append("}")
            for pin in outputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: output;")
                lines.append("function: \"lut_out\";")
                lines.append("}")

        else:
            for pin in inputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: input;")
                lines.append("}")
            for pin in outputs[typ]:
                lines.append("pin({})".format(pin) + " {")
                lines.append("direction: output;")
                lines.append("function: \"\";")
                lines.append("}")
        lines.append("}")
        lines.append("")

    lines.append("}")

    # for l in lines:
    #     print(l)

    with open("../definitions/"+file.replace(".json", ".lib"), "wt") as outfile:
        indent = ""
        for l in lines:
            if "}" in l and "{" not in l:
                indent = indent[:-4]
            outfile.write(indent+l+"\n")
            if "{" in l and "}" not in l:
                indent += "    "


for file in sorted(os.listdir(".")):
    if file.endswith(".json"):
        process(file)
