#include "plugin_gate_decorators.h"

#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"

std::string plugin_gate_decorators::get_name()
{
    return std::string("gate_decorators");
}

std::string plugin_gate_decorators::get_version()
{
    return std::string("1.1");
}

void plugin_gate_decorators::on_load()
{
}

void plugin_gate_decorators::on_unload()
{
}
