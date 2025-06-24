#ifndef _TCP_MEMORY_POOL_H_
#define _TCP_MEMORY_POOL_H_

#include <cstdint>
#include <cstddef>
#include <queue>
#include <array>
#include <mutex>
#include <memory>
#include "Constants.h"

namespace MarketData
{

class TCPMemoryPool {
    static constexpr size_t NUM_CONNECTIONS = 100;
public:
    using MemoryBuffer = std::array<uint8_t, sizeof(Packet)>;
    TCPMemoryPool() {
        for(size_t i = 0; i < NUM_CONNECTIONS; ++i) {
            pool.push(std::make_unique<MemoryBuffer>());
        }
    }

    std::unique_ptr<MemoryBuffer> allocate() {
        std::lock_guard<std::mutex> lock(mut);
        if(pool.empty()) {
            throw std::runtime_error("TCPMemoryPool is empty");
            //return nullptr;
        }

        auto buf = std::move(pool.front());
        pool.pop();
        return buf;
    }

    void deallocate(std::unique_ptr<MemoryBuffer> buffer) {
        std::lock_guard<std::mutex> lock(mut);
        pool.push(std::move(buffer));
    }
private:
    std::queue<std::unique_ptr<MemoryBuffer>> pool;
    std::mutex mut;
};

}
#endif