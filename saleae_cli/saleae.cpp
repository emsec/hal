#include <iostream>
#include <netlist_simulator_controller/saleae_directory.h>

using namespace std;

int main() {
    hal::SaleaeDirectory *sd = new hal::SaleaeDirectory("/home/parallels/Desktop/saleae/saleae.json", false);
    sd->dump();
}