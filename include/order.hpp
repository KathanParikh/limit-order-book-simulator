#ifndef ORDER_HPP
#define ORDER_HPP

#include <iostream>
#include <string>

// 1. Separate Side (Direction)
enum class Side {
    BUY,
    SELL
};

// 2. Separate Type (Behavior)
enum class OrderType {
    LIMIT,      // Standard: Buy/Sell at specific price
    MARKET,     // Aggressor: Buy/Sell immediately at best available
    STOP,       // Trigger: Becomes a MARKET/LIMIT order when price hits X
    ICEBERG     // Hidden: Only shows a small tip of the total size
};

struct Order {
    int id;
    Side side;              // BUY or SELL
    OrderType type;         // LIMIT, MARKET, etc.
    double price;           // Limit Price (ignored for MARKET)
    int quantity;           // Current visible quantity
    
    //NEW FIELDS
    int originalQuantity;   // Total size (for Icebergs & fill calculation)
    double stopPrice;       // Logic for Stop Orders (Trigger)
    int hiddenQuantity;     // Logic for Iceberg (Reserve)
    
    Order(int _id, Side _side, OrderType _type, double _price, int _qty, 
          double _stopPrice = 0.0, int _hiddenQty = 0)
        : id(_id), side(_side), type(_type), price(_price), quantity(_qty),
          originalQuantity(_qty), 
          stopPrice(_stopPrice), hiddenQuantity(_hiddenQty) 
    {
        // For Icebergs, original size is Visible + Hidden
        if (type == OrderType::ICEBERG) {
            originalQuantity = quantity + hiddenQuantity;
        }
    }
};

#endif