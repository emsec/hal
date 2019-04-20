import sys

def process_file(input_file, output_file, merge_components, ignore_list, focus_list):
	with open(input_file, "rt") as file:
		signal_cnt = 0
		component_cnt = dict()
		renaming = dict()
		output = ""
		in_port_map = False

		# go through the file line by line
		for line in file.readlines():
			stripped = line.strip()
			parts = stripped.split()

			# if line defines a signal we store the renaming information
			if stripped.startswith("signal "):

				# extract signal name
				name = stripped[stripped.index(" ")+1:]
				name = name[:name.index(" ")]

				# check ignore and focus list
				if not any(ignoreterm in name for ignoreterm in ignore_list):
					if len(focus_list) == 0 or any(focusterm in name for focusterm in focus_list):

						# register name and rename signal and update current line
						renaming[name] = "signal_" + str(signal_cnt)
						signal_cnt += 1
						line = line.replace(name, renaming[name])

			# check whether we are in a "port map" section
			elif not in_port_map and stripped == "port map (":
				in_port_map = True
			elif in_port_map and stripped == ");":
				in_port_map = False

			# if we are in a "port map" section, replace all signals
			elif in_port_map and " => " in stripped:

				# extract signal name and whatever comes afterwards
				name = stripped[stripped.index(" => ")+4:]
				rest = ""
				if "(" in name:
					rest = name[name.index("("):]
					name = name[:name.index("(")]
				if name[-1] == ",":
					name = name[:-1]
					rest = ","

				# if the signal is registered for renaming, do so, otherwise line is unchanged
				if name in renaming:
					line = line[:line.index(" => ") + 4] + renaming[name] + rest+"\n"

			# if we are not in a "port map" section, look for component instantiations "name : type"
			elif len(parts) == 3 and parts[1] == ":":

				# check ignore and focus list
				if not any(ignoreterm in parts[0] for ignoreterm in ignore_list):
					if len(focus_list) == 0 or any(focusterm in parts[0] for focusterm in focus_list):

						# replace component
						replacement = parts[2] + "_"
						if merge_components:
							replacement = "component_"

						if replacement not in component_cnt:
							component_cnt[replacement] = 0

						line = line.replace(parts[0], replacement+str(component_cnt[replacement]))
						component_cnt[replacement] += 1

			# append the (possibly) modified line to the output
			output += line

		# write the output to the output file
		with open(output_file, "wt") as outfile:
		    outfile.write(output)



##################################################################
##################################################################
##################################################################

def print_usage():
    print("VHDL Netlist Blinder")
    print("  (custom script, may contain errors)")
    print("")
    print("  Usage: python blind.py inputfile outputfile [-m/-merge] [-i/-ignore ignoreterms...] [-f/-focus focusterms...]")
    print("")
    print("  -f/-focus    If focusterms are present, only signals and components which contain a focusterm are analyzed.")
    print("  -i/-ignore   All signals and components which contain any ignoreterm will be left unchanged.")
    print("  -m/-merge    If merging is active, all components are named \"component_x\" instead of e.g. \"LUT6_x\".")
    print("")

# parameter check
if len(sys.argv) < 3:
    print_usage()
    sys.exit(1)

# parse the command line arguments
ignore_list = []
focus_list = []
do_ignore = False
do_focus = False
merge_components = False
for i in xrange(1, len(sys.argv)):
    if sys.argv[i] == "-i" or sys.argv[i] == "-ignore":
        do_ignore = True
        do_focus = False
        continue
    if sys.argv[i] == "-f" or sys.argv[i] == "-focus":
        do_focus = True
        do_ignore = False
        continue
    if sys.argv[i] == "-m" or sys.argv[i] == "-merge":
        merge_components = True
        continue
    if do_ignore:
        ignore_list.append(sys.argv[i])
    elif do_focus:
        focus_list.append(sys.argv[i])

# process the inputs
process_file(sys.argv[1], sys.argv[2], merge_components, ignore_list, focus_list)
