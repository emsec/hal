
all:

VDIRFB  := obj_dir
SUBMAKE := $(MAKE) --no-print-directory -C
VERILATOR := verilator
INCLUDE := -I./utils/

VFLAGS :=  -Wall -Wno-fatal --MMD -trace  --Mdir $(VDIRFB) --exe $(UTILS) -cc -DSIM_VERILATOR --trace-depth 1


.PHONY: all
all: generic_tb

veri_all: library verilate

%: %.cpp
	$(VERILATOR) $(INCLUDE) $(VFLAGS) $@.cpp bram_netlist.v SB_RAM256x16.v
	$(SUBMAKE) $(VDIRFB)/ -f Vbram_netlist.mk
	make run

run:
	$(VDIRFB)/Vbram_netlist

.PHONY: verilate
verilate: $(VDIRFB)/Vbram_netlist.h

$(VDIRFB)/Vbram_netlist.h:
	$(VERILATOR) $(VFLAGS) generic_tb.cpp bram_netlist.v SB_RAM256x16.v

.PHONY: library
library: $(VDIRFB)/Vbram_netlist.h
	$(SUBMAKE) $(VDIRFB)/ -f Vbram_netlist.mk

.PHONY: clean
clean:
	rm -rf $(VDIRFB)/

DEPS := $(wildcard $(VDIRFB)/*.d)

ifneq ($(DEPS),)
include $(DEPS)
endif
