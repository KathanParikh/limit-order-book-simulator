#include <benchmark/benchmark.h>
#include "../include/OrderBook.hpp"
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
        // State::PauseTiming(); // Pause clock while we prepare
        Order incoming(id++, Side::BUY, OrderType::MARKET, 0.0, 5);
        // State::ResumeTiming(); // Resume clock
        
        book.addOrder(incoming);
    }
}

// Register the functions
BENCHMARK(BM_AddLimitOrder);
BENCHMARK(BM_MatchOrder);

BENCHMARK_MAIN();