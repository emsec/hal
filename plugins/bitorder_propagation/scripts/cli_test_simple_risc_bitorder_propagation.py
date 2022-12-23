#!/usr/bin/env python3
import sys, os

#some necessary configuration:
sys.path.append("/home/simon/projects/hal/build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = "/home/simon/projects/hal/build/" # hal base path+

import hal_py

netlist_to_read = "/home/simon/workspaces/2022_12_22/simple_risc_v_bitorder_test/simple_risc_v_bitorder_test.hal"
gate_library_path = "/home/simon/projects/hal/plugins/real_world_reversing/gate_libraries/ice40ultra_iphone.hgl"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read)

from hal_plugins import bitorder_propagation

known_bitorders = [(71, "OUT"), (71, "IN"), (5, "3539_RDATA"), (5, "3539_WADDR"), (5, "3539_WDATA"), (5, "3539_RADDR"), (4, "3540_RADDR"), (4, "3540_WADDR"), (4, "3540_MASK"), (4, "3540_WDATA"), (4, "3540_RDATA"), (72, "OUT"), (72, "IN"), (2, "4417_ADDRESS"), (2, "4418_DATAOUT"), (2, "4418_DATAIN"), (2, "4417_DATAIN"), (2, "4417_MASKWREN"), (2, "4417_DATAOUT"), (3, "3541_RDATA"), (3, "3542_RDATA"), (3, "3541_WDATA"), (3, "3541_RADDR"), (3, "3541_WADDR"), (3, "3541_MASK"), (3, "3542_WDATA"), (70, "B"), (70, "A"), (74, "IN"), (74, "OUT"), (73, "OUT"), (73, "IN"), (64, "A"), (64, "OUT"), (64, "B"), (65, "OUT"), (65, "B"), (65, "A"), ]
unknown_bitorders = [(10, "Q"), (10, "D"), (51, "Q"), (51, "D"), (50, "Q"), (50, "D"), (44, "Q"), (44, "D"), (45, "D"), (45, "Q"), (53, "D"), (53, "Q"), (33, "D"), (33, "Q"), (24, "D"), (24, "Q"), (30, "D"), (30, "Q"), (26, "Q"), (26, "D"), (54, "D"), (54, "Q"), (20, "Q"), (20, "D"), (11, "D"), (11, "Q"), (41, "D"), (41, "Q"), (25, "Q"), (25, "D"), (13, "Q"), (13, "D"), (21, "Q"), (21, "D"), (52, "Q"), (52, "D"), (28, "D"), (28, "Q"), (35, "Q"), (35, "D"), (47, "D"), (47, "Q"), (16, "Q"), (16, "D"), (12, "D"), (12, "Q"), (62, "D"), (62, "Q"), (36, "D"), (36, "Q"), (23, "D"), (23, "Q"), (61, "D"), (61, "Q"), (29, "D"), (29, "Q"), (34, "D"), (34, "Q"), (46, "Q"), (46, "D"), (9, "Q"), (9, "D"), (39, "D"), (39, "Q"), (19, "Q"), (19, "D"), (48, "Q"), (48, "D"), (32, "Q"), (32, "D"), (14, "Q"), (14, "D"), (18, "D"), (18, "Q"), (60, "D"), (60, "Q"), (58, "Q"), (58, "D"), (59, "Q"), (59, "D"), (56, "D"), (56, "Q"), (42, "Q"), (42, "D"), (43, "Q"), (43, "D"), (17, "D"), (17, "Q"), (22, "Q"), (22, "D"), (57, "Q"), (57, "D"), (63, "D"), (63, "Q"), (15, "Q"), (15, "D"), (31, "D"), (31, "Q"), (38, "D"), (38, "Q"), (37, "D"), (37, "Q"), (40, "Q"), (40, "D"), (49, "Q"), (49, "D"), (27, "Q"), (27, "D"), (55, "D"), (55, "Q"), ]

res_1 = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(netlist, known_bitorders, unknown_bitorders)

print("Done")

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()
