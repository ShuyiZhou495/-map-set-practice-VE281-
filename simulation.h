//
// Created by 周舒意 on 2020/7/26.
//

#ifndef CODING4_SIMULATION_H
#define CODING4_SIMULATION_H

#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <fstream>
#include <utility>
#include <set>
#include <sstream>


using namespace std;

struct info{
    string client;
    int price, quantity, arrive_time, ID;
    info(string client, int price, int quantity, int arrive_time, int ID):
            client(move(client)), price(price), quantity(quantity), arrive_time(arrive_time), ID(ID){}
    info():client(""), price(-1), quantity(0), arrive_time(-1), ID(-1){}
};
template<class compare_f = std::less<int>>
class cmp{
public:
    bool operator()(const info& info1, const info& info2) const{
        compare_f f;
        if(info1.price!=info2.price) return f(info1.price, info2.price);
        else return info1.ID<info2.ID;
    };
};
class simulation {
private:
// for buy orders'priority queue, the higher price, the more prior, so, it uses std::greater
// for sell orders'priority queue, the lower price, the more prior, so, it uses std::less
// For the same price, the ealier it came, the more prior.

    int time=0, orders=0, ttt_size=0;
    unsigned int cmsn_earning=0, total_transfered=0, num_trades=0, num_shares=0;
    bool verbose, median, midpoint, transfers, ttt;
    string * ttt_name_s;
    set<string> ttt_name;
    map<string, map<int, struct info>> ttt_sell_info, ttt_buy_info;
    map<string, map<int, multiset<info, cmp<greater<int>>>>> buy_equity;
    // a map from equity name to information(buy).
    map<string, map<int, multiset<info, cmp<less<int>>>>> sell_equity;
    // a map from equity name to information(sell).
    map<string, multiset<int>> traded_equites;
    map<string, multiset<int>::iterator> median_its;
    // for median
    set<string> active_equity;
    map<string, int> active_buy_max;
    map<string, int> active_sell_min;
    // active orders' min sell price and max buy price, {"euity_name": <<expire_data, info_min_sell>, <<expire_data, info_max_buy>>>}
    // clients' recored
    set<string> client_names;
    map<string, int> client_transfers, client_buy, client_sell;

public:
    simulation(bool verbose, bool median, bool midpoint, bool transfers, bool ttt, int ttt_size, queue<string> & ttt_name):
         ttt_size(ttt_size), verbose(verbose), median(median), midpoint(midpoint), transfers(transfers), ttt(ttt){
        if(ttt) {
            this->ttt_name_s = new string[ttt_size];
            int i = 0;
            while (!ttt_name.empty()) {
                this->ttt_name_s[i] = ttt_name.front();
                this->ttt_name.insert(ttt_name.front());
                this->ttt_buy_info[ttt_name.front()] = map<int, info>();
                this->ttt_sell_info[ttt_name.front()] = map<int, info>();
                ttt_name.pop();
                i++;
            }
        }
    };
    ~simulation(){
        if(ttt) delete [] ttt_name_s;
    }
    void get_median();

    void add_traded_price(const string & equity, int price);

    void get_midpoint();

    template<class compare_f = std::less<int>>
    void add_order(string e_name, string client, int price, int last, int quantity,
            map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
            map<string, int> & active_min_max,
            map<string, map<int,info>> & ttt_info);

    void update_ttt(const string & e_name, info order, map<string, map<int, info>> & ttt_info);


    void remove_expire();

    template<class compare_f = std::less<int>>
    void get_top(const string & e_name, map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                 map<string,int> & active_min_max);

    template<class compare_f = std::less<int>>
    void remove_expire_help(map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                            map<string, int> & active_min_max);

    void test_add_client(const string& name);

    void add_for_total(bool buy_or_sell, const string & clientBuy, const string & clientSell, int price, int quantity);

    template<class compare_f = std::less<int>>
    int match_order(bool buy_or_sell, const string& e_name, const string& client, int price, int last, int quantity,
                     map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                     map<string, int> & active_min_max);

    void print_transfers();

    void print_ttt();

    void run() ;
};
#endif //CODING4_SIMULATION_H
