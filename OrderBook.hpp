/* * OPTIMIZATION NOTE:
 * Currently using std::map (Red-Black Tree) for simplicity.
 * In a real HFT production environment, we would replace this with a
 * contiguous memory structure (std::vector or flat array) to minimize
 * CPU cache misses and avoid dynamic memory allocation during trading.
 */


#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <bits/stdc++.h>
using namespace std;

class OrderBook {
private:
    map<double, vector<Order>> asks;                   // Low -> High
    map<double, vector<Order>, greater<double>> bids;  // High -> Low
    mutable std::mutex bookMtx;

    void executeTrade(Order& incoming, Order& bookOrder) {
        // How many can we trade? The smaller of the two quantities.
        int tradeQty = min(incoming.quantity, bookOrder.quantity);

        // cout<<" [TRADE] "<<tradeQty<<" shares @ "<<bookOrder.price<<endl;

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

    struct LevelInfo { double price; int quantity; };
    
    void getOrderBookSnapshot(vector<LevelInfo>& bestBids, vector<LevelInfo>& bestAsks) {
        lock_guard<mutex> lock(bookMtx); // LOCK while reading
        
        // Get Top 5 Asks
        int count = 0;
        for (auto& entry : asks) {
            int qty = 0;
            for (auto& o : entry.second) qty += o.quantity;
            bestAsks.push_back({entry.first, qty});
            if (++count >= 5) break;
        }

        // Get Top 5 Bids
        count = 0;
        for (auto& entry : bids) {
            int qty = 0;
            for (auto& o : entry.second) qty += o.quantity;
            bestBids.push_back({entry.first, qty});
            if (++count >= 5) break;
        }
    }

    // Returns value between -1.0 (Selling Pressure) and 1.0 (Buying Pressure)
    double getImbalance() {
        lock_guard<mutex> lock(bookMtx); // Safe access

        double totalBids = 0;
        double totalAsks = 0;

        // Sum up the quantity of the top 5 levels
        int count = 0;
        for (auto& entry : bids) {
            for (const auto& o : entry.second) totalBids += o.quantity;
            if (++count >= 5) break;
        }

        count = 0;
        for (auto& entry : asks) {
            for (const auto& o : entry.second) totalAsks += o.quantity;
            if (++count >= 5) break;
        }

        // Prevent division by zero if book is empty
        double total = totalBids + totalAsks;
        if (total == 0) return 0.0;

        return (totalBids - totalAsks) / total;
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