#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <map>
#include <vector>
#include <deque> 
#include <mutex>
#include <algorithm>
#include "Order.hpp"

using namespace std;

// Struct for Trade History
struct TradeInfo {
    double price;
    int quantity;
    Side side; // Who initiated? (Aggressor)
};

class OrderBook {
private:
    map<double, vector<Order>> asks;
    map<double, vector<Order>, greater<double>> bids;
    
    // History Buffer
    deque<TradeInfo> lastTrades; // Stores last 5 trades
    
    mutable std::mutex bookMtx;

    // --- CORE MATCHING LOGIC ---
    void executeTrade(Order& incoming, Order& bookOrder) {
        int tradeQty = min(incoming.quantity, bookOrder.quantity);
        
        // 1. Store Trade in History (Keep max 5)
        // Note: pushing to deque is fast (~10-20ns)
        lastTrades.push_front({bookOrder.price, tradeQty, incoming.side});
        if (lastTrades.size() > 5) {
            lastTrades.pop_back();
        }

        // 2. Update Quantities
        incoming.quantity -= tradeQty;
        bookOrder.quantity -= tradeQty;
    }

    void matchMarketOrder(Order& order) {
        if (order.side == Side::BUY) {
            while (order.quantity > 0 && !asks.empty()) {
                auto bestAskIt = asks.begin();
                vector<Order>& bookOrders = bestAskIt->second;
                for (auto it = bookOrders.begin(); it != bookOrders.end(); ) {
                    executeTrade(order, *it);
                    if (it->quantity == 0) it = bookOrders.erase(it);
                    else ++it;
                    if (order.quantity == 0) return;
                }
                if (bookOrders.empty()) asks.erase(bestAskIt);
            }
        }
        else { 
            while (order.quantity > 0 && !bids.empty()) {
                auto bestBidIt = bids.begin();
                vector<Order>& bookOrders = bestBidIt->second;
                for (auto it = bookOrders.begin(); it != bookOrders.end(); ) {
                    executeTrade(order, *it);
                    if (it->quantity == 0) it = bookOrders.erase(it);
                    else ++it;
                    if (order.quantity == 0) return;
                }
                if (bookOrders.empty()) bids.erase(bestBidIt);
            }
        }
    }

public:
    void addOrder(Order order) { 
        lock_guard<mutex> lock(bookMtx);

        if (order.type == OrderType::MARKET) {
            matchMarketOrder(order);
            return; 
        }

        if (order.side == Side::BUY) {
            matchBuyOrder(order);
            if (order.quantity > 0) bids[order.price].push_back(order);
        } else {
            matchSellOrder(order);
            if (order.quantity > 0) asks[order.price].push_back(order);
        }
    }

    void matchBuyOrder(Order& order) {
        while (order.quantity > 0 && !asks.empty()) {
            auto bestAskIt = asks.begin();
            double bestPrice = bestAskIt->first;
            if (order.price < bestPrice) break;

            vector<Order>& bookOrders = bestAskIt->second;
            for (auto it = bookOrders.begin(); it != bookOrders.end(); ) {
                executeTrade(order, *it);
                if (it->quantity == 0) it = bookOrders.erase(it);
                else ++it;
                if (order.quantity == 0) return;
            }
            if (bookOrders.empty()) asks.erase(bestAskIt);
        }
    }

    void matchSellOrder(Order& order) {
        while (order.quantity > 0 && !bids.empty()) {
            auto bestBidIt = bids.begin();
            double bestPrice = bestBidIt->first;
            if (order.price > bestPrice) break;

            vector<Order>& bookOrders = bestBidIt->second;
            for (auto it = bookOrders.begin(); it != bookOrders.end(); ) {
                executeTrade(order, *it);
                if (it->quantity == 0) it = bookOrders.erase(it);
                else ++it;
                if (order.quantity == 0) return;
            }
            if (bookOrders.empty()) bids.erase(bestBidIt);
        }
    }

    // --- SNAPSHOTS ---
    struct LevelInfo { double price; int quantity; };

    void getOrderBookSnapshot(vector<LevelInfo>& bestBids, vector<LevelInfo>& bestAsks) {
        lock_guard<mutex> lock(bookMtx);
        int count = 0;
        for (auto& entry : asks) {
            int qty = 0;
            for (auto& o : entry.second) qty += o.quantity;
            bestAsks.push_back({entry.first, qty});
            if (++count >= 5) break;
        }
        count = 0;
        for (auto& entry : bids) {
            int qty = 0;
            for (auto& o : entry.second) qty += o.quantity;
            bestBids.push_back({entry.first, qty});
            if (++count >= 5) break;
        }
    }

    // NEW: Get Recent Trades
    vector<TradeInfo> getLastTrades() {
        lock_guard<mutex> lock(bookMtx);
        return vector<TradeInfo>(lastTrades.begin(), lastTrades.end());
    }

    double getImbalance() {
        lock_guard<mutex> lock(bookMtx);
        double totalBids = 0, totalAsks = 0;
        int count = 0;
        for (auto& entry : bids) {
            for (auto& o : entry.second) totalBids += o.quantity;
            if (++count >= 5) break;
        }
        count = 0;
        for (auto& entry : asks) {
            for (auto& o : entry.second) totalAsks += o.quantity;
            if (++count >= 5) break;
        }
        double total = totalBids + totalAsks;
        return (total == 0) ? 0.0 : (totalBids - totalAsks) / total;
    }
};

#endif