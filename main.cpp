#include <bits/stdc++.h>
#include <random>
#include <atomic>
#include <chrono>
#include "Order.hpp"
#include "OrderBook.hpp"
#include "OrderQueue.hpp"

using namespace std;

// Shared resources
OrderBook book;
OrderQueue orderQueue;
atomic<bool> isRunning{true};

// --- PRODUCER FUNCTION ---
void simulateMarket() {
    // 1. Setup Randomness (Standard C++ Boilerplate)
    random_device rd;  // Get a seed from the OS
    mt19937 gen(rd()); // Initialize the Mersenne Twister engine
    
    // Define ranges
    uniform_int_distribution<> sideDist(0, 1);       // 0 or 1 (Buy/Sell)
    uniform_int_distribution<> priceDist(98, 102);   // Price between 98 and 102
    uniform_int_distribution<> quantDist(1, 100);    // Quantity 1-100
    
    int orderId = 1;

    while (isRunning) {
        // 2. Generate Random Values
        OrderType type = (sideDist(gen) == 0) ? OrderType::BUY : OrderType::SELL;
        double price = (double)priceDist(gen); // Simple integer prices for now
        int quantity = quantDist(gen);

        Order order(orderId++, type, price, quantity);
        
        // 3. Push to Engine
        orderQueue.push(order);
        
        // 4. Wait a bit (Market speed)
        // If we make this too fast (e.g., 1ms), the console will explode!
        this_thread::sleep_for(chrono::milliseconds(200)); 
    }
    
    orderQueue.stop();
}

// --- CONSUMER FUNCTION ---
void runMatchingEngine() {
    Order order(0, OrderType::BUY, 0, 0); // Temp placeholder
    long long totalLatency = 0;
    int ordersProcessed = 0;
    
    while (true) {
        // Wait for an order...
        bool active = orderQueue.pop(order);
        if (!active) break; 

        auto start = chrono::high_resolution_clock::now();
        // cout << "[Engine] Processing Order " << order.id << endl;
        book.addOrder(order);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        totalLatency += duration.count();
        ordersProcessed++;

        // Every 100 orders, print a status report
        if(ordersProcessed % 100 == 0) {
            double avgLatency = (double)totalLatency / ordersProcessed;
            cout << "[Metrics] Processed: " << ordersProcessed 
                 << " | Avg Latency: " << avgLatency << " ms" << endl;
        }
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
    cout << "Press ENTER to stop..." << endl;

    // Launch threads
    thread producerThread(simulateMarket);
    thread consumerThread(runMatchingEngine);

    // Keep main thread alive until user hits Enter
    cin.get(); 
    
    cout << "Shutting down..." << endl;
    isRunning = false;
    orderQueue.stop(); // Wake up consumer so it can exit

    producerThread.join();
    consumerThread.join();

    cout << "--- Simulation Complete ---" << endl;
    book.printBook();

    return 0;
}