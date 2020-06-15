#include "netlist/hdl_writer/hdl_writer_dispatcher.h"

#include "core/log.h"
#include "netlist/hdl_writer/hdl_writer.h"
#include "netlist/hdl_writer/hdl_writer_verilog.h"
#include "netlist/hdl_writer/hdl_writer_vhdl.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"

#include <chrono>

namespace hal
{
    namespace hdl_writer_dispatcher
    {
        program_options get_cli_options()
        {
            program_options description;
            description.add("--write-verilog", "Write Verilog to file", {""});
            description.add("--write-vhdl", "Write VHDL to file", {""});
            return description;
        }

        bool write(std::shared_ptr<netlist> g, const program_arguments& args)
        {
            // all configurations: command, language, file extension
            std::vector<std::tuple<std::string, std::string, std::string>> configs = {std::make_tuple("--write-vhdl", "vhdl", ".vhd"), std::make_tuple("--write-verilog", "verilog", ".v")};

            bool success = true;

            for (const auto& tup : configs)
            {
                // check whether the command was issued
                if (args.is_option_set(std::get<0>(tup)) > 0)
                {
                    std::filesystem::path file = g->get_input_filename();

                    // if an optional path was issued (via --write-XY path), use this one
                    std::string option = args.get_parameter(std::get<0>(tup));
                    if (!option.empty())
                    {
                        file = option;
                    }

                    // use correct extension
                    file.replace_extension(std::get<2>(tup));

                    // serialize
                    success &= write(g, std::get<1>(tup), file);
                }
            }

            return success;    // if nothing is written, the writer is always successful
        }

        bool write(std::shared_ptr<netlist> g, const std::string& format, const std::filesystem::path& file_name)
        {
            std::ofstream hdl_file;

            hdl_file.open(file_name.string());
            if (hdl_file.fail())
            {
                log_error("hdl_writer", "Cannot open or create file {}. Please verify that the file and the containing directory is writable!", file_name.string());
                return false;
            }

            std::stringstream output_stream;
            bool write_success = false;

            auto begin_time = std::chrono::high_resolution_clock::now();

            if (format == "vhdl")
            {
                write_success = hdl_writer_vhdl(output_stream).write(g);
            }
            else if (format == "verilog")
            {
                write_success = hdl_writer_verilog(output_stream).write(g);
            }
            else
            {
                log_error("hdl_writer", "Output format {} unknown!", format);
                return false;
            }

            // done
            hdl_file << output_stream.str();
            hdl_file.close();

            log_info("hdl_writer",
                     "wrote '{}' to '{}' in {:2.2f} seconds.",
                     g->get_design_name(),
                     file_name.string(),
                     (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

            return write_success;
        }
    }    // namespace hdl_writer_dispatcher
}    // namespace hal
