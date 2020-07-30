#include <iostream>
#include "simulation.h"
#include <getopt.h>
#include <queue>

using namespace std;

const char* const short_options="vmptg:";
static struct option long_options[]{
        {"verbose", no_argument, NULL, 'v'},
        {"median", no_argument, NULL, 'm'},
        {"midpoint", no_argument, NULL, 'p'},
        {"transfers", no_argument, NULL, 't'},
        {"ttt", required_argument, NULL, 'g'},
        {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    bool verbose= false, median= false, midpoint= false, transfers= false, ttt= false;
    int ttt_size = 0;
    queue<string> ttt_name;

//    bool verbose= true, median= true, midpoint= true, transfers= true, ttt= true;
//    int ttt_size = 1;
//    queue<string> ttt_name;
//    ttt_name.push("AMZN");


    int next_option;
    while((next_option = getopt_long(argc, argv, short_options, long_options, nullptr))!=-1){
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
                ttt_name.push(optarg);
            }
            default:
                break;
        }
    }

    simulation today_order = simulation(verbose, median, midpoint, transfers, ttt, ttt_size, ttt_name);
//    ifstream file;
//    file.open("../test2.txt");
//    today_order.test(file);
//    file.close();
    today_order.run();
//    today_order.try_median();

    return 0;
}