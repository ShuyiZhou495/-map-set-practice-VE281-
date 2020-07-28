#include <iostream>
#include "simulation.h"
#include <getopt.h>
#include <set>

using namespace std;

const char* const short_options="v:m:p:t:g";
const struct option long_options[]={
        {"verbose", 0, nullptr, 'v'},
        {"median", 0, nullptr, 'm'},
        {"midpoint", 0, nullptr, 'p'},
        {"transfers", 0, nullptr, 't'},
        {"ttt", 1, nullptr, 'g'},
        {nullptr, 0, nullptr, 0}
};

int main(int argc, char *argv[]) {
    bool verbose= false, median= false, midpoint= false, transfers= false, ttt= false;
    int ttt_size = 0;
    set<string> ttt_name;

    int next_option;
    do{
        next_option = getopt_long(argc, argv, short_options, long_options, nullptr);
        switch (next_option){
            case 'v':
                verbose = true;
                break;
            case 'm':
                median = true;
                break;
            case 'p':
                midpoint = true;
                break;
            case 't':
                transfers = true;
                break;
            case 'g': {
                ttt = true;
                ttt_size++;
                ttt_name.insert(optarg);
            }
            case -1:
                break;
            default:
                abort();
        }
    }while(next_option!=-1);

    simulation today_order = simulation(verbose, median, midpoint, transfers, ttt, ttt_size, ttt_name);
    today_order.run();
//    today_order.try_median();

    return 0;
}