BUILD_DIR = build

CXX_SOURCES = \
  saleae.cpp \
  ../src/saleae_directory.cpp \
  ../src/saleae_file.cpp \
  ../src/saleae_parser.cpp \
  ../../../../src/utilities/json_write_document.cpp \
  ../../../../src/utilities/program_options.cpp \
  ../../../../src/utilities/program_arguments.cpp \
  ../../../../src/utilities/log.cpp \
  ../../../../src/utilities/utils.cpp \
  ../../../../src/netlist/net.cpp \
  ../../../../src/netlist/endpoint.cpp \
  ../../../../src/netlist/gate.cpp \
  ../../../../src/netlist/boolean_function.cpp \
  ../../../../src/netlist/netlist.cpp \
  ../../../../src/netlist/netlist_internal_manager \
  ../../../../src/netlist/event_handler.cpp \
  ../../../../src/netlist/data_container.cpp


CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -I ../include -I ../../../../include
LDFLAGS =
TARGET = saleae

all: $(BUILD_DIR)/$(TARGET)

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR):
	mkdir $@

.phony: clean
clean:
	-rm -rf $(BUILD_DIR)
