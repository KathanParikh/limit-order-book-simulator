#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <random>
#include <chrono>
#include <iomanip>
#include <cmath>

#include "../include/Order.hpp"
#include "../include/OrderBook.hpp"
#include "../include/OrderQueue.hpp"

using namespace std;

// --- SHARED METRICS ---
struct SystemMetrics {
    atomic<int> ordersProcessed{0};
    atomic<long long> totalLatency{0}; 
    atomic<double> avgLatency{0.0};
};

vector<long long> latencies;
OrderBook book;
OrderQueue orderQueue;
atomic<bool> isRunning{true};
SystemMetrics metrics;

// --- PRODUCER ---
void simulateMarket() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> sideDist(0, 1);       
    uniform_int_distribution<> priceDist(98, 102);   
    uniform_int_distribution<> quantDist(10, 80);   
    uniform_int_distribution<> typeDist(1, 100);     
    int orderId = 1;

    while (isRunning) {
        Side side = (sideDist(gen) == 0) ? Side::BUY : Side::SELL;
        int quantity = quantDist(gen);
        double price = (double)priceDist(gen);
        OrderType type = OrderType::LIMIT;

        // 15% Market Orders
        if (typeDist(gen) <= 15) {
            type = OrderType::MARKET;
            price = 0.0;
        }

        Order order(orderId++, side, type, price, quantity);
        orderQueue.push(order);
        
        int delay = (orderId % 20 == 0) ? 10 : 50; 
        this_thread::sleep_for(chrono::milliseconds(delay)); 
    }
    orderQueue.stop();
}

// --- CONSUMER ---
void runMatchingEngine() {
    Order order(0, Side::BUY, OrderType::LIMIT, 0, 0); 
    latencies.reserve(100000);
    long long localTotalLatency = 0;
    int localCount = 0;

    while (true) {
        bool active = orderQueue.pop(order);
        if (!active) break; 

        auto start = chrono::high_resolution_clock::now();
        book.addOrder(order); 
        auto end = chrono::high_resolution_clock::now();
        
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        latencies.push_back(duration.count());

        localTotalLatency += duration.count();
        localCount++;

        if (localCount % 10 == 0) {
            metrics.ordersProcessed += 10;
            metrics.totalLatency += localTotalLatency;
            long long total = metrics.totalLatency.load();
            int count = metrics.ordersProcessed.load();
            if (count > 0) metrics.avgLatency = (double)total / count;
            localTotalLatency = 0; 
        }
    }
}

// --- HELPER: ASCII BAR ---
string drawProgressBar(double percentage) {
    int width = 10; 
    string bar = "[";
    if (percentage > 0) {
        bar += string(width, ' '); 
        bar += "|"; 
        int fill = (int)(percentage * width);
        bar += string(fill, '#'); 
        bar += string(width - fill, ' ');
    } else {
        int fill = (int)(abs(percentage) * width);
        bar += string(width - fill, ' ');
        bar += string(fill, '#');
        bar += "|"; 
        bar += string(width, ' ');
    }
    bar += "]";
    return bar;
}

// --- LIVE DASHBOARD ---
void displayStats() {
    while (isRunning) {
        cout << "\033[2J\033[1;1H"; // Clear Screen

        double imbalance = book.getImbalance();
        auto lastTrades = book.getLastTrades();
        
        string prediction = "NEUTRAL";
        string color = "\033[0m"; 
        if (imbalance > 0.3) { prediction = "BULLISH"; color = "\033[32m"; }
        else if (imbalance < -0.3) { prediction = "BEARISH"; color = "\033[31m"; }

        // --- HEADER: PERFORMANCE HUD (Feature #4) ---
        cout << "================================================" << endl;
        cout << " [SYSTEM STATUS]  Orders: " << setw(5) << metrics.ordersProcessed 
             << " | Latency: " << setw(3) << (int)metrics.avgLatency << " us" << endl;
        cout << "================================================" << endl;
        
        // MARKET SENTIMENT
        cout << " Signal    : " << color << drawProgressBar(imbalance) 
             << " " << prediction << "\033[0m" << endl;
        cout << "------------------------------------------------" << endl;

        // ORDER BOOK VISUALIZATION
        vector<OrderBook::LevelInfo> bids, asks;
        book.getOrderBookSnapshot(bids, asks);

        cout << "   ASKS (Sellers)" << endl;
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            cout << "   $" << setw(6) << it->price << " | " << string(it->quantity / 5, '*') << " (" << it->quantity << ")" << endl;
        }

        cout << "   ---------------------------------" << endl;

        for (auto& level : bids) {
            cout << "   $" << setw(6) << level.price << " | " << string(level.quantity / 5, '*') << " (" << level.quantity << ")" << endl;
        }
        cout << "   BIDS (Buyers)" << endl;
        cout << "------------------------------------------------" << endl;

        // LAST TRADE (Single Line Only)
        if (!lastTrades.empty()) {
            auto t = lastTrades.front(); // Most recent
            string sideStr = (t.side == Side::BUY) ? "BUY " : "SELL";
            string sideColor = (t.side == Side::BUY) ? "\033[32m" : "\033[31m";
            cout << " LAST TRADE: " << sideColor << sideStr << "\033[0m" 
                 << t.quantity << " @ $" << t.price << endl;
        } else {
            cout << " LAST TRADE: (Waiting...)" << endl;
        }

        cout << "================================================" << endl;
        cout << " [ENTER] to Stop and View Full Trade Log" << endl;
        
        this_thread::sleep_for(chrono::milliseconds(200)); 
    }
}

void saveLatenciesToCSV() {
    ofstream file("latencies.csv");
    file << "Order_ID,Latency_Microseconds\n";
    for (size_t i = 0; i < latencies.size(); ++i) {
        file << i << "," << latencies[i] << "\n";
    }
    file.close();
}

int main() {
    cout << "--- Simulation Started ---" << endl;
    thread producerThread(simulateMarket);
    thread consumerThread(runMatchingEngine);
    thread displayThread(displayStats);

    cin.get(); // BLOCKS HERE until you hit Enter
    
    // --- SHUTDOWN SEQUENCE ---
    isRunning = false;
    orderQueue.stop(); 
    producerThread.join();
    consumerThread.join();
    displayThread.join();

    // --- SESSION REPORT (This is what you asked for!) ---
    cout << "\n\n";
    cout << "========================================" << endl;
    cout << "          SESSION SUMMARY REPORT        " << endl;
    cout << "========================================" << endl;
    cout << " Total Orders Processed : " << metrics.ordersProcessed << endl;
    cout << " Average Latency        : " << metrics.avgLatency << " microseconds" << endl;
    cout << "----------------------------------------" << endl;
    cout << " LAST 5 TRADES:" << endl;
    
    auto history = book.getLastTrades();
    for (const auto& t : history) {
        string side = (t.side == Side::BUY) ? "BUY " : "SELL";
        cout << "  -> " << side << " " << t.quantity << " @ $" << t.price << endl;
    }
    cout << "========================================" << endl;

    saveLatenciesToCSV();
    return 0;
}