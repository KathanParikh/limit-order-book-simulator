#include <benchmark/benchmark.h>
#include <thread>
#include <atomic>
#include <random>
#include "../include/OrderBook.hpp"
#include "../include/OrderQueue.hpp"
#include "../include/Order.hpp"

// Benchmark 1: Measure raw insertion speed of a Sell Limit Order
static void BM_AddLimitOrder(benchmark::State& state) {
    // Setup (Runs once)
    OrderBook book;
    
    // The Loop (Runs millions of times)
    for (auto _ : state) {
        // We create a new order every time to simulate traffic
        // Using a static ID (1) means we are updating/inserting the same entry mostly
        book.addOrder(Order(1, Side::SELL, OrderType::LIMIT, 100.0, 10));
    }
}

// Benchmark 2: Measure Matching (Buy meets Sell)
static void BM_MatchOrder(benchmark::State& state) {
    OrderBook book;
    // Pre-fill the book with 10,000 Sell Orders (Liquidity)
    for(int i=0; i<10000; ++i) {
        book.addOrder(Order(i, Side::SELL, OrderType::LIMIT, 100.0 + (i%10), 10));
    }

    int id = 20000;
    for (auto _ : state) {
        Order incoming(id++, Side::BUY, OrderType::MARKET, 0.0, 5);
        book.addOrder(std::move(incoming));
    }
}

// Benchmark 3: Multi-threaded Producer-Consumer Throughput Test
static void BM_MultiThreadedThroughput(benchmark::State& state) {
    const int numProducers = state.range(0);
    const int ordersPerProducer = 10000;
    
    for (auto _ : state) {
        OrderBook book;
        OrderQueue orderQueue;
        std::atomic<int> orderIdCounter{0};
        std::atomic<bool> producersFinished{false};
        std::atomic<int> processedCount{0};
        
        // Consumer thread
        std::thread consumer([&]() {
            Order order(0, Side::BUY, OrderType::LIMIT, 0, 0);
            while (true) {
                bool active = orderQueue.pop(order);
                if (!active) break;
                book.addOrder(std::move(order));
                processedCount++;
            }
        });
        
        // Multiple producer threads
        std::vector<std::thread> producers;
        for (int p = 0; p < numProducers; ++p) {
            producers.emplace_back([&, p]() {
                std::mt19937 gen(p);
                std::uniform_int_distribution<> sideDist(0, 1);
                std::uniform_int_distribution<> priceDist(95, 105);
                std::uniform_int_distribution<> qtyDist(10, 50);
                
                for (int i = 0; i < ordersPerProducer; ++i) {
                    int id = orderIdCounter++;
                    Side side = (sideDist(gen) == 0) ? Side::BUY : Side::SELL;
                    double price = (double)priceDist(gen);
                    int qty = qtyDist(gen);
                    
                    orderQueue.push(Order(id, side, OrderType::LIMIT, price, qty));
                }
            });
        }
        
        // Wait for producers to finish
        for (auto& t : producers) {
            t.join();
        }
        
        // Signal consumer to stop
        orderQueue.stop();
        consumer.join();
        
        // Set the items processed metric
        state.SetItemsProcessed(processedCount);
    }
}

// Register the functions
BENCHMARK(BM_AddLimitOrder);
BENCHMARK(BM_MatchOrder);
BENCHMARK(BM_MultiThreadedThroughput)->DenseRange(1, 4)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();