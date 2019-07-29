#/usr/bin/python3

import json


scan_ff = "{\n"
scan_ff += "\t\"library\": {\n"


scan_ff += "\t\t\"library_name\": \"SCAN_FF_LIB\",\n"
scan_ff += "\t\t\"elements\": {\n"
scan_ff += "\t\t\t\"BUF\" : [[\"I\"], [], [\"O\"]], \n"




# add ands and ors
for i in range(2,1000):
    scan_ff += "\t\t\t\"and" + str(i) + "\" : [[\"I0\""
    for j in range (1, i):
        scan_ff += ", \"I" + str(j) + "\""
    scan_ff += "], [], [\"O\"]],\n"

for i in range(2,1000):
    scan_ff += "\t\t\t\"or" + str(i) + "\" : [[\"I0\""
    for j in range (1, i):
        scan_ff += ", \"I" + str(j) + "\""
    scan_ff += "], [], [\"O\"]],\n"


scan_ff += "\t\t\t\"FF\" : [[\"D\"], [], [\"Q\", \"QN\"]] \n"

scan_ff += "\t\t},\n"


scan_ff += "\t\t\"global_gnd_nodes\": [\"GND\"],\n"
scan_ff += "\t\t\"global_vcc_nodes\": [\"VCC\"]\n"



scan_ff += "\t}\n"
scan_ff += "}\n"



file = open("./definitions/SCAN_FF_LIB.json", "w")
file.write(scan_ff)
file.close()