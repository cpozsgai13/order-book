#ifndef _CORE_MESSAGES_H_
#define _CORE_MESSAGES_H_

#include "MarketDataDefinitions.h"
#include "FixedPrecisionPrice.h"
#include <cstdint>
#include <cstddef>
#include <memory.h>

namespace MarketData 
{

static constexpr size_t MAX_PACKET_SIZE = 1500;
static constexpr size_t CACHE_LINE_SIZE = 64;


struct Header {
    uint16_t num_messages{0};
    uint16_t total_length{0};

    void clear() {
        num_messages = 0;
        total_length = 0;
    }
};

struct NullMessage {
    uint16_t sz{0};
    NullMessage() = default;
};

constexpr size_t SYMBOL_MAX_LEN = 20;
constexpr size_t PRICE_LEN = sizeof(FixedPrecisionPrice<uint64_t, 6>);
constexpr size_t SYMBOL_SIZE = SYMBOL_MAX_LEN + PRICE_LEN ;
struct alignas(CACHE_LINE_SIZE) Symbol {
    Symbol() = default;
    ~Symbol() = default;
    Symbol(const Symbol& s) = default;
    Symbol(std::string sym, InstrumentID id, FixedPrecisionPrice<uint64_t, 6> price):
    instrument_id(id),
    last_price(price)
    {
        strncpy(symbol, sym.c_str(), SYMBOL_MAX_LEN);
    }
    Symbol& operator=(const Symbol& s) {
        instrument_id = s.instrument_id;
        strncpy(symbol, s.symbol, SYMBOL_MAX_LEN);
        last_price = s.last_price;
    }
    char symbol[SYMBOL_MAX_LEN];
    InstrumentID instrument_id{0};
    FixedPrecisionPrice<uint64_t, 6> last_price;
};

struct SymbolComp {
    bool operator()(const Symbol& s1, const Symbol& s2) {
        return memcmp(s1.symbol, s2.symbol, SYMBOL_MAX_LEN) == 0;
    }
};

struct alignas(CACHE_LINE_SIZE) AddOrder {
    AddOrder() = default;
    ~AddOrder() = default;

    AddOrder(OrderType t, InstrumentID inst_id, OrderID oid, Side s, FixedPrecisionPrice<uint64_t, 6> p, Quantity q, uint64_t creation_time_ns):
        order_type(t),
        instrument_id(inst_id),
        order_id(oid),
        side(s),
        price(p),
        quantity(q),
        update_time_ns(creation_time_ns)
    {
    }

    OrderID GetOrderID() const {
        return order_id;
    }
    size_t instrument_id{0};
    OrderID order_id{0};
    FixedPrecisionPrice<uint64_t, 6> price;
    MarketData::Quantity quantity;
    MarketData::Side side;
    OrderType order_type;
    Timestamp update_time_ns{0};
};

struct AddOrderComp {
    bool operator()(const AddOrder& s1, const AddOrder& s2) {
        return s1.instrument_id == s2.instrument_id && s1.order_id == s2.order_id;
    }
};

struct alignas(CACHE_LINE_SIZE) ModifyOrder {
    ModifyOrder() = default;
    ~ModifyOrder() = default;
    ModifyOrder(const ModifyOrder& mo) = default;
    ModifyOrder(OrderID oid, InstrumentID inst_id, Side s, FixedPrecisionPrice<uint64_t, 6> p, Quantity q, uint64_t creation_time_ns):
        order_id(oid),
        instrument_id(inst_id),
        side(s),
        price(p),
        quantity(q),
        update_time_ns(creation_time_ns)
    {
    }

    OrderID GetOrderID() const {
        return order_id;
    }

    Price ToPrice() const {
        return price.rawValue();
    }
    InstrumentID instrument_id{0};
    FixedPrecisionPrice<uint64_t, 6> price;
    MarketData::Side side;
    Quantity quantity;
    Timestamp update_time_ns{0};
    OrderID order_id{0};
};

struct ModifyOrderComp {
    bool operator()(const ModifyOrder& s1, const ModifyOrder& s2) {
        return s1.instrument_id == s2.instrument_id && s1.order_id == s2.order_id;
    }
};

struct alignas(CACHE_LINE_SIZE) CancelOrder {
    CancelOrder(InstrumentID inst_id, OrderID id): 
    order_id(id),
    instrument_id(inst_id)
    {
    }
    OrderID order_id{0};
    InstrumentID instrument_id{0};
};

struct alignas(CACHE_LINE_SIZE) CoreMessage {
    union Message {
        NullMessage null_message;
        Symbol symbol;
        AddOrder add_order;
        ModifyOrder update_order;
        CancelOrder cancel_order;

        Message() : null_message(){}
    };
    Message data;
};

struct Packet {
    Header header;
    std::vector<CoreMessage> messages;

    bool AddMessage(CoreMessage&& msg) {
        auto sz_msg = sizeof(msg);
        auto cur_sz = GetSize();

        if(sz_msg + cur_sz <= MAX_PACKET_SIZE) {
            messages.push_back(std::move(msg));
        }
    };

    bool AddMessage(CoreMessage& msg) {
        auto sz_msg = sizeof(msg);
        auto cur_sz = GetSize();

        if(sz_msg + cur_sz <= MAX_PACKET_SIZE) {
            messages.push_back(msg);

            header.num_messages++;
            header.total_length = sz_msg + cur_sz;
            return true;
        }
        return false;
    };

    size_t GetSize() {
        size_t sz = sizeof(header);
        sz += messages.size()*sizeof(CoreMessage);
        return sz;
    }

    size_t MessageCount() {
        return messages.size();
    }
    void clear() {
        header.clear();
        messages.clear();
    }

    bool FromBuffer(char *buffer, size_t len) {
        if(len < sizeof(Header)) {
            return false;
        }

        Header *header = reinterpret_cast<Header*>(buffer);

    }
};

}

#endif