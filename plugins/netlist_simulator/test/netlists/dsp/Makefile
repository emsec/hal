
all:

VDIRFB  := obj_dir
SUBMAKE := $(MAKE) --no-print-directory -C
VERILATOR := verilator
UTILS := ./utils/utils.cpp
INCLUDE := -I./utils/

VFLAGS :=  -Wall -Wno-fatal --MMD -trace  --Mdir $(VDIRFB) --exe $(UTILS) -cc -DSIM_VERILATOR --trace-depth 3


.PHONY: all
all: generic_tb

veri_all: library verilate

%: %.cpp
	$(VERILATOR) $(INCLUDE) $(VFLAGS) $@.cpp dsp_netlist.v DSP.v
	$(SUBMAKE) $(VDIRFB)/ -f Vdsp_netlist.mk
	make run

run:
	$(VDIRFB)/Vdsp_netlist

.PHONY: verilate
verilate: $(VDIRFB)/Vbram_netlist.h

$(VDIRFB)/Vbram_netlist.h:
	$(VERILATOR) $(VFLAGS) generic_tb.cpp dsp_netlist.v DSP.v

.PHONY: library
library: $(VDIRFB)/Vdsp_netlist.h
	$(SUBMAKE) $(VDIRFB)/ -f Vdsp_netlist.mk

.PHONY: clean
clean:
	rm -rf $(VDIRFB)/

DEPS := $(wildcard $(VDIRFB)/*.d)

ifneq ($(DEPS),)
include $(DEPS)
endif
