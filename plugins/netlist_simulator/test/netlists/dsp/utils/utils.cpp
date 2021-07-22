#include "utils.h"
#include <string.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int get_number_of_lines(std::ifstream& file)
{
    int lines = 0;
    std::string line;

    while (std::getline(file, line))
        ++lines;

    return lines;
}

std::string get_number_of_lines_in_hex(std::ifstream& file)
{
    int lines = get_number_of_lines(file);

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(8) << std::hex << lines;

    return stream.str();
}

std::string hex_to_bin(std::string& input_string)
{
    std::stringstream ss;
    ss << std::hex << input_string;
    unsigned n;
    ss >> n;
    std::bitset<32> b(n);

    return b.to_string();
}
