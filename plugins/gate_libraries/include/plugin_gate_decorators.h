#ifndef HAL_PLUGIN_GATE_DECORATORS_H
#define HAL_PLUGIN_GATE_DECORATORS_H

#include "core/interface_base.h"
#include <map>
#include <memory>
#include <string>

class bdd;
class gate;

class PLUGIN_API plugin_gate_decorators : virtual public i_base
{
public:
    std::string get_name() override;

    std::string get_version() override;

    void on_load() override;

    void on_unload() override;

private:
};

#endif
