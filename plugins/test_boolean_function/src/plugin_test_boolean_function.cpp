#include "test_boolean_function/plugin_test_boolean_function.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/types.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<TestBooleanFunctionPlugin>();
    }

    std::string TestBooleanFunctionPlugin::get_name() const
    {
        return std::string("test_boolean_function");
    }

    std::string TestBooleanFunctionPlugin::get_version() const
    {
        return std::string("0.1");
    }

    ProgramOptions TestBooleanFunctionPlugin::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--test_boolean_function", "executes the plugin");

        return description;
    }

    bool TestBooleanFunctionPlugin::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        UNUSED(args);

        UNUSED(nl);

        const auto start = std::chrono::system_clock::now();

        for (int i = 0; i < 100; i++)
        {
            const auto function   = std::get<0>(BooleanFunction::from(
                "((((((((((((((((((((((((((((((((0b0 | (((((I0 & (! I1)) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & I1) & (! I2)) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & (! "
                "I2)) & (! I3)) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & (! I3)) & (! I4)) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & "
                "I3) & (! I4)) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & (! I5))) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & (! I5))) | (((((I0 & I1) & I2) & I3) & (! I4)) & (! "
                "I5))) | ((((((! I0) & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & (! I5))) | ((((((! I0) & (! I1)) & (! I2)) & I3) & I4) & (! I5))) "
                "| (((((I0 & (! I1)) & (! I2)) & I3) & I4) & (! I5))) | (((((I0 & I1) & (! I2)) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & I2) & I3) & I4) & (! I5))) | ((((((! I0) & I1) & (! I2)) "
                "& (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & I1) & I2) & (! I3)) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & (! I2)) & I3) & (! "
                "I4)) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & (! I4)) & I5)) | (((((I0 & I1) & (! I2)) & I3) & (! I4)) & I5)) | ((((((! I0) & (! I1)) & I2) & I3) & (! I4)) & I5)) | ((((((! I0) "
                "& (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & (! I3)) & I4) & I5)) | ((((((! I0) & I1) & I2) & (! "
                "I3)) & I4) & I5)) | (((((I0 & I1) & I2) & (! I3)) & I4) & I5)) | (((((I0 & (! I1)) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & I1) & (! I2)) & I3) & I4) & I5)) | ((((((! I0) & (! "
                "I1)) & I2) & I3) & I4) & I5)) | (((((I0 & (! I1)) & I2) & I3) & I4) & I5)) | (((((I0 & I1) & I2) & I3) & I4) & I5))"));
            const auto simplified = function.simplify();
        }

        const auto duration_in_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start).count();

        printf("DONEEEE\n");

        return true;
    }
    void TestBooleanFunctionPlugin::initialize()
    {
    }
}    // namespace hal
