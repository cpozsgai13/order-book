#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <atomic>

/*
  - circular array of fixed size
  - use atomics instead of locks
  - single producer/consumer
*/
template<typename T, size_t N>
class RingBufferSPSC {
public:
    RingBufferSPSC():
    head(0),    // read (lags write)
    tail(0),     // write 
    data(N)
    {}

    bool push(const T& value) {
        //  Can use relaxed as long but requires stronger ordering
        //  when storing
        size_t cur_tail = tail.load(std::memory_order::memory_order_relaxed);

        size_t next_tail = (cur_tail + 1) % N;

        //  Check if full.
        if(next_tail == head.load(std::memory_order::memory_order_acquire)) {
            return false;
        }

        data[next_tail] = value;
        tail.store(next_tail, std::memory_order::memory_order_release);
        return true;
    }

    bool pop(T& value) {
        size_t cur_head = head.load(std::memory_order::memory_order_relaxed);

        if(cur_head == tail.load(std::memory_order::memory_order_acquire)) {
            return false;
        }

        value = data[cur_head];

        auto new_head = (cur_head + 1) % N;

        head.store(new_head, std::memory_order::memory_order_release);
        return true;
    }

    bool empty() {
        return head.load(std::memory_order::memory_order_acquire) != tail.load(std::memory_order::memory_order_acquire);
    }
private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    std::vector<T> data;
    size_t capacity{N};
};
#endif
