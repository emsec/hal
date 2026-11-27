#include "hgl_parser/hgl_parser.h"
#include "hgl_parser/upgrade_hgl.h"

#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include <filesystem>
#include <fstream>
#include <sstream>

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
    namespace
    {
        /**
         * A library with a single-output LUT, a multi-output LUT in the style of Xilinx LUT2_D, and a flip-flop, with
         * LUT configurations in the flat format of HGL version 5 and earlier.
         */
        std::string make_legacy_library(u32 version, const std::string& extra_cell = "")
        {
            auto pin_group = [](const std::string& name, const std::string& direction, const std::string& type) {
                return R"({"name": ")" + name + R"(", "direction": ")" + direction + R"(", "type": ")" + type + R"(", "ascending": false, "start_index": 0, "pins": [{"name": ")" + name
                       + R"(", "direction": ")" + direction + R"(", "type": ")" + type + R"("}]})";
            };
            const std::string flat_lut = R"("lut_config": {"bit_order": "descending", "data_category": "generic", "data_identifier": "INIT"})";

            std::string doc = R"({"version": )" + std::to_string(version) + R"(, "library": "LEGACY", "cells": [)";
            doc += R"({"name": "LUT2", "types": ["combinational", "lut"], )" + flat_lut + R"(, "pin_groups": [)" + pin_group("I0", "input", "none") + ", " + pin_group("I1", "input", "none") + ", "
                   + pin_group("O", "output", "lut") + "]}, ";
            doc += R"({"name": "LUT2_D", "types": ["combinational", "c_lut"], )" + flat_lut + R"(, "pin_groups": [)" + pin_group("I0", "input", "none") + ", " + pin_group("I1", "input", "none")
                   + ", " + pin_group("O", "output", "lut") + ", " + pin_group("LO", "output", "lut") + "]}, ";
            doc += R"({"name": "FF", "types": ["sequential", "ff"], "ff_config": {"state": "IQ", "neg_state": "IQN", "next_state": "D", "clocked_on": "C", "data_category": "generic", "data_identifier": "INIT"}, "pin_groups": [)"
                   + pin_group("D", "input", "data") + ", " + pin_group("C", "input", "clock") + ", " + pin_group("Q", "output", "state") + "]}";
            if (!extra_cell.empty())
            {
                doc += ", " + extra_cell;
            }
            doc += "]}";
            return doc;
        }

        rapidjson::Document parse_json(const std::string& str)
        {
            rapidjson::Document document;
            document.Parse(str.c_str());
            return document;
        }

        const rapidjson::Value* find_cell(const rapidjson::Document& document, const std::string& name)
        {
            for (const auto& cell : document["cells"].GetArray())
            {
                if (name == cell["name"].GetString())
                {
                    return &cell;
                }
            }
            return nullptr;
        }

        /** A fresh directory for files that tests write, removed again when it goes out of scope. */
        class TempDirectory
        {
        public:
            TempDirectory(const std::string& name) : m_path(std::filesystem::temp_directory_path() / ("hal_hgl_parser_test_" + name))
            {
                std::filesystem::remove_all(m_path);
                std::filesystem::create_directories(m_path);
            }
            ~TempDirectory()
            {
                std::error_code ec;
                std::filesystem::remove_all(m_path, ec);
            }
            const std::filesystem::path& path() const
            {
                return m_path;
            }

        private:
            std::filesystem::path m_path;
        };

        void write_file(const std::filesystem::path& path, const std::string& content)
        {
            std::ofstream(path) << content;
        }

        std::string read_file(const std::filesystem::path& path)
        {
            std::ifstream ifs(path);
            std::stringstream ss;
            ss << ifs.rdbuf();
            return ss.str();
        }
    }    // namespace

    /**
     * Upgrading migrates every LUT configuration, including those of multi-output LUTs, so that each output depends on
     * all inputs and the whole INIT value, as it did in the flat format.
     *
     * Functions: hgl_upgrade::upgrade_document, hgl_upgrade::needs_upgrade
     */
    TEST_F(HGLParserTest, check_upgrade_multi_output_lut)
    {
        TEST_START
        {
            rapidjson::Document document = parse_json(make_legacy_library(4));
            ASSERT_TRUE(hgl_upgrade::needs_upgrade(document));

            auto res = hgl_upgrade::upgrade_document(document);
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            // the multi-output LUT should be reviewed, nothing else
            ASSERT_EQ(res.get().size(), 1);
            EXPECT_NE(res.get().front().find("LUT2_D"), std::string::npos);

            EXPECT_EQ(document["version"].GetUint(), hgl_upgrade::TARGET_VERSION);
            EXPECT_FALSE(hgl_upgrade::needs_upgrade(document));

            const rapidjson::Value* lut2_d = find_cell(document, "LUT2_D");
            ASSERT_NE(lut2_d, nullptr);
            const auto& output_pins = (*lut2_d)["lut_config"]["output_pins"];
            ASSERT_TRUE(output_pins.IsArray());
            ASSERT_EQ(output_pins.Size(), 2);
            std::set<std::string> pins;
            for (const auto& entry : output_pins.GetArray())
            {
                pins.insert(entry["pin"].GetString());
                EXPECT_STREQ(entry["data_identifier"].GetString(), "INIT");
                EXPECT_EQ(entry["bit_offset"].GetUint(), 0);
                EXPECT_EQ(entry["bit_count"].GetUint(), 4);
                EXPECT_FALSE(entry["is_ascending"].GetBool());
                ASSERT_EQ(entry["input_pins"].Size(), 2);
                EXPECT_STREQ(entry["input_pins"][0].GetString(), "I0");
                EXPECT_STREQ(entry["input_pins"][1].GetString(), "I1");
            }
            EXPECT_EQ(pins, std::set<std::string>({"O", "LO"}));

            const auto& parameters = (*lut2_d)["parameters"];
            ASSERT_EQ(parameters.Size(), 1);
            EXPECT_STREQ(parameters[0]["name"].GetString(), "INIT");
            EXPECT_EQ(parameters[0]["size"].GetUint(), 4);

            const rapidjson::Value* lut2 = find_cell(document, "LUT2");
            ASSERT_NE(lut2, nullptr);
            EXPECT_EQ((*lut2)["lut_config"]["output_pins"].Size(), 1);
            EXPECT_STREQ((*lut2)["types"][1].GetString(), "c_lut");

            const rapidjson::Value* ff = find_cell(document, "FF");
            ASSERT_NE(ff, nullptr);
            EXPECT_EQ((*ff)["parameters"][0]["size"].GetUint(), 1);
        }
        TEST_END
    }

    /**
     * A cell that cannot be migrated makes the whole upgrade fail instead of producing a document that claims to be up
     * to date.
     *
     * Functions: hgl_upgrade::upgrade_document
     */
    TEST_F(HGLParserTest, check_upgrade_fails_for_unmigratable_cell)
    {
        TEST_START
        {
            // a LUT configuration without any LUT output pin
            const std::string broken = R"({"name": "BROKEN", "types": ["combinational"], "lut_config": {"data_category": "generic", "data_identifier": "INIT"}, )"
                                       R"("pin_groups": [{"name": "I", "direction": "input", "type": "none", "ascending": false, "start_index": 0, "pins": [{"name": "I", "direction": "input", "type": "none"}]}, )"
                                       R"({"name": "O", "direction": "output", "type": "none", "ascending": false, "start_index": 0, "pins": [{"name": "O", "direction": "output", "type": "none"}]}]})";
            rapidjson::Document document = parse_json(make_legacy_library(4, broken));

            auto res = hgl_upgrade::upgrade_document(document);
            ASSERT_TRUE(res.is_error());
            EXPECT_NE(res.get_error().get().find("BROKEN"), std::string::npos);
            EXPECT_EQ(document["version"].GetUint(), 4);
        }
        TEST_END
    }

    /**
     * A file of the current version that still contains flat LUT configurations, as written by earlier upgrades for
     * multi-output LUTs, is rejected by the parser and repaired by an upgrade, without overwriting the existing backup.
     *
     * Functions: parse, hgl_upgrade::needs_upgrade, hgl_upgrade::replace_file
     */
    TEST_F(HGLParserTest, check_repair_of_incomplete_upgrade)
    {
        TEST_START
        {
            TempDirectory dir("repair");
            const std::filesystem::path lib_path    = dir.path() / "legacy.hgl";
            const std::filesystem::path old_backup  = dir.path() / "legacy.json";
            const std::filesystem::path new_backup  = dir.path() / "legacy.1.json";
            const std::string content               = make_legacy_library(hgl_upgrade::TARGET_VERSION);
            write_file(lib_path, content);
            write_file(old_backup, "backup of an earlier upgrade");

            EXPECT_TRUE(hgl_upgrade::needs_upgrade(parse_json(content)));

            {
                HGLParser parser;
                auto gl_res = parser.parse(lib_path);
                ASSERT_TRUE(gl_res.is_error());
                EXPECT_TRUE(GateLibraryParser::get_parse_status() == GateLibraryParser::ParseStatus::NeedsUpgrade);
            }
            EXPECT_EQ(read_file(lib_path), content);

            GateLibraryParser::request_upgrade();
            {
                HGLParser parser;
                auto gl_res = parser.parse(lib_path);
                ASSERT_TRUE(gl_res.is_ok()) << gl_res.get_error().get();
                std::unique_ptr<GateLibrary> gl = gl_res.get();

                GateType* lut2_d = gl->get_gate_type_by_name("LUT2_D");
                ASSERT_NE(lut2_d, nullptr);
                const auto* lut_comp = lut2_d->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                ASSERT_NE(lut_comp, nullptr);
                EXPECT_EQ(lut_comp->get_output_pin_configs().size(), 2);
            }

            EXPECT_EQ(read_file(old_backup), "backup of an earlier upgrade");
            EXPECT_EQ(read_file(new_backup), content);
            EXPECT_FALSE(hgl_upgrade::needs_upgrade(parse_json(read_file(lib_path))));
        }
        TEST_END
    }

    /**
     * A flat LUT configuration that cannot be migrated is rejected without asking for an upgrade, instead of creating a
     * LUT without any function.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_flat_lut_config_is_rejected)
    {
        TEST_START
        {
            TempDirectory dir("flat_lut");
            const std::filesystem::path lib_path = dir.path() / "flat.hgl";
            // a flat LUT configuration without a data identifier
            const std::string content = R"({"version": 6, "library": "FLAT", "cells": [{"name": "LUT1", "types": ["combinational", "c_lut"], "lut_config": {"data_category": "generic"}, )"
                                        R"("pin_groups": [{"name": "I", "direction": "input", "type": "none", "ascending": false, "start_index": 0, "pins": [{"name": "I", "direction": "input", "type": "none"}]}, )"
                                        R"({"name": "O", "direction": "output", "type": "lut", "ascending": false, "start_index": 0, "pins": [{"name": "O", "direction": "output", "type": "lut"}]}]}]})";
            write_file(lib_path, content);

            HGLParser parser;
            auto gl_res = parser.parse(lib_path);
            ASSERT_TRUE(gl_res.is_error());
            // the configuration cannot be migrated, so the user must not be asked for an upgrade
            EXPECT_TRUE(GateLibraryParser::get_parse_status() != GateLibraryParser::ParseStatus::NeedsUpgrade);
            EXPECT_EQ(read_file(lib_path), content);
        }
        TEST_END
    }
}    //namespace hal
