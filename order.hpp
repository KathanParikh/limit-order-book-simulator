#ifndef ORDER_HPP
#define ORDER_HPP

#include <bits/stdc++.h>
using namespace std;
// 1. Enum for Order Type
// specific types prevent magic strings like "buy" or "sell"
enum class OrderType {
    BUY,
    SELL
};

// 2. The Order Structure
struct Order {
    int id;             // Unique ID
    OrderType type;     // Buy or Sell
    double price;       // Price limit
    int quantity;       // Number of shares

    // Constructor for easy creation
    Order(int _id, OrderType _type, double _price, int _quantity)
        : id(_id), type(_type), price(_price), quantity(_quantity) {}
        
    // Helper to print order details (good for debugging)
    void print() const {
        cout <<"Order[" <<id<< "] "<<(type == OrderType::BUY ? "BUY" : "SELL")<<" "<<quantity<<" @ "<<price<<endl;
    }
};

#endif