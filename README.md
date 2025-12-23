# High-Performance Limit Order Book Engine

> A microsecond-latency matching engine in C++.  Built to understand how stock exchanges work and to practice concurrent programming at scale.

![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![Latency](https://img.shields.io/badge/Latency-Sub--microsecond-brightgreen)
![Multi-threaded](https://img.shields.io/badge/Multi--threading-Enabled-orange)

---

## What Is This? 

A **Limit Order Book** is the core system that powers stock exchanges.  When you place a trade, it goes into an order book that matches buyers with sellers. 

This project simulates that system with a focus on: 
- **Speed** (processing orders in nanoseconds)
- **Thread safety** (handling multiple orders simultaneously)
- **Real-world features** (stop orders, cascade protection, live monitoring)

---

## Key Features

### 1. Order Types
| Type | What It Does |
|------|--------------|
| **LIMIT** | "Buy 100 shares at $150 (or better)" |
| **MARKET** | "Buy now, whatever the price" |
| **STOP** | "Sell if price drops below $145" |

### 2. Multi-Threaded Design

The system runs 3 threads simultaneously: 

```
Producer Thread          Queue          Consumer Thread          Order Book
(Generates orders) ────▶ [████] ────▶ (Matches trades) ────▶ Bids | Asks
                                                                    |
                                                    Dashboard Thread
                                                    (Shows live stats)
```

**Why 3 threads?**
- **Producer:** Creates realistic market activity
- **Consumer:** Executes the matching logic (the "hot path")
- **Dashboard:** Displays live stats without slowing down the engine

**How they communicate safely:**
- Uses `mutex` locks (prevents race conditions)
- Uses `condition_variable` (wakes up consumer when orders arrive)
- No busy-waiting (efficient CPU usage)

---

### 3. Data Structure:  Price-Level Maps

Orders are stored in sorted maps for fast lookups:

```cpp
std::map<double, vector<Order>> asks;  // Sellers (sorted low to high)
std::map<double, vector<Order>, greater<double>> bids;  // Buyers (high to low)
```

**Why this works:**
- Finding the best price = O(1) (just grab the first element)
- Orders at the same price are matched in order (FIFO)

---

### 4. Stop Order Protection

**The Problem:** When stop orders trigger, they can cause a chain reaction (like the 2010 Flash Crash).

**My Solution:**
- Check for triggered stops every 10 trades (not every trade) = 90% faster
- Block recursive checking (prevents infinite loops)
- Queue triggered orders and process them one-by-one

```cpp
if (isCheckingStops) return;  // Prevent re-entry
isCheckingStops = true;
// ... collect and process triggered stops safely
isCheckingStops = false;
```

---

### 5. Performance Tracking

Every order is timed with microsecond precision:

```cpp
auto start = chrono::high_resolution_clock:: now();
book.addOrder(order);
auto end = chrono:: high_resolution_clock::now();
```

**Output on shutdown:**
```
========================================
     LATENCY DISTRIBUTION
========================================
Samples      :  10,000+
Min Latency  : <1 µs
Avg Latency  : ~5-10 µs
----------------------------------------
p50 (Median) : ~5 µs
p99          : <70 µs
p99.9        : <250 µs
========================================
```

Data is saved to `latencies.csv` for visualization.

---

### 6. Google Benchmark Results

Micro-benchmarks measure raw speed:

```
Benchmark              Time       CPU    Iterations
----------------------------------------------------------------
BM_AddLimitOrder     126 ns    122 ns    6,400,000
BM_MatchOrder       64.8 ns   64.2 ns   11,200,000
```

**What this means:**
- Can insert **~8 million orders/second** (single-threaded)
- Matching is even faster when the book already has liquidity
- Compiler optimizations (`-O3 -march=native`) are crucial

---

### 7. Live Dashboard

While running, you see real-time updates:

```
================================================
[SYSTEM STATUS]  Orders: 1,245 | Latency: 7 µs | Stops: 12
================================================
Signal    :    [     |##########     ] BULLISH
------------------------------------------------
   ASKS (Sellers)
   $102  | ****** (35 shares)
   $101  | ***** (28 shares)
   ---------------------------------
   $100  | ****** (33 shares)
   $ 99  | ***** (29 shares)
   BIDS (Buyers)
------------------------------------------------
LAST TRADE: BUY 15 @ $101
================================================
```

Shows:
- Top 5 price levels on each side
- Market imbalance (buying vs.  selling pressure)
- Live trade feed

---

## How to Build

```bash
# Clone the repo
git clone https://github.com/KathanParikh/limit-order-book-simulator.git
cd limit-order-book-simulator

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. 
cmake --build .

# Run simulator
./simulator
# Press ENTER to stop and see final stats

# Run benchmarks
./bench_test
```

**Visualize latency:**
```bash
pip install pandas matplotlib
python ../plot_latencies.py
```

---

## Project Structure

```
.
├── include/
│   ├── order.hpp          # Order types and enums
│   ├── OrderBook.hpp      # Matching engine logic
│   └── OrderQueue.hpp     # Thread-safe queue
├── src/
│   └── main.cpp           # 3-thread simulator + dashboard
├── benchmarks/
│   └── main.cpp           # Performance tests
└── plot_latencies.py      # Visualization script
```

---

## What I Learned

**Technical Skills:**
- **Concurrency:** Mutexes, condition variables, atomics
- **Performance:** Profiling, compiler flags, data structure choice
- **C++ Best Practices:** Move semantics, RAII, STL containers

**Domain Knowledge:**
- How exchanges match orders (price-time priority)
- Why stop orders can cause crashes
- Market microstructure (spread, liquidity, imbalance)

**Tools:**
- CMake for building
- Google Benchmark for micro-benchmarks
- Python for data visualization

---

## Future Enhancements

**Order Management:**
- [ ] Order cancellation/modification
- [ ] Iceberg order execution (currently just defined)

**Performance:**
- [ ] Lock-free queue (compare vs. current implementation)
- [ ] Memory pooling (reduce allocations)

**Features:**
- [ ] Multi-symbol support (manage multiple books)
- [ ] Market data replay (feed real tick data)
- [ ] ML-based trading agent (predict price movements)

**Monitoring:**
- [ ] Web-based dashboard
- [ ] Latency heatmaps by order type

---

## Why This Matters

This architecture is used by:
- **Exchanges:** NASDAQ, CME, Binance

The skills here apply to any system where **speed** and **correctness** matter: 
- Payment processing
- Gaming servers (matchmaking)
- Real-time bidding (ad auctions)

---

## Performance Highlights

| Metric | Value |
|--------|-------|
| Order insertion | ~126 ns |
| Order matching | ~65 ns |
| End-to-end latency (p50) | <10 µs |
| Throughput | 100K+ orders/sec |

**Key optimizations:**
- Pre-allocate memory (avoid reallocations)
- Batch processing (check stops every 10 trades)
- Lock-free reads where possible (atomic counters)
- Compiler flags (`-O3 -march=native`)

---

## Real-World Context

In high-frequency trading, 1 microsecond of latency can cost thousands of dollars.  This project demonstrates: 
1. How to measure and optimize latency
2. How to handle concurrency without bugs
3. How to build observable systems (dashboards, metrics)

These are the core skills needed for trading infrastructure roles.

---

## License

This project is licensed under the **MIT License**. 

You are free to use, modify, and distribute this code for personal or commercial purposes.  See the [LICENSE](LICENSE) file for full details. 

---

## Contact

**Kathan Parikh**  
[GitHub](https://github.com/KathanParikh) • [LinkedIn](https://linkedin.com/in/kathan2210)

*Built to learn how financial markets work at the systems level.*
