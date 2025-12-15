#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <bits/stdc++.h>
using namespace std;

class OrderBook {
private:
    map<double, vector<Order>> asks;                   // Low -> High
    map<double, vector<Order>, greater<double>> bids;  // High -> Low

    void executeTrade(Order& incoming, Order& bookOrder) {
        // How many can we trade? The smaller of the two quantities.
        int tradeQty = min(incoming.quantity, bookOrder.quantity);

        cout<<" [TRADE] "<<tradeQty<<" shares @ "<<bookOrder.price<<endl;

        // Update quantities
        incoming.quantity -= tradeQty;
        bookOrder.quantity -= tradeQty;
    }

public:
    // Core Logic: Try to match, then store remainder
    void addOrder(Order order){ 
        if (order.type == OrderType::BUY) {
            matchBuyOrder(order); // Try to buy from Asks
            if (order.quantity > 0) {
                bids[order.price].push_back(order); // Store remaining
            }
        }else{
            matchSellOrder(order); // Try to sell to Bids
            if(order.quantity>0){
                asks[order.price].push_back(order); // Store remaining
            }
        }
    }

    void matchBuyOrder(Order& order){
        while(order.quantity > 0 && !asks.empty()) {
            auto bestAskIt = asks.begin(); // Lowest sell price
            double bestPrice = bestAskIt->first;
            vector<Order>& bookOrders = bestAskIt->second;

            // If my buy price is LOWER than the cheapest seller, no trade possible.
            if(order.price < bestPrice)break;

            // Note: We use an index loop because we might delete items
            for(auto it = bookOrders.begin();it != bookOrders.end();) {
                executeTrade(order, *it);
                // If book order is empty, remove it
                if(it->quantity == 0){
                    it = bookOrders.erase(it);
                }else{
                    ++it;
                }

                if(order.quantity==0)break;
            }
            if(bookOrders.empty()){
                asks.erase(bestAskIt);
            }
        }
    }

    void matchSellOrder(Order& order){
        while(order.quantity > 0 && !bids.empty()) {
            auto bestBidIt = bids.begin(); // Highest buy price
            double bestPrice = bestBidIt->first;
            vector<Order>& bookOrders = bestBidIt->second;

            if(order.price > bestPrice)break;

            for(auto it = bookOrders.begin();it != bookOrders.end();) {
                executeTrade(order, *it);

                if(it->quantity==0){
                    it = bookOrders.erase(it);
                }else{
                    ++it;
                }

                if(order.quantity==0)break;
            }

            if(bookOrders.empty()){
                bids.erase(bestBidIt);
            }
        }
    }

    void printBook(){
        cout<<"\n=== ORDER BOOK ===" << endl;
        cout<<"   ASKS (Sellers) "<<endl;
        for(auto it = asks.rbegin();it != asks.rend();++it){
             double price = it->first;
             int total_qty = 0;
             for(const auto& o : it->second) total_qty += o.quantity;
             cout<<"Price: "<<price <<" | Qty: "<<total_qty<<endl;
        }
        cout<<"------------------"<<endl;
        for(auto it = bids.begin(); it != bids.end(); ++it){
             double price = it->first;
             int total_qty = 0;
             for(const auto& o : it->second) total_qty += o.quantity;
             cout<<"Price: "<<price<<" | Qty: "<<total_qty<<endl;
        }
        cout<<"   BIDS (Buyers) "<<endl;
        cout<<"==================\n"<<endl;
    }
};

#endif