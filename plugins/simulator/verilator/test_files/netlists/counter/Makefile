
all:

VDIRFB  := obj_dir
SUBMAKE := $(MAKE) --no-print-directory -C
VERILATOR := verilator
INCLUDE := -I.

VFLAGS :=  -Wall -Wno-fatal --MMD -trace -y xilinx_unisim_includes/ --Mdir $(VDIRFB) --exe -cc -DSIM_VERILATOR --trace-depth 1


.PHONY: all
all: generic_tb

veri_all: library verilate

%: %.cpp
	$(VERILATOR) $(INCLUDE) $(VFLAGS) $@.cpp counter.v
	$(SUBMAKE) $(VDIRFB)/ -f Vcounter.mk
	make run

run:
	$(VDIRFB)/Vcounter

.PHONY: verilate
verilate: $(VDIRFB)/Vcounter.h

$(VDIRFB)/Vcounter.h:
	$(VERILATOR) $(VFLAGS) generic_tb.cpp counter.v

.PHONY: library
library: $(VDIRFB)/Vcounter.h
	$(SUBMAKE) $(VDIRFB)/ -f Vcounter.mk

.PHONY: clean
clean:
	rm -rf $(VDIRFB)/

DEPS := $(wildcard $(VDIRFB)/*.d)

ifneq ($(DEPS),)
include $(DEPS)
endif