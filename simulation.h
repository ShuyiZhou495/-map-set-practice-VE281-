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

    int time=0, orders=0;
    unsigned int cmsn_earning=0, total_transfered=0, num_trades=0, num_shares=0;
    bool verbose, median, midpoint, transfers, ttt;
    int ttt_size=0;
    string * ttt_name_s;
    set<string> ttt_name;
    map<string, map<int, struct info>> ttt_sell_info;
    map<string, map<int, struct info>> ttt_buy_info; // first is buy, second is sell
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
    verbose(verbose), median(median), midpoint(midpoint), transfers(transfers), ttt(ttt), ttt_size(ttt_size){
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
    void get_median(){
        for(auto & traded_equite : traded_equites){
            int median;
            if(traded_equite.second.size()%2!=0) median = *median_its[traded_equite.first];
            else{
                median = *(median_its[traded_equite.first]++);
                median += *(median_its[traded_equite.first]--);
                median /= 2;
            }
            cout<<"Median match price of "<<traded_equite.first<<" at time "<<time<<" is $"<<median<<endl;
        }
    }
    void add_traded_price(const string & equity, int price){
        if(traded_equites.find(equity)==traded_equites.end()){
            traded_equites[equity] = multiset<int>();
            traded_equites[equity].insert(price);
            median_its[equity] = traded_equites[equity].begin();
        }
        else{
            int current_median = *median_its[equity], current_size = traded_equites[equity].size();
            bool even = (current_size%2==0);
            traded_equites[equity].insert(price);
            if(price<current_median){
                // if originally 2 elements, iterator is at the first, iterator doesn't change.
                // if originally 1 element, iterator change to the first
                if(!even)  median_its[equity]--;
            }
            else {
                // if originally 1 elements, iterator is at the first, iterator doesn't change.
                // if originally 2 element, iterator change to the second
                if(even) median_its[equity]++;
            }
        }
    }

    // delete
    void try_median(){
        add_traded_price("B", 15);
        add_traded_price("A", 2);
        add_traded_price("A", 16);
        add_traded_price("C", 1);
        add_traded_price("A", 14);
        add_traded_price("A", 20);
        add_traded_price("A", 2);
        add_traded_price("D", 2);
        add_traded_price("A", 2);
        add_traded_price("B", 2);
        get_median();
        for(auto it = traded_equites["A"].begin(); it!=traded_equites["A"].end();it++)cout<<*it<<endl;

    }

    // delete
    template <class compare_f>
    void print(const string& buy, const map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
               const map<string, int> & active_min_max,
               const map<string, info> & ttt_info){
        cout<<buy<<endl;
        for(auto it=equity.begin(); it!=equity.end();it++){ // every equity
            for(auto it2 = it->second.begin();it2!=it->second.end();it2++){ // every timestamp
                for(auto it3 = it2->second.begin(); it3!=it2->second.end();it3++)
                    cout<<"Equity: "<<it->first<<" time: "<<it2->first<<" price: "<<it3->price<<endl;
            }
        }
        cout<<"active_min_max"<<endl;
        for(auto it = active_min_max.begin(); it!=active_min_max.end(); it++) {
            cout << "the " << buy << " price of " << it->first << " is "
                 << equity.find(it->first)->second.find(it->second)->second.begin()->price << " at time " << it->second
                 << endl;
        }
        cout<<"ttt_info"<<endl;
        for(auto it = ttt_info.begin();it!=ttt_info.end();it++)
            cout<<"best "<<buy<<" time of "<<it->first<<" is "<<it->second.arrive_time<<endl;

    }

    void get_midpoint(){
        for(auto it=active_equity.begin();it!=active_equity.end();it++)
            if(active_sell_min.count(*it)==0 || active_buy_max.count(*it)==0) cout<<"Midpoint of "<<*it<<" at time "<<time<<" is undefined"<<endl;
                // if there is no value, place holder is -1
            else cout<<"Midpoint of "<<*it<<" at time "<<time<<" is $"<<
            (buy_equity[*it][active_buy_max[*it]].begin()->price
            +sell_equity[*it][active_sell_min[*it]].begin()->price)/2<<endl;
    }

    template<class compare_f = std::less<int>>
    void add_order(bool buy_or_sell, string e_name, string client, int price, int last, int quantity,
            map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
            map<string, int> & active_min_max,
            map<string, map<int,info>> & ttt_info){
        int expire = last==-1?-1:this-> time+last;
        cmp<compare_f> Compare;
        // if it is not IOC
        if(last != 0){
            // insert to all the orders
            if(equity[e_name].count(expire)==0) equity[e_name][expire] = multiset<info, cmp<compare_f>>();
            auto it = equity[e_name][expire].insert(info(std::move(client), price, quantity, this->time, this->orders));

            // update the active min and max
            if(active_min_max.count(e_name)==0) {
                active_min_max[e_name] = expire;
                if (active_equity.count(e_name) == 0) active_equity.insert(e_name);
            }
            else if (Compare(*it, *equity[e_name][active_min_max[e_name]].begin())) active_min_max[e_name] = expire;

            // update ttt
            update_ttt(e_name, *it, ttt_info);
        }
        else{
            // IOC
            // update the active min and max
            if (active_equity.count(e_name) == 0) active_equity.insert(e_name);
            // update ttt
            info temp = info(std::move(client), price, quantity, this->time, this->orders);
            update_ttt(e_name, temp, ttt_info);
//            if (ttt&&this->ttt_name.count(e_name)  && (ttt_info[e_name].ID == -1 || Compare(temp, ttt_info[e_name])))
//                ttt_info[e_name] = temp;
        }
    }

    void update_ttt(const string & e_name, info order, map<string, map<int, info>> & ttt_info){
        if(ttt && ttt_name.count(e_name)!=0){
            ttt_info[e_name][order.ID] = order;
        }
    }


    void remove_expire(){
        // equity
        remove_expire_help<greater<int>>(buy_equity, active_buy_max);
        remove_expire_help<less<int>>(sell_equity, active_sell_min);
    }

    template<class compare_f = std::less<int>>
    void get_top(const string & e_name, map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                 map<string,int> & active_min_max){
        if(equity[e_name].empty()) active_min_max.erase(e_name);
        else {
            int time = equity[e_name].begin()->first;
            cmp<compare_f> Compare;
            for (auto it = equity[e_name].begin(); it != equity[e_name].end(); it++) {
                if(Compare(*(it->second.begin()), *equity[e_name][time].begin())) time = it->first;
            }
            active_min_max[e_name] = time;
        }

    }

    template<class compare_f = std::less<int>>
    void remove_expire_help(map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                            map<string, int> & active_min_max){
        for (auto it = equity.begin();it!=equity.end();it++){
            // equity
            auto it_now = it->second.upper_bound(time);
            auto it_forever = it->second.upper_bound(-1);
            it->second.erase(it_forever, it_now);

            // min_max
            if(active_min_max[it->first]<=time&&active_min_max[it->first]>=0) get_top<compare_f>(it->first, equity, active_min_max);
        }
    }

    void test_add_client(const string& name){
        if(client_names.count(name)==0){
            client_names.insert(name);
            client_transfers[name] = 0;
            client_buy[name] = 0;
            client_sell[name] = 0;
        }
    }

    void add_for_total(bool buy_or_sell, const string & clientBuy, const string & clientSell, int price, int quantity){
        // add commision total earning, total money transfered, completed trades, completed shares, change client_buy/sell
        int com_earn = price*quantity/100;
        this->cmsn_earning += com_earn * 2;
        this->total_transfered += price * quantity;
        this->num_trades ++;
        this->num_shares += quantity;
        if(transfers) {
            if(buy_or_sell) {//buy
                this->client_buy[clientBuy] += quantity;
                this->client_sell[clientSell] += quantity;
                this->client_transfers[clientBuy] -= price * quantity;
                this->client_transfers[clientSell] += price * quantity;
            }
            else{
                this->client_sell[clientBuy] += quantity;
                this->client_buy[clientSell] += quantity;
                this->client_transfers[clientBuy] += price * quantity;
                this->client_transfers[clientSell] -= price * quantity;
            }
        }
    }

    template<class compare_f = std::less<int>>
    int match_order(bool buy_or_sell, const string& e_name, const string& client, int price, int last, int quantity,
                     map<string, map<int, multiset<info, cmp<compare_f>>>> & equity,
                     map<string, int> & active_min_max){
        // return left amount of the item
        if(active_min_max.count(e_name)==0) return quantity; // no active
        else{
            cmp<compare_f> Compare; // for buy order, compare is less, input are the sell blabla
            info temp = info(client, price, quantity, time, orders);
            auto temp_it = equity[e_name][active_min_max[e_name]].begin();
            while(active_min_max.count(e_name)!=0 && temp.quantity>=temp_it->quantity) {
                // for buy, the highest buy price is lower than the lowest sell price
                if (Compare(temp, *temp_it)) return temp.quantity;
                // fullfill
                add_traded_price(e_name, temp_it->price);
                if(verbose) {
                    if (buy_or_sell) {//true is buy
                        cout << client << " purchased " << temp_it->quantity
                             << " shares of " << e_name << " from " << temp_it->client
                             << " for $" << temp_it->price << "/share" << endl;
                    } else {
                        cout << temp_it->client << " purchased " << temp_it->quantity
                             << " shares of " << e_name << " from " << client
                             << " for $" << temp_it->price << "/share" << endl;
                    }
                }
                add_for_total(buy_or_sell, client, temp_it->client, temp_it->price, temp_it->quantity);
                temp.quantity -= temp_it->quantity;
                equity[e_name][active_min_max[e_name]].erase(temp_it);
                if(equity[e_name][active_min_max[e_name]].empty()) equity[e_name].erase(active_min_max[e_name]);
                get_top<compare_f>(e_name, equity, active_min_max);

                if(active_min_max.count(e_name)==0) return temp.quantity;
                else {
//                    cout<<active_min_max[e_name]<<endl;
                    temp_it = equity[e_name][active_min_max[e_name]].begin();
//                    cout<<active_min_max[e_name]<<" "<<temp_it->price<<endl;
                }
            }
            if(temp.quantity==0) return 0;
            if (Compare(temp, *temp_it)) return temp.quantity;
            // fullfill
            add_traded_price(e_name, temp_it->price);
            add_for_total(buy_or_sell, client, temp_it->client, temp_it->price, temp.quantity);
            // adjust the quantity
            info add_again = *temp_it;
            equity[e_name][active_min_max[e_name]].erase(temp_it);
            add_again.quantity -= temp.quantity;
            temp_it = equity[e_name][active_min_max[e_name]].insert(add_again);

            if(verbose) {
                if (buy_or_sell) {//true is buy
                    cout << client << " purchased " << temp.quantity
                         << " shares of " << e_name << " from " << temp_it->client
                         << " for $" << temp_it->price << "/share" << endl;
                } else {
                    cout << temp_it->client << " purchased " << temp.quantity
                         << " shares of " << e_name << " from " << client
                         << " for $" << temp_it->price << "/share" << endl;
                }
            }

            return 0;
        }
    }

    void print_transfers(){
        for(auto it = client_names.begin(); it != client_names.end(); it++){
            cout<<*it<<" bought "<<client_buy[*it]<<" and sold "<<client_sell[*it]
                <<" for a net transfer of $"<<client_transfers[*it]<<endl;
        }
    }

    void print_ttt(){
        for(int i = 0; i < ttt_size; i++){
            string tttName = ttt_name_s[i];
            if(ttt_sell_info[tttName].empty()) continue;
            else if(ttt_buy_info[tttName].empty()) {
                cout << "Time travelers would buy " << tttName << " at time: -1 and sell it at time: -1" << endl;
                continue;
            }
            else {
                int buy_time = -1, sell_time = -1;
                int max_earn = 0;
                bool valid = false;
                for (auto it = ttt_sell_info[tttName].begin(); it != ttt_sell_info[tttName].end(); it++) {
                    for(auto it2 = ttt_buy_info[tttName].upper_bound(it->first); it2 != ttt_buy_info[tttName].end(); it2++){
                        if(!valid || (valid && (it2->second.price - it->second.price)>max_earn)){
                            valid = true;
                            max_earn = it2->second.price - it->second.price;
                            sell_time = it2->second.arrive_time;
                            buy_time = it->second.arrive_time;
                        }
                    }
                }
                cout << "Time travelers would buy " << tttName << " at time: "
                     << buy_time << " and sell it at time: " << sell_time << endl;
            }
        }
    }

    void run() {
        int timestamp, price, amount, last;
        string temp;
        string name, buy, ename,sprice, samount;
//        for(int i = 0; i<16;i++){
        while(!cin.eof()) {
            getline(cin, temp);
            if(!temp.empty()) {
                stringstream ss;
                ss<<temp;
                ss >> timestamp >> name >> buy >> ename >> sprice >> samount >> last;
                price = stoi(sprice.substr(1));
                amount = stoi(samount.substr(1));
                test_add_client(name);
//            cout<<timestamp<<endl;
                if (timestamp != time) {
                    if (this->median) get_median();
                    if (this->midpoint) get_midpoint();
                    this->time = timestamp;
                    remove_expire();
//                cout<<time<<endl;
                }
                if (buy == "BUY") {
                    int left = match_order<less<int>>(true, ename, name, price, last, amount, sell_equity,
                                                      active_sell_min);
                    if (left == 0)
                        add_order<greater<int>>(true, ename, name, price, 0, amount, this->buy_equity,
                                                this->active_buy_max, this->ttt_buy_info);
                    else
                        add_order<greater<int>>(true, ename, name, price, last, left, this->buy_equity,
                                                this->active_buy_max, this->ttt_buy_info);
                } else {
                    int left = match_order<greater<int>>(false, ename, name, price, last, amount, buy_equity,
                                                         active_buy_max);
                    if (left == 0)
                        add_order<less<int>>(false, ename, name, price, 0, amount, this->sell_equity,
                                             this->active_sell_min, this->ttt_sell_info);
                    else
                        add_order<less<int>>(false, ename, name, price, last, left, this->sell_equity,
                                             this->active_sell_min, this->ttt_sell_info);
                }
                orders++;
            }
        }
        if (this->median) get_median();
        if (this->midpoint) get_midpoint();
//        if(buy=="BUY") add_order(ename, name, price, last, amount, this->buy_equity,
//                                 this->active_buy_max, this->ttt_buy_info);
//        else  add_order(ename, name, price, last, amount, this->sell_equity,
//                        this->active_sell_min, this->ttt_sell_info);
        cout<<"---End of Day---"<<endl
            <<"Commission Earnings: $"<<cmsn_earning<<endl
            <<"Total Amount of Money Transferred: $"<<total_transfered<<endl
            <<"Number of Completed Trades: "<<num_trades<<endl
            <<"Number of Shares Traded: "<<num_shares<<endl;
        if(transfers) print_transfers();
        if(ttt) print_ttt();
    }

    void test_add(ifstream & file){
        int timestamp, price, amount, last;
        string name, buy, ename,sprice, samount;
        for(int i = 0; i<16; i++) {
            file >> timestamp >> name >> buy >> ename >> sprice >> samount >> last;
            price = stoi(sprice.substr(1));
            amount = stoi(samount.substr(1));
            if (timestamp != time) {
                time = timestamp;
                remove_expire();
            }
            if (buy == "BUY")
                add_order<greater<int>>(true, ename, name, price, last, amount, this->buy_equity,
                          this->active_buy_max, this->ttt_buy_info);
            else
                add_order<less<int>>(false, ename, name, price, last, amount, this->sell_equity,
                          this->active_sell_min, this->ttt_sell_info);
            cout<<"time: "<<time<<endl;
//            print("BUY", buy_equity, active_buy_max, ttt_sell_info);
//            print("SELL", sell_equity, active_sell_min, ttt_buy_info);
        }
    }

    void test(ifstream & file){
        int timestamp, price, amount, last;
        string name, buy, ename,sprice, samount;
        for(int i = 0; i<16; i++) {
            file >> timestamp >> name >> buy >> ename >> sprice >> samount >> last;
            price = stoi(sprice.substr(1));
            amount = stoi(samount.substr(1));
            test_add_client(name);
//            cout<<timestamp<<endl;
            if (timestamp != time) {
                if (this->median) get_median();
                if (this->midpoint) get_midpoint();
                this->time = timestamp;
                remove_expire();
//                cout<<time<<endl;
            }
            if(buy=="BUY"){
                int left = match_order<less<int>>(true, ename, name, price, last, amount, sell_equity, active_sell_min);
                if(left==0) add_order<greater<int>>(true, ename, name, price, 0, amount, this->buy_equity,
                                                                    this->active_buy_max, this->ttt_buy_info);
                else add_order<greater<int>>(true, ename, name, price, last, left, this->buy_equity,
                                                             this->active_buy_max, this->ttt_buy_info);
            }
            else{
                int left = match_order<greater<int>>(false, ename, name, price, last, amount, buy_equity, active_buy_max);
                if(left==0) add_order<less<int>>(false, ename, name, price, 0, amount, this->sell_equity,
                                                    this->active_sell_min, this->ttt_sell_info);
                else add_order<less<int>>(false, ename, name, price, last, left, this->sell_equity,
                                             this->active_sell_min, this->ttt_sell_info);
            }
            orders++;
        }
        if (this->median) get_median();
        if (this->midpoint) get_midpoint();
//        if(buy=="BUY") add_order(ename, name, price, last, amount, this->buy_equity,
//                                 this->active_buy_max, this->ttt_buy_info);
//        else  add_order(ename, name, price, last, amount, this->sell_equity,
//                        this->active_sell_min, this->ttt_sell_info);
        cout<<"---End of Day---"<<endl
        <<"Commission Earnings: $"<<cmsn_earning<<endl
        <<"Total Amount of Money Transferred: $"<<total_transfered<<endl
        <<"Number of Completed Trades: "<<num_trades<<endl
        <<"Number of Shares Traded: "<<num_shares<<endl;
        if(transfers) print_transfers();
        if(ttt) print_ttt();
    }



};


#endif //CODING4_SIMULATION_H
