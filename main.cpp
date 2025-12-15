#include <bits/stdc++.h>
#include "Order.hpp"
#include "OrderBook.hpp"

using namespace std;

int main() {
    cout<<"--- Limit Order Book Simulator Started ---" <<endl;

    // Order order1(1, OrderType::BUY, 100.50, 10);
    // Order order2(2, OrderType::SELL, 101.00, 5);

    // order1.print();
    // order2.print();
    OrderBook book;
    book.addOrder(Order(1, OrderType::SELL, 100.00, 10));
    book.addOrder(Order(2, OrderType::SELL, 101.00, 10));
    
    cout<<"Initial Book:"<<endl;
    book.printBook();

    cout<<"Incoming BUY Order: 15 @ 102.00"<<endl;
    book.addOrder(Order(3, OrderType::BUY, 102.00, 15));

    cout<<"\nFinal Book:"<<endl;
    book.printBook();

    return 0;
}