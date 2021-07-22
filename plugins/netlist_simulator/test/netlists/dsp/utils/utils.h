#ifndef UTILS_H
#define UTILS_H


#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int get_number_of_lines(std::ifstream& file);
std::string get_number_of_lines_in_hex(std::ifstream& file);
std::string hex_to_bin(std::string& input_string);

#endif /* UTILS_H */