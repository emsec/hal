#include "hgl_parser/hgl_parser.h"

#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include <filesystem>

namespace hal
{
    class HGLParserTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Testing parsing an HGL file.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_library)
    {
        TEST_START
        {
            std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/test.hgl";

            HGLParser parser;
            auto gl_res = parser.parse(path_lib);
            ASSERT_TRUE(gl_res.is_ok());
            std::unique_ptr<GateLibrary> gl_parsed = gl_res.get();
            const GateLibrary* gl_original         = test_utils::get_gate_library();
            ASSERT_NE(gl_parsed.get(), nullptr);
            ASSERT_NE(gl_original, nullptr);

            EXPECT_TRUE(test_utils::gate_libraries_are_equal(gl_parsed.get(), gl_original));
        }
        TEST_END
    }

    /**
     * Parse every .hgl file found in the installed gate library directories and
     * verify that none of them produce a parser error.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_all_gate_libraries)
    {
        TEST_START
        {
            const auto gl_dirs = utils::get_gate_library_directories();

            std::vector<std::filesystem::path> hgl_files;
            for (const auto& dir : gl_dirs)
            {
                if (!std::filesystem::exists(dir))
                    continue;
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
                {
                    if (entry.is_regular_file() && entry.path().extension() == ".hgl")
                        hgl_files.push_back(entry.path());
                }
            }

            ASSERT_FALSE(hgl_files.empty()) << "No .hgl files found in gate library directories — check build configuration.";

            HGLParser parser;
            for (const auto& path : hgl_files)
            {
                auto gl_res = parser.parse(path.string());
                EXPECT_TRUE(gl_res.is_ok()) << "Failed to parse gate library: " << path << "\n" << (gl_res.is_error() ? gl_res.get_error().get() : "");
                if (gl_res.is_ok())
                    EXPECT_NE(gl_res.get(), nullptr) << "Parser returned null for: " << path;
            }
        }
        TEST_END
    }
    /**
     * For every gate type in every installed gate library, verify that each
     * init_identifier referenced by a LUTOutputConfig or InitComponent is also
     * present as a parameter on the gate type, with a sufficient declared size.
     */
    TEST_F(HGLParserTest, check_init_identifiers_have_parameters)
    {
        TEST_START
        {
            const auto gl_dirs = utils::get_gate_library_directories();
            std::vector<std::filesystem::path> hgl_files;
            for (const auto& dir : gl_dirs)
            {
                if (!std::filesystem::exists(dir))
                    continue;
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
                    if (entry.is_regular_file() && entry.path().extension() == ".hgl")
                        hgl_files.push_back(entry.path());
            }
            ASSERT_FALSE(hgl_files.empty());

            HGLParser parser;
            for (const auto& path : hgl_files)
            {
                auto gl_res = parser.parse(path.string());
                if (!gl_res.is_ok())
                    continue;    // parse errors are covered by check_all_gate_libraries
                const auto* gl = gl_res.get().get();
                for (const auto& [gt_name, gt] : gl->get_gate_types())
                {
                    const auto& params = gt->get_parameters();

                    // LUT: every init_identifier needs a parameter sized >= bit_offset + bit_count
                    if (const auto* lut_comp = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }))
                    {
                        for (const auto& [pin_name, cfg] : lut_comp->get_output_pin_configs())
                        {
                            const u32 required_size = cfg.bit_offset + cfg.bit_count;
                            auto it = params.find(cfg.init_identifier);
                            EXPECT_NE(it, params.end())
                                << path.filename().string() << ": " << gt_name << " pin " << pin_name
                                << " — init_identifier '" << cfg.init_identifier << "' has no parameter";
                            if (it != params.end())
                                EXPECT_GE(it->second.get_size(), required_size)
                                    << path.filename().string() << ": " << gt_name << " pin " << pin_name
                                    << " — parameter '" << cfg.init_identifier << "' size " << it->second.get_size()
                                    << " < required " << required_size;
                        }
                    }

                    // FF / latch: every init_identifier needs a parameter
                    if (const auto* init_comp = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); }))
                    {
                        for (const auto& identifier : init_comp->get_init_identifiers())
                        {
                            EXPECT_NE(params.find(identifier), params.end())
                                << path.filename().string() << ": " << gt_name
                                << " — init_identifier '" << identifier << "' has no parameter";
                        }
                    }
                }
            }
        }
        TEST_END
    }
}    //namespace hal
