#include <iostream>
#include "simulation.h"
#include <fstream>

using namespace std;

int main() {
    bool verbose= true, median= true, midpoint= true, transfers= true, ttt= true;
    int ttt_size = 3;
    string ttt_name[3] = {"AMD", "GE", "AMZN"};
    simulation today_order = simulation(verbose, median, midpoint, transfers, ttt, ttt_size, ttt_name);
    ifstream file;
    file.open("../test.txt");
    today_order.test(file);
    file.close();
//    today_order.try_median();

    return 0;
}