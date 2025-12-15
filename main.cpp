#include <bits/stdc++.h>
#include "Order.hpp"
#include "OrderBook.hpp"
#include "OrderQueue.hpp"

using namespace std;

// Shared resources
OrderBook book;
OrderQueue orderQueue;

// --- PRODUCER FUNCTION ---
void simulateMarket() {
    for (int i = 0; i < 10; ++i) {
        // creating fake orders
        OrderType type = (i % 2 == 0) ? OrderType::BUY : OrderType::SELL;
        double price = 100.0 + (i % 5); // Random-ish price
        
        Order order(i, type, price, 10);
        
        cout << "[Market] Generated Order " << i << endl;
        orderQueue.push(order);
        
        // Simulate time delay (market isn't instant)
        this_thread::sleep_for(chrono::milliseconds(100)); 
    }
    // Tell consumer we are done
    orderQueue.stop();
}

// --- CONSUMER FUNCTION ---
void runMatchingEngine() {
    Order order(0, OrderType::BUY, 0, 0); // Temp placeholder
    
    while (true) {
        // Wait for an order...
        bool active = orderQueue.pop(order);
        
        if (!active) break; // Stop if queue is closed

        cout << "[Engine] Processing Order " << order.id << endl;
        book.addOrder(order);
    }
}



int main() {
    cout<<"--- Limit Order Book Simulator Started ---" <<endl;

    // Order order1(1, OrderType::BUY, 100.50, 10);
    // Order order2(2, OrderType::SELL, 101.00, 5);

    // order1.print();
    // order2.print();
    // OrderBook book;
    // book.addOrder(Order(1, OrderType::SELL, 100.00, 10));
    // book.addOrder(Order(2, OrderType::SELL, 101.00, 10));
    
    // cout<<"Initial Book:"<<endl;
    // book.printBook();

    // cout<<"Incoming BUY Order: 15 @ 102.00"<<endl;
    // book.addOrder(Order(3, OrderType::BUY, 102.00, 15));

    // cout<<"\nFinal Book:"<<endl;
    // book.printBook();

    cout << "--- Starting Simulation ---" << endl;

    thread producerThread(simulateMarket);
    thread consumerThread(runMatchingEngine);

    // Wait for them to finish
    producerThread.join();
    consumerThread.join();

    cout << "--- Simulation Complete ---" << endl;
    book.printBook();

    return 0;
}