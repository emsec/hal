#!/usr/bin/python3

import os
import getpass

# ---- CONFIGURATION ----
username = getpass.getuser()

path_to_core_collection = "/home/" + username + "/git/hal-benchmarks"
path_to_hal = "/home/" + username + "/git/hal"
path_to_hal_build = path_to_hal + "/build-release"
path_to_hal_bin = path_to_hal_build + "/bin/hal"
path_dataflow_out = path_to_hal_build + "/dataflow_out"

netlists = dict()
netlists["vivado"] = dict()
netlists["synopsys"] = dict()


default_gate_libraries = dict()
default_gate_libraries["vivado"] = "XILINX_UNISIM"
default_gate_libraries["synopsys"] = "lsi_10k"

special_gate_libraries = dict()

# test
netlists["vivado"]["test"] = "test/test.vhd"

# aes
netlists["vivado"]["tiny_aes"] = "crypto/tiny_aes/tiny_aes_hsing_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["tiny_aes"] = "crypto/tiny_aes/tiny_aes_hsing_lsi_10k_synopsys.v"

# des
netlists["vivado"]["des"] = "crypto/des/des_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["des"] = "crypto/des/des_lsi_10k_synopsys.v"


# keccak
netlists["vivado"]["sha-3"] = "crypto/sha-3/sha-3_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["sha-3"] = "crypto/sha-3/sha-3_lsi_10k_synopsys.v"

# present
netlists["vivado"]["present"] = "crypto/present/present_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["present"] = "crypto/present/present_lsi_10k_synopsys.v"

# rsa
netlists["vivado"]["rsa"] = "crypto/rsa/rsa_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["rsa"] = "crypto/rsa/rsa_lsi_10k_synopsys.v"



# open8
netlists["vivado"]["open8"] = "cpu/open8/open8_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["open8"] = "cpu/open8/open8_lsi_10k_synopsys.v"


# edge
netlists["vivado"]["edge"] = "cpu/edge/egde_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["edge"] = "cpu/edge/edge_lsi_10k_synopsys.v"

# ibex
netlists["vivado"]["ibex"] = "cpu/ibex_risc-v/ibex_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["ibex"] = "cpu/ibex_risc-v/ibex_lsi_10k_synopsys.v"



# opentitan
netlists["vivado"]["opentitan"] = "soc/opentitan/opentitan_xilinx_unisim_vivado.vhd"
netlists["synopsys"]["opentitan"] = "soc/opentitan/opentitan_lsi_10k_synopsys.vhd"
special_gate_libraries[("synopsys", "opentitan")] = "lsi_10k"

netlists["vivado"]["axi"] = "others/1m_2s_v1.vhd"

netlists["vivado"]["soc"] = "soc/netlist/soc.vhd"


netlists["vivado"]["aes_trojan"] = "crypto/tiny_aes/tiny_aes_trojan.vhd"
netlists["vivado"]["rsa_trojan"] = "crypto/rsa/rsa_trojan.vhd"


# netlists for paper:

netlists_paper = dict()

# crypto
netlists_paper["tiny_aes"] = "128"
netlists_paper["des"] = "56,32"
netlists_paper["rsa"] = "512"
netlists_paper["present"] = "80,64"
netlists_paper["sha-3"] = "1600,512,1088"


# cpu
netlists_paper["ibex"] = "31,32,33"
netlists_paper["edge"] = "32"
netlists_paper["open8"] = "16,8"


#soc
netlists_paper["opentitan"] = "512,256,128,33,32,31"
