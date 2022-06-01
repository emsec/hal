#include <iostream>
#include <iomanip>
#include <algorithm>
#include <netlist_simulator_controller/saleae_directory.h>
#include <netlist_simulator_controller/saleae_file.h>
#include <hal_core/utilities/program_options.h>
#include <hal_core/utilities/program_arguments.h>

using namespace std;
using namespace hal;

// template for space saving printing
template<typename T> void print_element(T t, const int& width, bool align) {
    if (align) {
        cout << left << setw(width) << setfill(' ') << t << " | ";
    }
    else {
        cout << right << setw(width) << setfill(' ') << t << " | ";
    }
}

// check, given the size, whether an entry may be printed
bool check_size(bool necessary, char op, int size_val, int compare_val) {
    bool ret = true;
    if (necessary) {
        ret = false;
        switch (op) {
            case '+':
                if (compare_val > size_val) ret = true;
                break;
            case '-':
                if (compare_val < size_val) ret = true;
                break;
            default:
                if (compare_val == size_val) ret = true;
                break;
        }
    }
    return ret;
}

// check, given an id list, whether an entry may be printed
bool check_ids(bool necessary, vector<int> ids, int id_to_check) {
    bool ret = true;
    if (necessary) {
        ret = false;
        if (find(ids.begin(), ids.end(), id_to_check) != ids.end()) {
            ret = true;
        }
    }
    return ret;
}

// saleae ls-tool
void saleae_ls(string path, string size, string ids) {
    // handle --dir option
    path = (path == "") ? "./saleae.json" : path + "/saleae.json";

    // handle --size option
    bool size_necessary = false;
    char size_op = '=';
    int size_val = 0;
    if (size != "") {
        size_necessary = true;
        if (size[0] == '+' || size[0] == '-') {
            size_op = size[0];
            size_val = stoi(size.substr(1));
        }
        else {
            size_val = stoi(size);
        }
    }

    // handle --id option
    bool ids_necessary = false;
    vector<int> id_vector;
    if (ids != "") {
        ids_necessary = true;
        stringstream ss(ids);
        vector<string> splited_ids;
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            splited_ids.push_back(substr);
        }

        for (string id_entry : splited_ids) {
            if (id_entry.find('-') != string::npos) {
                stringstream range_stream(id_entry);
                vector<string> range;
                while (range_stream.good()) {
                    string substr;
                    getline(range_stream, substr, '-');
                    range.push_back(substr);
                }
                int tmp_id = stoi(range.front());
                while (tmp_id <= stoi(range.back())) {
                    id_vector.push_back(tmp_id);
                    tmp_id ++;
                }
            }
            else {
               id_vector.push_back(stoi(id_entry)); 
            }
        }
    }

    // collect length for better formatting
    SaleaeDirectory *sd = new SaleaeDirectory(path, false);
    vector<SaleaeDirectoryNetEntry> NetEntries = sd->dump();
    int format_length [6] = {7, 8, 19, 11, 10, 15}; // length of the column titles
    for (const SaleaeDirectoryNetEntry& sdne : sd->dump()) {
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes()) {
            if (check_size(size_necessary, size_op, size_val, sdfi.numberValues()) && check_ids(ids_necessary, id_vector, sdne.id())) {
                format_length[0] = (format_length[0] < to_string(sdne.id()).length()) ? to_string(sdne.id()).length() : format_length[0];
                format_length[1] = (format_length[1] < sdne.name().length()) ? sdne.name().length() : format_length[1];
                format_length[2] = (format_length[2] < to_string(sdfi.numberValues()).length()) ? to_string(sdfi.numberValues()).length() : format_length[2];
                format_length[3] = (format_length[3] < to_string(sdfi.beginTime()).length()) ? to_string(sdfi.beginTime()).length() : format_length[3];
                format_length[4] = (format_length[4] < to_string(sdfi.endTime()).length()) ? to_string(sdfi.endTime()).length() : format_length[4];
                format_length[5] = (format_length[5] < to_string(sdfi.index()).length() + 12) ? to_string(sdfi.index()).length() + 12: format_length[5];
            }
        }
    }
    int abs_length = format_length[0] + format_length[1] + format_length[2] + format_length[3] + format_length[4] + format_length[5] + 16;

    // print saleae dir content
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
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes()) {
            if (check_size(size_necessary, size_op, size_val, sdfi.numberValues()) && check_ids(ids_necessary, id_vector, sdne.id())) {
                cout << '|';
                print_element(sdne.id(), format_length[0], false);
                print_element(sdne.name(), format_length[1], true);
                print_element(sdfi.numberValues(), format_length[2], false);
                print_element(sdfi.beginTime(), format_length[3], false);
                print_element(sdfi.endTime(), format_length[4], false);
                print_element("digital_" + to_string(sdfi.index()) + ".bin", format_length[5], true);
                cout << endl;
            }
        }
    }
}


// void saleae_cat(string path) {
//     SaleaeInputFile *sf = new SaleaeInputFile("/home/parallels/Desktop/saleae/digital_0.bin");
//     SaleaeDataBuffer *db = new SaleaeDataBuffer();
//     db = sf->get_buffered_data(1);
//     db->dump();
// }


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
        saleae_ls(args.get_parameter("--dir"), args.get_parameter("--size"), args.get_parameter("--id"));
    }
    else if (args.is_option_set("cat")) {
        cli_options.remove("ls");
        cli_options.remove("cat");
        cli_options.add({"-d", "--dir"}, "binary file is not in current directory but in directory given by path name", {ProgramOptions::A_REQUIRED_PARAMETER});
        cli_options.add({"-h", "--only-header"}, "dump only header");
        cli_options.add({"-b", "--only-data"}, "dump only data including start value");

        ProgramArguments args = cli_options.parse(argc, argv);
        //saleae_cat("/home/parallels/Desktop/saleae/digital_0.bin");
    }

    bool unknown_option_exists = false;
    /* process help output */
    if (args.is_option_set("--help") || args.get_set_options().size() == 0 || unknown_option_exists) {
        std::cout << cli_options.get_options_string() << std::endl;
    }
}