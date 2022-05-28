#include <iostream>
#include <iomanip>
#include <netlist_simulator_controller/saleae_directory.h>
#include <hal_core/utilities/program_options.h>
#include <hal_core/utilities/program_arguments.h>

using namespace std;
using namespace hal;

template<typename T> void print_element(T t, const int& width, bool align)
{
    if (align) {
        cout << left << setw(width) << setfill(' ') << t << " | ";
    }
    else {
        cout << right << setw(width) << setfill(' ') << t << " | ";
    }
}

void saleae_ls(string path, int size) {
    if (path == "") {
        path = "./saleae.json";
    }
    else {
        path += "/saleae.json";
    }

    SaleaeDirectory *sd = new SaleaeDirectory(path, false);


    vector<SaleaeDirectoryNetEntry> NetEntries = sd->dump();
    int format_length [6] = {7,8,19,11,10,15};
    for (const SaleaeDirectoryNetEntry& sdne : sd->dump()) {
        format_length[0] = (format_length[0] < to_string(sdne.id()).length()) ? to_string(sdne.id()).length() : format_length[0];
        format_length[1] = (format_length[1] < sdne.name().length()) ? sdne.name().length() : format_length[1];
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes()) {
            format_length[2] = (format_length[2] < to_string(sdfi.numberValues()).length()) ? to_string(sdfi.numberValues()).length() : format_length[2];
            format_length[3] = (format_length[3] < to_string(sdfi.beginTime()).length()) ? to_string(sdfi.beginTime()).length() : format_length[3];
            format_length[4] = (format_length[4] < to_string(sdfi.endTime()).length()) ? to_string(sdfi.endTime()).length() : format_length[4];
            format_length[5] = (format_length[5] < to_string(sdfi.index()).length() + 12) ? to_string(sdfi.index()).length() + 12: format_length[5];
        }
    }
    int abs_length = format_length[0] + format_length[1] + format_length[2] + format_length[3] + format_length[4] + format_length[5] + 16;

    cout << string(abs_length + 2, '-') << endl;
    print_element("| Net ID", format_length[0], true);
    print_element("Net Name", format_length[1], true);
    print_element("Total Number Values", format_length[2], true);
    print_element("First Event", format_length[3], true);
    print_element("Last Event", format_length[4], true);
    print_element("Binary filename", format_length[5], true);
    cout << endl;
    cout << '|' << string(abs_length, '-') << '|' << endl;
    for (const SaleaeDirectoryNetEntry& sdne : sd->dump()) {
        cout << '|';
        print_element(sdne.id(), format_length[0], false);
        print_element(sdne.name(), format_length[1], true);
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes()) {
            print_element(sdfi.numberValues(), format_length[2], false);
            print_element(sdfi.beginTime(), format_length[3], false);
            print_element(sdfi.endTime(), format_length[4], false);
            print_element("digital_" + to_string(sdfi.index()) + ".bin", format_length[5], true);
        }
        cout << endl;
    }
}





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

        ProgramArguments args = cli_options.parse(argc, argv);
        saleae_ls(args.get_parameter("--dir"), 0);
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

    
}