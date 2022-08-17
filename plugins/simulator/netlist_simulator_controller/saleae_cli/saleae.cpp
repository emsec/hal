#include <iostream>
#include <iomanip>
#include <netlist_simulator_controller/saleae_directory.h>
#include <netlist_simulator_controller/saleae_file.h>
#include <hal_core/utilities/program_options.h>
#include <hal_core/utilities/program_arguments.h>
#include <hal_core/utilities/log.h>

using namespace hal;

// checks if a file exists 
bool file_exists (const std::string& path)
{
    if (FILE *ff = fopen(path.c_str(), "rb"))
    {
        fclose(ff);
        return true;
    }
    return false;   
}

// template for space saving printing
template<typename T> void print_element(T t, const int& width, bool align)
{
    if (align)
    {
        std::cout << std::left << std::setw(width) << std::setfill(' ') << t << " | ";
    }
    else
    {
        std::cout << std::right << std::setw(width) << std::setfill(' ') << t << " | ";
    }
}

// check, given the size, whether an entry may be printed
bool check_size(bool necessary, char op, int size_val, int compare_val)
{
    if (!necessary) return true;
    switch (op)
    {
        case '+':
            return (compare_val > size_val);
        case '-':
            return (compare_val < size_val);
        default:
            return (compare_val == size_val);
    }
}

// check, given an id list, whether an entry may be printed
bool check_ids(bool necessary, std::unordered_set<int> id_set, int id_to_check)
{
    return ((id_set.count(id_to_check)) || (!necessary));
}


std::unordered_set<int> parse_list_of_ids(std::string list_of_ids)
{
    std::unordered_set<int> id_set;
    std::stringstream ss(list_of_ids);
    std::vector<std::string> splited_ids;
    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        splited_ids.push_back(substr);
    }

    for (std::string id_entry : splited_ids)
    {
        if (id_entry.find('-') != std::string::npos)
        {
            std::stringstream range_stream(id_entry);
            std::vector<std::string> range;
            while (range_stream.good())
            {
                std::string substr;
                getline(range_stream, substr, '-');
                range.push_back(substr);
            }
            int tmp_id = std::stoi(range.front());
            while (tmp_id <= std::stoi(range.back()))
            {
                id_set.insert(tmp_id);
                tmp_id ++;
            }
        }
        else
        {
           id_set.insert(std::stoi(id_entry));
        }
    }
    return id_set;
}

int time_within_tolerance(const std::vector<uint64_t>& time_vec, int cur_time, int tolerance)
{
    // find closest time
    int closest_time;
    auto i = std::lower_bound(
        time_vec.begin(),
        time_vec.end(),
        cur_time
    );

    if (i == time_vec.begin())
    {
        closest_time = cur_time;
    }
    else
    {
        int s_time = *(i - 1); // smaller time
        int g_time = *(i); // greater time
        if (abs(cur_time - s_time) <= abs(cur_time - g_time))
        {
            closest_time = time_vec[i - time_vec.begin() - 1];
        }
        else
        {
            closest_time = time_vec[i - time_vec.begin()];
        }
    }

    // is closest time in tolerance range
    int suitable_time = -1;
    if (abs(cur_time - closest_time) <= tolerance)
    {
        suitable_time = closest_time;
    }

    return suitable_time;
}


// saleae ls-tool
void saleae_ls(std::string path, std::string size, std::string ids)
{
    // handle --dir option
    path = (path == "") ? "./saleae.json" : path + "/saleae.json";
    if (!file_exists(path))
    {
        std::cout << "Cannot open file: " << path << std::endl;
        return;
    }

    // handle --size option
    bool size_necessary = false;
    char size_op = '=';
    int size_val = 0;
    if (size != "")
    {
        size_necessary = true;
        if (size[0] == '+' || size[0] == '-')
        {
            size_op = size[0];
            size_val = std::stoi(size.substr(1));
        }
        else
        {
            size_val = std::stoi(size);
        }
    }

    // handle --id option
    bool ids_necessary = false;
    std::unordered_set<int> id_set;
    if (ids != "")
    {
        ids_necessary = true;
        id_set = parse_list_of_ids(ids);
    }

    SaleaeDirectory *sd = new SaleaeDirectory(path, false);
    std::vector<SaleaeDirectoryNetEntry> net_entries = sd->dump();

    // collect length for better formatting
    int format_length [6] = {7, 8, 19, 11, 10, 15}; // length of the column titles
    for (const SaleaeDirectoryNetEntry& sdne : net_entries)
    {
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
        {
            if (check_size(size_necessary, size_op, size_val, sdfi.numberValues()) && check_ids(ids_necessary, id_set, sdne.id()))
            {
                format_length[0] = (format_length[0] < std::to_string(sdne.id()).length()) ? std::to_string(sdne.id()).length() : format_length[0];
                format_length[1] = (format_length[1] < sdne.name().length()) ? sdne.name().length() : format_length[1];
                format_length[2] = (format_length[2] < std::to_string(sdfi.numberValues()).length()) ? std::to_string(sdfi.numberValues()).length() : format_length[2];
                format_length[3] = (format_length[3] < std::to_string(sdfi.beginTime()).length()) ? std::to_string(sdfi.beginTime()).length() : format_length[3];
                format_length[4] = (format_length[4] < std::to_string(sdfi.endTime()).length()) ? std::to_string(sdfi.endTime()).length() : format_length[4];
                format_length[5] = (format_length[5] < std::to_string(sdfi.index()).length() + 12) ? std::to_string(sdfi.index()).length() + 12: format_length[5];
            }
        }
    }
    int abs_length = format_length[0] + format_length[1] + format_length[2] + format_length[3] + format_length[4] + format_length[5] + 16;

    // print saleae-dir content
    std::cout << std::string(abs_length + 2, '-') << std::endl;
    print_element("| Net ID", format_length[0], true);
    print_element("Net Name", format_length[1], true);
    print_element("Total Number Values", format_length[2], true);
    print_element("First Event", format_length[3], true);
    print_element("Last Event", format_length[4], true);
    print_element("Binary filename", format_length[5], true);
    std::cout << std::endl;
    std::cout << '|' << std::string(abs_length, '-') << '|' << std::endl;
    for (const SaleaeDirectoryNetEntry& sdne : net_entries)
    {
        for (const SaleaeDirectoryFileIndex& sdfi : sdne.indexes())
        {
            if (check_size(size_necessary, size_op, size_val, sdfi.numberValues()) && check_ids(ids_necessary, id_set, sdne.id()))
            {
                std::cout << '|';
                print_element(sdne.id(), format_length[0], false);
                print_element(sdne.name(), format_length[1], true);
                print_element(sdfi.numberValues(), format_length[2], false);
                print_element(sdfi.beginTime(), format_length[3], false);
                print_element(sdfi.endTime(), format_length[4], false);
                print_element("digital_" + std::to_string(sdfi.index()) + ".bin", format_length[5], true);
                std::cout << std::endl;
            }
        }
    }
    std::cout << std::string(abs_length + 2, '-') << std::endl;
}


// saleae cat-tool
void saleae_cat(std::string path, std::string file_name, bool dump_header, bool dump_data)
{
    // handle --dir option
    path = (path == "") ? "./" + file_name : path + "/" + file_name;
    if (!file_exists(path))
    {
        std::cout << "Cannot open file: " << path << std::endl;
        return;
    }

    // handle --only-data and --only-header option
    if (!dump_header && !dump_data)
    {
        dump_header = true;
        dump_data = true;
    }

    SaleaeInputFile *sf = new SaleaeInputFile(path);
    uint64_t num_transitions = sf->header()->mNumTransitions;

    // dump header
    if (dump_header)
    {
        // get header content
        uint64_t begin_time = sf->header()->mBeginTime;
        int32_t start_val = sf->header()->mValue;
        std::string data_format;
        switch (sf->header()->storageFormat())
        {
        case SaleaeHeader::Double:
            data_format = "Double";
            break;
        case SaleaeHeader::Uint64:
            data_format = "Uint64";
            break;
        case SaleaeHeader::Coded:
            data_format = "Coded";
            break;
        }

        // collect length for better formatting
        int format_length [4] = {12, 10, 11, 21}; // length of the column titles
        format_length[0] = (format_length[0] < data_format.length()) ? data_format.length() : format_length[0];
        format_length[1] = (format_length[1] < std::to_string(begin_time).length()) ? std::to_string(begin_time).length() : format_length[1];
        format_length[2] = (format_length[2] < std::to_string(start_val).length()) ? std::to_string(start_val).length() : format_length[2];
        format_length[3] = (format_length[3] < std::to_string(num_transitions).length()) ? std::to_string(num_transitions).length() : format_length[3];
        int abs_length = format_length[0] + format_length[1] + format_length[2] + format_length[3] + 10;

        // print saleae-file header
        std::cout << std::string(abs_length + 2, '-') << std::endl;
        std::cout << '|';
        print_element(" Data Format", format_length[0], true);
        print_element("Start Time", format_length[1], true);
        print_element("Start Value", format_length[2], true);
        print_element("Number of Transitions", format_length[3], true);
        std::cout << std::endl;
        std::cout << '|' << std::string(abs_length, '-') << '|' << std::endl;
        std::cout << "| ";
        print_element(data_format, format_length[0] - 1, true);
        print_element(begin_time, format_length[1], false);
        print_element(start_val, format_length[2], false);
        print_element(num_transitions, format_length[3], false);
        std::cout << std::endl;
        std::cout << std::string(abs_length + 2, '-') << std::endl;
    }

    // dump data
    if (dump_data && (num_transitions > 0))
    {
        SaleaeDataBuffer *db = sf->get_buffered_data(num_transitions + 1);

        // get data
        uint64_t* time_array = db->mTimeArray;
        int* value_array = db->mValueArray;

        // collect length for better formatting
        int format_length [3] = {4, 4, 5}; // length of the column titles
        for (int i = 0; i <= num_transitions; i++)
        {
            format_length[0] = (format_length[0] < std::to_string(i).length()) ? std::to_string(i).length() : format_length[0];
            format_length[1] = (format_length[1] < std::to_string(time_array[i]).length()) ? std::to_string(time_array[i]).length() : format_length[1];
            format_length[2] = (format_length[2] < std::to_string(value_array[i]).length()) ? std::to_string(time_array[i]).length() : format_length[2];
        }
        int abs_length = format_length[0] + format_length[1] + format_length[2] + 7;

        // print saleae-file data
        std::cout << std::string(abs_length + 2, '-') << std::endl;
        std::cout << '|';
        print_element(" No.", format_length[0], true);
        print_element("Time", format_length[1], true);
        print_element("Value", format_length[2], true);
        std::cout << std::endl;
        std::cout << '|' << std::string(abs_length, '-') << '|' << std::endl;
        for (int i = 0; i <= num_transitions; i++)
        {
            std::cout << '|';
            print_element(i, format_length[0], false);
            print_element(time_array[i], format_length[1], false);
            print_element(value_array[i], format_length[2], false);
            std::cout << std::endl;
        }
        std::cout << std::string(abs_length + 2, '-') << std::endl;
    }
}


// saleae diff-tool
void saleae_diff(std::string path_1, std::string path_2, std::string ids, bool only_diff, int tolerance)
{
    // handle --dir option
    std::string path_1_json = (path_1 == "") ? "./saleae.json" : path_1 + "/saleae.json";
    if (!file_exists(path_1_json))
    {
        std::cout << "Cannot open file: " << path_1_json << std::endl;
        return;
    }
    std::string path_2_json = path_2 + "/saleae.json";
    if (!file_exists(path_2_json))
    {
        std::cout << "Cannot open file: " << path_2_json << std::endl;
        return;
    }

    // handle --id option
    bool ids_necessary = false;
    std::unordered_set<int> id_set;
    if (ids != "")
    {
        ids_necessary = true;
        id_set = parse_list_of_ids(ids);
    }

    struct row_t
    {
        int alt_time;
        int val_1;
        bool val_1_avail;
        int val_2;
        bool val_2_avail;
        bool diff;
    };

    struct net_t
    {
        int id;
        std::string name_1;
        std::string name_2;
        bool name_diff;
        bool data_diff;
        std::map<uint64_t, row_t> net_data;
        int format_length[4];
    };

    std::vector<int> ids_not_in_2;
    std::vector<int> ids_not_in_1;
    std::vector<net_t> diff_vec;
    bool diff_found = false;

    SaleaeDirectory *sd_1 = new SaleaeDirectory(path_1_json, false);
    std::vector<SaleaeDirectoryNetEntry> net_entries_1 = sd_1->dump();
    SaleaeDirectory *sd_2 = new SaleaeDirectory(path_2_json, false);
    std::vector<SaleaeDirectoryNetEntry> net_entries_2 = sd_2->dump();
    for (const SaleaeDirectoryNetEntry& sdne_1 : net_entries_1)
    {
        if (!check_ids(ids_necessary, id_set, sdne_1.id()))
        {
            continue;
        }
        bool id_found = false;
        for (const SaleaeDirectoryNetEntry& sdne_2 : net_entries_2)
        {
            if (sdne_1.id() != sdne_2.id())
            {
                continue;
            }
            id_found = true;

            // save net info
            struct net_t cur_net;
            cur_net.id = sdne_1.id();
            cur_net.name_1 = sdne_1.name();
            cur_net.name_2 = sdne_2.name();

            // format len
            cur_net.format_length[0] = 2;
            cur_net.format_length[1] = 4;
            cur_net.format_length[2] = cur_net.name_1.length();
            cur_net.format_length[3] = cur_net.name_2.length();

            // name diff?
            cur_net.name_diff = cur_net.name_1 != cur_net.name_2;

            // compare data
            int diff_cnt = 0;
            std::map<uint64_t, row_t> net_data; // key=time, value=row struct
            std::vector<uint64_t> time_vec;
            for (const SaleaeDirectoryFileIndex& sdfi : sdne_1.indexes())
            {
                path_1 = (path_1 == "") ? "." : path_1;
                std::string bin_path = path_1 + "/digital_" + std::to_string(sdfi.index()) + ".bin";
                if (!file_exists(bin_path))
                {
                    std::cout << "Error in database: " << path_1 << "\nCannot open file: " << bin_path << std::endl;
                    return;
                }
                SaleaeInputFile *sf = new SaleaeInputFile(bin_path);
                SaleaeDataBuffer *db = sf->get_buffered_data(sf->header()->mNumTransitions);
                // save first net_data times in map
                for (int i = 0; i < db->mCount; i++)
                {
                    uint64_t t = db->mTimeArray[i];
                    time_vec.push_back(t);
                    net_data[t] = row_t{.val_1 = db->mValueArray[i], .val_1_avail = true, .val_2_avail = false, .diff = true};
                    diff_cnt++;
                    // update format len
                    cur_net.format_length[1] = (cur_net.format_length[1] < std::to_string(t).length()) ? std::to_string(t).length() : cur_net.format_length[1];
                    cur_net.format_length[2] = (cur_net.format_length[2] < std::to_string(net_data[t].val_1).length()) ? std::to_string(net_data[t].val_1).length() : cur_net.format_length[2];
                }
            }

            for (const SaleaeDirectoryFileIndex& sdfi : sdne_2.indexes())
            {
                std::string bin_path = path_2 + "/digital_" + std::to_string(sdfi.index()) + ".bin";
                if (!file_exists(bin_path))
                {
                    std::cout << "Error in database: " << path_2 << "\nCannot open file: " << bin_path << std::endl;
                    return;
                }
                SaleaeInputFile *sf = new SaleaeInputFile(bin_path);
                SaleaeDataBuffer *db = sf->get_buffered_data(sf->header()->mNumTransitions);

                // save second net_data times in map
                for (int i = 0; i < db->mCount; i++) 
                {
                    uint64_t t = db->mTimeArray[i];
                    // is there a timestamp within tolerance range?
                    int twt = time_within_tolerance(time_vec, t, tolerance);
                    if (twt >= 0)
                    {
                        net_data[twt].alt_time = t;
                        net_data[twt].val_2 = db->mValueArray[i];
                        net_data[twt].val_2_avail = true;
                        net_data[twt].diff = (net_data[twt].val_1 != net_data[twt].val_2);
                        diff_cnt = net_data[twt].diff ? diff_cnt : diff_cnt - 1;
                        // update format len
                        cur_net.format_length[3] = (cur_net.format_length[3] < std::to_string(net_data[twt].val_2).length()) ? std::to_string(net_data[twt].val_2).length() : cur_net.format_length[3];
                    }
                    else
                    {
                        net_data[t] = row_t{.val_1_avail = false, .val_2 = db->mValueArray[i], .val_2_avail = true, .diff = true};
                        diff_cnt++;
                        // update format len
                        cur_net.format_length[1] = (cur_net.format_length[1] < std::to_string(t).length()) ? std::to_string(t).length() : cur_net.format_length[1];
                        cur_net.format_length[3] = (cur_net.format_length[3] < std::to_string(net_data[t].val_2).length()) ? std::to_string(net_data[t].val_2).length() : cur_net.format_length[3];
                    }
                }
                // data diff?
                cur_net.data_diff = diff_cnt > 0;
                if (cur_net.data_diff)
                {
                    cur_net.net_data = net_data;
                }
                // save net struct if there is a difference
                if (cur_net.data_diff || cur_net.name_diff)
                {
                    diff_vec.push_back(cur_net);
                    diff_found = true;
                }
            }

        }
        if (!id_found)
        {
            ids_not_in_2.push_back(sdne_1.id());
            diff_found = true;
        }
    }
    for (const SaleaeDirectoryNetEntry& sdne_2 : net_entries_2)
    {
        if (!check_ids(ids_necessary, id_set, sdne_2.id()))
        {
            continue;
        }
        bool id_found = false;
        for (const SaleaeDirectoryNetEntry& sdne_1 : net_entries_1)
        {
            if (sdne_2.id() != sdne_1.id())
            {
                continue;
            }
            id_found = true;
        }
        if (!id_found)
        {
            ids_not_in_1.push_back(sdne_2.id());
            diff_found = true;
        }
    }

    // output
    if (!diff_found) {
        std::cout << "- Content of waveform database is the same" << std::endl;
        exit (0);
    }
    std::cout << "=> Database 1: " << path_1 << "\n=> Database 2: " << path_2 << "\n\n" << std::endl;
    // id not found
    for (int id : ids_not_in_2)
    {
        std::cout << "- Waveform ID " << id << " found in database 1 but not in database 2\n" << std::endl;
    }
    for (int id : ids_not_in_1)
    {
        std::cout << "- Waveform ID " << id << " found in database 2 but not in database 1\n" << std::endl;
    }
    for (net_t cur_net : diff_vec)
    {
        if (cur_net.name_diff && !cur_net.data_diff)
        {
            // only diffrent name
            std::cout << "- Waveform ID " << cur_net.id << " is named \"" << cur_net.name_1 << "\" in database 1 but \"" << cur_net.name_2 << "\" in database 2\n" << std::endl;
        }
        else
        {
            // diffrent data
            // header row
            std::cout << "\n- Waveform ID " << cur_net.id << " has a data difference" << std::endl;
            int abs_length = cur_net.format_length[0] + cur_net.format_length[1] + cur_net.format_length[2] + cur_net.format_length[3] + 10;
            std::cout << std::string(abs_length + 2, '-') << std::endl;
            std::string diff_char = cur_net.name_diff ? "*" : " ";

            print_element("| " + diff_char, cur_net.format_length[0], true);
            print_element("Time", cur_net.format_length[1], true);
            print_element(cur_net.name_1, cur_net.format_length[2], true);
            print_element(cur_net.name_2, cur_net.format_length[3], true);
            std::cout << std::endl;
            std::cout << '|' << std::string(abs_length, '-') << '|' << std::endl;

            // data rows
            for (auto &item : cur_net.net_data)
            {
                struct row_t cur_row = item.second;
                // handle --only-differences option
                if (only_diff && !cur_row.diff)
                {
                    continue;
                }
                std::string diff_char = cur_row.diff ? "*" : " ";
                std::string val_1 = cur_row.val_1_avail ? std::to_string(cur_row.val_1) : "-";
                std::string val_2 = cur_row.val_2_avail ? std::to_string(cur_row.val_2) : "-";
                int main_time = item.first;
                int alt_time = (cur_row.val_1_avail && cur_row.val_2_avail) ? cur_row.alt_time : main_time;

                std::cout << '|';
                print_element(diff_char, cur_net.format_length[0], true);

                if (alt_time < main_time)
                {
                    print_element(alt_time, cur_net.format_length[1], false);
                }
                else
                {
                    print_element(main_time, cur_net.format_length[1], false);
                }


                print_element(val_1, cur_net.format_length[2], true);
                print_element(val_2, cur_net.format_length[3], true);
                std::cout << std::endl;
            }
            std::cout << std::string(abs_length + 2, '-') << std::endl;
            std::cout << "\n" << std::endl;
        }
    }
    exit (1);
}


int main(int argc, const char* argv[])
{
    std::cout << std::endl;
    // initialize logging
    LogManager& lm = LogManager::get_instance();
    lm.add_channel("core", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.deactivate_all_channels();

    // initialize and parse options
    ProgramOptions generic_options("generic options");
    generic_options.add("--help", "print help messages");

    ProgramOptions tool_options("tool options");
    tool_options.add("ls", "Lists content of saleae directory file saleae.json");
    tool_options.add("cat", "Dump content of binary file into console", {""});
    tool_options.add("diff", "Compares content of database in current saleae directory with other saleae database", {""});

    ProgramOptions ls_options("ls options");
    ls_options.add({"-d", "--dir"}, "lists saleae directory from directory given by absolute or relative path name", {ProgramOptions::A_REQUIRED_PARAMETER});
    ls_options.add({"-s", "--size"}, "lists only entries with given number of waveform events", {ProgramOptions::A_REQUIRED_PARAMETER});
    ls_options.add({"-i", "--id"}, "list only entries where ID matches entry in list. Entries are separated by comma. A single entry can be either an ID or a range sepearated by hyphen", {ProgramOptions::A_REQUIRED_PARAMETER});

    ProgramOptions cat_options("cat options");
    cat_options.add({"-d", "--dir"}, "binary file is not in current directory but in directory given by path name", {ProgramOptions::A_REQUIRED_PARAMETER});
    cat_options.add({"-h", "--only-header"}, "dump only header");
    cat_options.add({"-b", "--only-data"}, "dump only data including start value");

    ProgramOptions diff_options("diff options");
    diff_options.add({"-d", "--dir"}, "current database not in current directory but in directory given by path name", {ProgramOptions::A_REQUIRED_PARAMETER});
    diff_options.add({"-i", "--id"}, "compares only entries where ID matches entry in list", {ProgramOptions::A_REQUIRED_PARAMETER});
    diff_options.add({"-x", "--only-differences"}, "when dumping waveform data values all rows without differences are suppressed (except header row)");
    diff_options.add({"-t", "--max-tolerance"}, "the integer value sets the maximum tolerance when comparing waveform data. On default (zero tolerance) two waveforms A,B with transition values A=[0,12000,16000] B=[0,12010,16010] are considered to be different. However, when tolerance is set to 10 or higher the comparison will not find any differences", {ProgramOptions::A_REQUIRED_PARAMETER});


    ProgramArguments args = tool_options.parse(argc, argv);

    if (args.is_option_set("ls"))
    {
        ls_options.add(generic_options);
        ProgramArguments args = ls_options.parse(argc, argv);

        bool unknown_option_exists = false;
        for (std::string opt : ls_options.get_unknown_arguments())
        {
            unknown_option_exists = (opt != "ls") ? true : unknown_option_exists;     
        }

        if (args.is_option_set("--help") || unknown_option_exists)
        {
            std::cout << ls_options.get_options_string() << std::endl;
        }
        else
        {
            saleae_ls(args.get_parameter("--dir"), args.get_parameter("--size"), args.get_parameter("--id"));
        }
    }
    else if (args.is_option_set("cat"))
    {
        std::string filename = args.get_parameter("cat");
        cat_options.add(generic_options);
        ProgramArguments args = cat_options.parse(argc, argv);

        bool unknown_option_exists = false;
        for (std::string opt : cat_options.get_unknown_arguments())
        {
            unknown_option_exists = ((opt != "cat") && (opt != filename)) ? true : unknown_option_exists;
        }
        
        if (filename == "")
        {
            std::cout << tool_options.get_options_string();
            std::cout << ls_options.get_options_string();
            std::cout << cat_options.get_options_string();
            std::cout << diff_options.get_options_string() << std::endl;
        }
        else if (args.is_option_set("--help") || unknown_option_exists)
        {
            std::cout << cat_options.get_options_string() << std::endl;
        }
        else
        {
            saleae_cat(args.get_parameter("--dir"), filename, args.is_option_set("--only-header"), args.is_option_set("--only-data"));
        }
    }
    else if (args.is_option_set("diff"))
    {
        std::string diff_path = args.get_parameter("diff");
        diff_options.add(generic_options);
        ProgramArguments args = diff_options.parse(argc, argv);

        bool unknown_option_exists = false;
        for (std::string opt : diff_options.get_unknown_arguments())
        {
            unknown_option_exists = ((opt != "diff") && (opt != diff_path)) ? true : unknown_option_exists;
        }
        int tolerance = 0;
        if (args.is_option_set("--max-tolerance"))
        {
            tolerance = std::stoi(args.get_parameter("--max-tolerance"));
            if (tolerance < 0)
            {
                unknown_option_exists = true;
            }
        }
        if (diff_path == "")
        {
            std::cout << tool_options.get_options_string();
            std::cout << ls_options.get_options_string();
            std::cout << cat_options.get_options_string();
            std::cout << diff_options.get_options_string() << std::endl;
        }
        else if (args.is_option_set("--help") || unknown_option_exists)
        {
            std::cout << diff_options.get_options_string() << std::endl;
        }
        else
        {

            saleae_diff(args.get_parameter("--dir"), diff_path, args.get_parameter("--id"), args.is_option_set("--only-differences"), tolerance);
        }
    }
    else
    {
        tool_options.add(generic_options);

        std::cout << tool_options.get_options_string();
        std::cout << ls_options.get_options_string();
        std::cout << cat_options.get_options_string();
        std::cout << diff_options.get_options_string() << std::endl;
    }
}
