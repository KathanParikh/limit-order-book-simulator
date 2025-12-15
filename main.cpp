#include <bits/stdc++.h>
#include "Order.hpp"
using namespace std;

int main() {
    cout << "--- Limit Order Book Simulator Started ---" <<endl;

    // Create a fake Buy order
    Order order1(1, OrderType::BUY, 100.50, 10);
    
    // Create a fake Sell order
    Order order2(2, OrderType::SELL, 101.00, 5);
    
    order1.print();
    order2.print();

    return 0;
}