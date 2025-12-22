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

//Order Structure
struct Order {
    int id;             // ID
    OrderType type;     // Buy or Sell
    double price;       // Price limit
    int quantity;       // Number of shares

    // Constructor
    Order(int _id, OrderType _type, double _price, int _quantity)
        : id(_id), type(_type), price(_price), quantity(_quantity) {}
        
    void print() const {
        cout <<"Order[" <<id<< "] "<<(type == OrderType::BUY ? "BUY" : "SELL")<<" "<<quantity<<" @ "<<price<<endl;
    }
};

#endif