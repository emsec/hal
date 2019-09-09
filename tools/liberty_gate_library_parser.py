#!/usr/bin/env python3

import sys
import json
import re
import os


# enable output for simulator
simulator = True

groups_of_interest = ["library", "cell", "pin", "ff", "latch"]
attributes_of_interest = ["direction", "function", "three_state", "x_function", "next_state", "clocked_on", "clear", "preset", "clear_preset_var1", "clear_preset_var2", "data_in", "enable"]


################################ Print functions

def print_usage():
    print("HAL liberty gate library parser")
    print("  usage: liberty_parser [liberty_file]")
    print("")
    print("Extracts the HAL .json gate library as well as the corresponding BDDs from a liberty file.")


def print_critical(text):
    print("[Critical] " + text)
    sys.exit()


################################ Liberty file tree classes

class Statement:
    def __init__(self, parent, is_group):
        self.parent = parent
        self.is_group = is_group


class GroupStatement(Statement):
    def __init__(self, parent, group_name, name):
        super().__init__(parent, True)
        self.group_name = group_name
        self.name = name
        self.statements = list() 

    def __str__(self):
        string = self.group_name + "(" + self.name + ") {\n"
        for s in self.statements:
            string += str(s)

        string += "}\n"

        return string

    def add_statement(self, statement):
        self.statements.append(statement)


class AttributeStatement(Statement):
    def __init__(self, parent, attribute_name, attribute_value):
        super().__init__(parent, False)
        self.attribute_name = attribute_name
        self.attribute_value = attribute_value

    def __str__(self):
        return self.attribute_name + " : " + self.attribute_value + ";\n"


################################ Library object classes

class Library:
    def __init__(self, name):
        self.name = name
        self.cells = list()

    def __str__(self):
        return self.name

    def __repr__(self):
        return "### LIBRARY - " + self.name + ":\n" + "".join([repr(cell) for cell in self.cells])

    def add_cell(self, cell):
        self.cells.append(cell)


class Cell:
    def __init__(self, name):
        self.name = name
        self.pins = list()

        # sequential logic (dealt with by simulator)
        self.is_ff = False
        self.is_latch = False
        self.states = None
        self.next_state = None
        self.clocked_on = None
        self.data_in = None
        self.enable = None
        self.clear = None
        self.preset = None
        self.clear_preset_var1 = None
        self.clear_preset_var2 = None

    def __str__(self):
        return self.name

    def __repr__(self):
        string  = " * " + self.name + ":\n"
        string += "   - IN:    " + ", ".join([repr(pin) for pin in self.pins if pin.direction == "input"]) + "\n"
        string += "   - INOUT: " + ", ".join([repr(pin) for pin in self.pins if pin.direction == "inout"]) + "\n"
        string += "   - OUT:   " + ", ".join([repr(pin) for pin in self.pins if pin.direction == "output"]) + "\n"
        if self.states is not None:
            ff_list = ["states(" + ", ".join(self.states) + ")", "clocked_on(" + str(self.clocked_on) + ")", "next_state(" + str(self.next_state) + ")"]
            if self.clear is not None:
                ff_list.append("clear(" + str(self.clear) + ")")
            if self.preset is not None:
                ff_list.append("preset(" + str(self.preset) + ")")
            if self.clear_preset_var1 is not None:
                ff_list.append("clear_preset_var1(" + self.clear_preset_var1 + ")")
            if self.clear_preset_var2 is not None:
                ff_list.append("clear_preset_var2(" + self.clear_preset_var2 + ")")
            string += "   - FF:    " + ", ".join(ff_list) + "\n"
        return string

    def __lt__(self, other):
        return self.name < other.name

    def add_pin(self, pin):
        self.pins.append(pin)

    def get_input_pins(self):
        return [pin for pin in self.pins if pin.direction == "input"]

    def get_inout_pins(self):
        return [pin for pin in self.pins if pin.direction == "inout"]

    def get_output_pins(self):
        return [pin for pin in self.pins if pin.direction == "output"]


class Pin:
    def __init__(self, name):
        self.name = name
        self.direction = None
        self.equation = None
        self.three_state = None
        self.x_function = None
    
    def __str__(self):
        return self.name

    def __repr__(self):
        string = self.name
        if self.equation is not None:
            string += "(" + str(self.equation) + ")"
        if self.three_state is not None:
            string += "[" + str(self.three_state) + "]"
        return string


class Equation:
    def __init__(self, equation):
        self.equation = equation
        self.__unify_equation()

    def __str__(self):
        return self.equation

    def __eq__(self, other):
        return self.equation == other.equation

    def __unify_equation(self):
        # deal with different versions of "and" and "or"
        p = re.compile(r"\s*[\*\&]\s*")
        self.equation = p.sub(r" & ", self.equation)
        p = re.compile(r"\s*[\+\|]\s*")
        self.equation = p.sub(r" | ", self.equation)
        p = re.compile(r"(?<=[\w+\(\)\'])\s+(?=[\w+\!\(\)])")
        self.equation = p.sub(r" & ", self.equation)

        # insert spaces around "^"
        p = re.compile(r"\s*\^\s*")
        self.equation = p.sub(r" ^ ", self.equation)

        # deal with negation by "'"
        while ")'" in self.equation:
            bracket_level = 0
            open_pos = 0
            close_pos = self.equation.rfind(")'")

            for i in reversed(range(close_pos)):
                if self.equation[i] is ")":
                    bracket_level += 1
                if self.equation[i] is "(":
                    if bracket_level == 0:
                        open_pos = i
                        break
                    bracket_level -= 1

            self.equation = self.equation[:open_pos] + "!" + self.equation[open_pos:close_pos+1] + self.equation[close_pos+2:]

        p = re.compile(r"(?P<var>\w+)'")
        self.equation = p.sub(r"!\1", self.equation)

    def bdd_equation(self):
        bdd_equation = self.equation

        # replace "0"
        if bdd_equation == "0":
            return "bdd_false()"

        # replace "1"
        if bdd_equation == "1":
            return "bdd_true()"

        # replace input pins
        p = re.compile(r"(?P<pin>\w+)")
        bdd_equation = p.sub(r'*(input_pin_type_to_bdd["\1"])', bdd_equation)

        # deal with negation by "!"
        p = re.compile(r"!(?P<var>\*\(input_pin_type_to_bdd\[\"\w+\"\]\))")
        bdd_equation = p.sub(r"bdd_not(\1)", bdd_equation)

        p = re.compile(r"!\(")
        bdd_equation = p.sub(r"bdd_not(", bdd_equation)
            
        return bdd_equation


################################ Library extraction functions

def read_liberty(file_name):
    try:
        with open(file_name) as f:
            lines = f.readlines()
            lines = remove_comments(lines)
            return lines
            
    except IOError:
        print_critical("Could not read file: " + file_name + ".")


def remove_comments(lines):
    comment_depth = 0       # should be 0 or 1
    clean_lines = list()

    # Regex patterns for multi-line comments
    p_full = re.compile(r"/\*.*\*/")
    p_start = re.compile(r"/\*.*")
    p_end = re.compile(r".*\*/")

    for line in lines:
        comment_depth -= line.count("*/")

        if comment_depth > 0:
            comment_depth += line.count("/*")
            continue
        
        comment_depth += line.count("/*")

        line = p_full.sub("", line)
        line = p_start.sub("", line)
        line = p_end.sub("", line)

        line = line.strip()
        if line != "":
            clean_lines.append(line)

    if comment_depth > 0:
        print_critical("Invalid comment syntax.")

    return clean_lines


def extract_library(lines):
    comment_depth = 0           # should be 0 or 1
    ignore_group_depth = 0      # counts brackets

    # Regex patterns for multi-line comments
    p_full = re.compile(r"/\*.*\*/")
    p_start = re.compile(r"/\*.*")
    p_end = re.compile(r".*\*/")

    start = None
    current = None

    for line in lines:
        ignore_brackets = 0

        # comment removal
        comment_depth -= line.count("*/")

        if comment_depth > 0:   # multi-line comment
            comment_depth += line.count("/*")
            continue
        
        comment_depth += line.count("/*")

        line = p_full.sub("", line)
        line = p_start.sub("", line)
        line = p_end.sub("", line)

        line = line.strip()
        if line == "":
            continue
        
        # currently in ignored group?
        if ignore_group_depth != 0:
            ignore_brackets = ignore_group_depth

            ignore_group_depth += line.count("{") - line.count("}")

            if ignore_group_depth <= 0:
                ignore_group_depth = 0

                for i in range(line.count("}") - ignore_brackets):
                    current = current.parent

            continue

        # extract group statement from current line
        group = extract_group(line)
        if group is not None:
            group_name, name = group
            if group_name not in groups_of_interest:
                ignore_group_depth += line.count("{")
                continue

            gs = GroupStatement(current, group_name, name)

            if group_name != "library":
                current.add_statement(gs)
            else:
                start = gs

            current = gs

        # extract attribute statement from current line
        attribute = extract_attribute(line)
        if attribute is not None:
            attribute_name, attribute_value = attribute
            if attribute_name not in attributes_of_interest:
                continue

            current.add_statement(AttributeStatement(current, attribute_name, attribute_value))

        # leave current group(s)
        for i in range(line.count("}") - ignore_brackets):
            current = current.parent

    if comment_depth > 0:
        print_critical("Invalid comment syntax.")

    # start should be library group statement
    if start.is_group != True or start.group_name != "library":
        print_critical("Invalid library format.")

    library = Library(start.name)

    for s0 in start.statements:
        if s0.is_group == True:
            # look for cell group statements within library
            if s0.group_name == "cell":
                cell = Cell(s0.name)
                library.add_cell(cell)
                for s1 in s0.statements:
                    if s1.is_group == True:
                        # look for pins within cell
                        if s1.group_name == "pin":
                            pin = Pin(s1.name)
                            cell.add_pin(pin)
                            for s2 in s1.statements:
                                if s2.is_group == False:
                                    # extract direction, function, and three-state logic
                                    if s2.attribute_name == "direction":
                                        pin.direction = s2.attribute_value
                                    elif s2.attribute_name == "function":
                                        pin.equation = Equation(s2.attribute_value)
                                    elif s2.attribute_name == "three_state":
                                        pin.three_state = Equation(s2.attribute_value)
                                    elif s2.attribute_name == "x_function":
                                        pin.three_state = Equation(s2.attribute_value)

                        # is current cell a flip-flop?
                        elif s1.group_name == "ff":
                            cell.states = extract_ff_states(s1.name)
                            cell.is_ff = True
                            for s2 in s1.statements:
                                if s2.is_group == False:
                                    # extract clocked_on, next_state, clear, preset, and clear_preset_var
                                    if s2.attribute_name == "clocked_on":
                                        cell.clocked_on = Equation(s2.attribute_value)
                                    if s2.attribute_name == "next_state":
                                        cell.next_state = Equation(s2.attribute_value)
                                    if s2.attribute_name == "clear":
                                        cell.clear = Equation(s2.attribute_value)
                                    if s2.attribute_name == "preset":
                                        cell.preset = Equation(s2.attribute_value)
                                    if s2.attribute_name == "clear_preset_var1":
                                        cell.clear_preset_var1 = s2.attribute_value
                                    if s2.attribute_name == "clear_preset_var2":
                                        cell.clear_preset_var2 = s2.attribute_value

                        # is current cell a latch?
                        elif s1.group_name == "latch":
                            cell.states = extract_ff_states(s1.name)
                            cell.is_latch = True
                            for s2 in s1.statements:
                                if s2.is_group == False:
                                    # extract enable, data_in, clear, preset, and clear_preset_var
                                    if s2.attribute_name == "enable":
                                        cell.enable = Equation(s2.attribute_value)
                                    if s2.attribute_name == "data_in":
                                        cell.data_in = Equation(s2.attribute_value)
                                    if s2.attribute_name == "clear":
                                        cell.clear = Equation(s2.attribute_value)
                                    if s2.attribute_name == "preset":
                                        cell.preset = Equation(s2.attribute_value)
                                    if s2.attribute_name == "clear_preset_var1":
                                        cell.clear_preset_var1 = s2.attribute_value
                                    if s2.attribute_name == "clear_preset_var2":
                                        cell.clear_preset_var2 = s2.attribute_value


    return library


def extract_group(line):
    p = re.compile(r"\s*(?P<group_name>\w+)\s*\((?P<name>.*)\)\s*\{")
    res = p.search(line)

    if res:
        return res.group("group_name"), res.group("name")

    return None


def extract_attribute(line):
    p = re.compile(r"\s*(?P<attribute_name>\w+)\s*:\s*\"?(?P<attribute_value>[^\"]*[^\s\"])\"?\s*;")
    res = p.search(line)
    
    if res is not None:
        return res.group("attribute_name"), res.group("attribute_value")

    return None


def extract_complex_attribute(line):
    p = re.compile(r"\s*(?P<attribute_name>\w+)\s*:\s*\((?P<attribute_value>[^\"]*[^\s\"])\)\s*;")
    res = p.search(line)
    
    if res is not None:
        return res.group("attribute_name"), res.group("attribute_value")

    return None


def extract_ff_states(string):
    p = re.compile(r"\"(?P<s1>\w+)\"\s*,\s*\"(?P<s2>\w+)\"")
    res = p.search(string)

    if res is not None:
        return res.group("s1"), res.group("s2")

    p = re.compile(r"(?P<s1>\w+)\s*,\s*(?P<s2>\w+)")
    res = p.search(string)

    if res is not None:
        return res.group("s1"), res.group("s2")

    return None


################################ Library writer functions

# writes JSON library
def write_definition(library):
    file_name = library.name + "/" + library.name + ".json"

    element_types = sorted(library.cells)

    json_out = {"library" : {"element_types" : [cell.name for cell in element_types], 
                             "elements_input_types" : {cell.name : [pin.name for pin in cell.get_input_pins()] for cell in element_types}, 
                             "elements_inout_types" : {cell.name : [pin.name for pin in cell.get_inout_pins()] for cell in element_types}, 
                             "elements_output_types" : {cell.name : [pin.name for pin in cell.get_output_pins()] for cell in element_types},
                             "library_name" : library.name,
                             "vhdl_includes" : list(),
                             "global_gnd_nodes" : [cell.name for cell in element_types for pin in cell.pins if pin.equation is not None and pin.equation == Equation("0")],
                             "global_vcc_nodes" : [cell.name for cell in element_types for pin in cell.pins if pin.equation is not None and pin.equation == Equation("1")]}}  

    try:
        with open(file_name, "w") as f:
            # write to file
            json.dump(json_out, f, indent=2, sort_keys=True)

    except IOError:
        print_critical("Could not write file: " + file_name + ".")


# writes C++ file containing library BDDs
def write_gate_decorator_bdd(library):
    file_name = library.name + "/gate_decorator_bdd_" + library.name + ".cpp"

    try:
        with open(file_name, "w") as f:
            code = list()

            # includes
            code.append("#include \"plugin_gate_decorators.h\"\n"
                        "#include \"gate_decorator_system/gate_decorator_system.h\"\n"
                        "#include \"gate_decorator_system/decorators/gate_decorator_bdd.h\"\n"
                        "\n"
                        "#include \"core/log.h\"\n"
                        "#include \"netlist/gate.h\"\n"
                        "\n"
                        "namespace bdd_" + library.name + "_helper\n"
                        "{\n")

            # BDD functions for cells with output pins
            for cell in sorted(library.cells):
                # select only output pins with valid (i.e. non-DFF) equations
                valid_pins = [pin for pin in cell.pins if pin.direction in ["output", "inout"] and pin.equation is not None and pin.equation not in [Equation("IQ"), Equation("IQN")]]

                # build output for each valid output pin
                if valid_pins:
                    code.append("    std::map<std::string, std::shared_ptr<bdd>> get_bdd_" + library.name + "_" + cell.name + "(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)\n"
                                "    {\n"
                                "        UNUSED(gate);\n"
                                "        std::map<std::string, std::shared_ptr<bdd>> result;\n")

                    pin_num = 0
                    for pin in valid_pins:
                        code.append("\n"
                                    "        // function : " + str(pin) + " = " + str(pin.equation) + "\n"
                                    "        auto bdd_output_" + str(pin_num) + " = std::make_shared<bdd>();\n"
                                    "        *bdd_output_" + str(pin_num) + " = " + pin.equation.bdd_equation() + ";\n"
                                    "        result[\"" + str(pin) + "\"] = bdd_output_" + str(pin_num) + ";\n")

                        pin_num += 1

                    code.append("        return result;\n"
                                "    }\n")
                    code.append("\n")

            # end of BDD functions
            code.pop()
            code.append("}    // namespace bdd_" + library.name + "_helper\n"
                        "\n"
                        "static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {\n")

            # generate cell to BDD function map
            for cell in sorted(library.cells):
                # select only output pins with valid (i.e. non-DFF) equations
                valid_pins = [pin for pin in cell.pins if pin.direction in ["output", "inout"] and pin.equation is not None and pin.equation not in [Equation("IQ"), Equation("IQN")]]

                if valid_pins:
                    code.append("    {\"" + cell.name + "\", bdd_" + library.name + "_helper::get_bdd_" + library.name + "_" + cell.name + "},\n")

            code[len(code)-1] = code[len(code)-1][:-2] + "\n"

            # generate BDD generator and availability tester functions
            code.append("};\n"
                        "\n"
                        "std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_" + library.name + "(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)\n"
                        "{\n"
                        "    auto type = g->get_type();\n"
                        "\n"
                        "    if (m_bbd_generators.find(type) == m_bbd_generators.end())\n"
                        "    {\n"
                        "        log_error(\"netlist.decorator\", \"not implemented reached for gate type '{}'.\", type);\n"
                        "        return std::map<std::string, std::shared_ptr<bdd>>();\n"
                        "    }\n"
                        "\n"
                        "    gate_decorator_system::bdd_decorator_generator generator = m_bbd_generators.at(type);\n"
                        "    return generator(g, input_pin_type_to_bdd);\n"
                        "}\n"
                        "\n"
                        "bool plugin_gate_decorators::bdd_availability_tester_" + library.name + "(std::shared_ptr<gate> g)\n"
                        "{\n"
                        "    auto type = g->get_type();\n"
                        "\n"
                        "    if (m_bbd_generators.find(type) == m_bbd_generators.end())\n"
                        "    {\n"
                        "        return false;\n"
                        "    }\n"
                        "    return true;\n"
                        "}\n")

            # write to file
            f.writelines(code)

    except IOError:
        print_critical("Could not write file: " + file_name + ".")


# writes a README file containing additional information
def write_readme(library):
    file_name = library.name + "/README.txt"

    try:
        with open(file_name, "w") as f:
            text = list()
            text.append("### README: " + library.name + "\n"
                        "##########################################################################\n"
                        "\n"
                        "1. Place '" + library.name + ".json' in 'plugins/gate_libraries/definitions'.\n"
                        "\n"
                        "\n"
                        "2. Place 'gate_decorator_bdd_" + library.name + ".cpp' in 'plugins/gate_libraries/src'.\n"
                        "\n"
                        "\n"
                        "3. Add the following code to the method 'plugin_gate_decorators::on_load()' within 'plugins/gate_libraries/src/plugin_gate_decorators.cpp':\n"
                        "\n"
                        "        gate_decorator_system::register_bdd_decorator_function(\"" + library.name + "\", &bdd_availability_tester_" + library.name + ", &bdd_generator_" + library.name + ");\n"
                        "\n"
                        "\n"
                        "4. Add the following code to the method 'plugin_gate_decorators::on_unload()' within 'plugins/gate_libraries/src/plugin_gate_decorators.cpp':\n"
                        "\n"
                        "        gate_decorator_system::remove_bdd_decorator_function(\"" + library.name + "\");\n"
                        "\n"
                        "\n"
                        "5. Add the following code to the class 'plugin_gate_decorators' within 'plugins/gate_libraries/include/plugin_gate_decorators.h':\n"
                        "\n"
                        "        static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_" + library.name + "(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);\n"
                        "        static bool bdd_availability_tester_" + library.name + "(std::shared_ptr<gate> g);\n"
                        "\n"
                        "\n")

            if simulator == True:
                text.append("6. Place 'simulator_" + library.name + ".h' in 'plugins/hal-plugins/simulator/include/simulators'.\n"
                            "\n"
                            "\n"
                            "7. Place 'simulator_" + library.name + ".cpp' in 'plugins/hal-plugins/simulator/src/simulators'.\n"
                            "\n"
                            "\n"
                            "8. Add the following code to the includes within 'plugins/hal-plugins/simulator/src/plugin_simulator.cpp':\n"
                            "\n"
                            "        #include \"simulators/simulator_" + library.name + ".h\""
                            "\n"
                            "\n"
                            "9. Add the following code to the method 'plugin_simulator::instantiate_simulator(const std::string& library)' within 'plugins/hal-plugins/simulator/src/plugin_simulator.cpp':\n"
                            "\n"
                            "        else if (library == \"" + library.name + "\")\n"
                            "        {\n"
                            "            m_simulator = std::make_shared<simulation::simulator_NangateOpenCellLibrary>();\n"
                            "        }\n"
                            "\n"
                            "\n"
                            "10. Rebuild HAL.")
            else:
                text.append("6. Rebuild HAL.")

            # write to file
            f.writelines(text)

    except IOError:
        print_critical("Could not write file: " + file_name + ".")


################################ main

if len(sys.argv) == 2:
    liberty_file = sys.argv[1]
else:
    print_usage()
    exit()

lines = read_liberty(liberty_file)

library = extract_library(lines)

os.makedirs(library.name, exist_ok=True)

write_definition(library)
write_gate_decorator_bdd(library)
write_readme(library)
