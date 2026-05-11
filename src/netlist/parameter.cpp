#include "hal_core/netlist/parameter.h"

namespace hal
{
    template<>
    std::map<Parameter::Type, std::string> EnumStrings<Parameter::Type>::data = {
        {Parameter::Type::BitVector, "bit_vector"},
        {Parameter::Type::Enum, "enum"},
    };

    bool Parameter::operator==(const Parameter& other) const
    {
        return name == other.name && type == other.type && size == other.size && default_value == other.default_value && enum_values == other.enum_values;
    }

    bool Parameter::operator!=(const Parameter& other) const
    {
        return !(*this == other);
    }
}    // namespace hal
