#ifndef ORDERQUEUE_HPP
#define ORDERQUEUE_HPP

#include <bits/stdc++.h>
#include "order.hpp"
using namespace std;

class OrderQueue {
private:
    queue<Order> queue;
    mutex mtx;
    condition_variable cv;
    bool finished = false;

public:
    // PRODUCER calls this
    void push(const Order& order) {
        {
            // Lock the door
            lock_guard<mutex> lock(mtx);
            queue.push(order);
        } // Lock releases automatically
        
        // Wake up the consumer!
        cv.notify_one();
    }

    // CONSUMER calls this
    // Returns true if we got an order, false if we should shut down
    bool pop(Order& order) {
        unique_lock<mutex> lock(mtx);
        
        // Wait until queue is NOT empty OR we are finished
        // This prevents "busy waiting" (burning CPU)
        cv.wait(lock, [this] { return !queue.empty() || finished; });

        if (queue.empty() && finished) {
            return false; // Time to stop
        }

        order = queue.front();
        queue.pop();
        return true;
    }

    // Signal that no more orders are coming
    void stop() {
        {
            lock_guard<mutex> lock(mtx);
            finished = true;
        }
        cv.notify_all(); // Wake everyone up so they can exit
    }
};

#endif