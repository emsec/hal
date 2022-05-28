#include <iostream>
#include <netlist_simulator_controller/saleae_directory.h>
#include <hal_core/utilities/program_options.h>
#include <hal_core/utilities/program_arguments.h>

using namespace std;
using namespace hal;




int main(int argc, const char* argv[]) {
    /* initialize and parse cli options */
    ProgramOptions cli_options("cli options");
    cli_options.add("--help", "print help messages");
    cli_options.add("ls", "Lists content of saleae directory file saleae.json");
    cli_options.add("cat", "Dump content of binary file into console", {""});

    ProgramArguments args = cli_options.parse(argc, argv);

    if (args.is_option_set("ls")) {
        cli_options.remove("ls");
        cli_options.remove("cat");
        cli_options.add({"-d", "--dir"}, "lists saleae directory from directory given by absolute or relative path name", {ProgramOptions::A_REQUIRED_PARAMETER});
        cli_options.add({"-s", "--size"}, "lists only entries with given number of waveform events", {ProgramOptions::A_REQUIRED_PARAMETER});
        cli_options.add({"-i", "--id"}, "list only entries where ID matches entry in list. Entries are separated by comma. A single entry can be either an ID or a range sepearated by hyphen.", {ProgramOptions::A_REQUIRED_PARAMETER});
    }
    else if (args.is_option_set("cat")) {
        cli_options.remove("ls");
        cli_options.remove("cat");
        cli_options.add({"-d", "--dir"}, "binary file is not in current directory but in directory given by path name", {ProgramOptions::A_REQUIRED_PARAMETER});
        cli_options.add({"-h", "--only-header"}, "dump only header");
        cli_options.add({"-b", "--only-data"}, "dump only data including start value");
    }


    bool unknown_option_exists = false;
    /* process help output */
    if (args.is_option_set("--help") || args.get_set_options().size() == 0 || unknown_option_exists) {
        std::cout << cli_options.get_options_string() << std::endl;
    }

    SaleaeDirectory *sd = new SaleaeDirectory("/home/parallels/Desktop/saleae/saleae.json", false);
    //sd->dump();
}