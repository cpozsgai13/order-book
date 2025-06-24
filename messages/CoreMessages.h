#ifndef _CORE_MESSAGES_H_
#define _CORE_MESSAGES_H_

#include "MarketDataDefinitions.h"
#include "Constants.h"
#include <cstdint>
#include <cstddef>
#include <memory.h>
#include <iostream>

namespace MarketData 
{

enum class DataType : uint8_t {
  SYMBOL = 0,
  ADD_ORDER = 1,
  UPDATE_ORDER = 2,
  CANCEL_ORDER = 3,
  TRADE = 4,
  INVALID = 5
};

constexpr size_t SYMBOL_MAX_LEN = 20;
constexpr size_t PRICE_LEN = sizeof(FixedPrecisionPrice<uint64_t, 6>);
constexpr size_t SYMBOL_SIZE = SYMBOL_MAX_LEN + PRICE_LEN;

const auto formatText=[](char *buffer, size_t max_len = SYMBOL_MAX_LEN) -> std::string {
	std::string ret;
	size_t i = 0;
	while(buffer[i] != 0x00 && i < max_len) {
		ret.push_back(buffer[i]);
		++i;
	}
	return ret;
};

#pragma pack(push, 1)
struct Header {
    uint16_t num_messages{0};
    uint16_t total_length{0};

    void clear() {
        num_messages = 0;
        total_length = 0;
    }
};

struct Symbol {
    Symbol()
    {
        memset(symbol, 0, SYMBOL_MAX_LEN);
    }
    ~Symbol() = default;
    Symbol(const Symbol& s) {
        *this = s;
    }
    Symbol(const std::string& sym, InstrumentID id, FixedPrecisionPrice<uint64_t, 6> price):
    instrument_id(id),
    last_price(price)
    {
      memset(symbol, 0, SYMBOL_MAX_LEN);
      strncpy(symbol, sym.c_str(), sym.length());
    }
    Symbol& operator=(const Symbol& s) {
      instrument_id = s.instrument_id;
      memset(symbol, 0, SYMBOL_MAX_LEN);
      strncpy(symbol, s.symbol, SYMBOL_MAX_LEN);
      last_price = s.last_price;
      return *this;
    }
    char symbol[SYMBOL_MAX_LEN];
    InstrumentID instrument_id{0};
    FixedPrecisionPrice<uint64_t, 6> last_price{0.0};
};

struct AddOrder {
    AddOrder() = default;
    ~AddOrder() = default;

    AddOrder(OrderType t, InstrumentID inst_id, OrderID oid, Side s, FixedPrecisionPrice<uint64_t, 6> p, Quantity q, uint64_t creation_time_ns):
        instrument_id(inst_id),
        order_id(oid),
        price(p.rawValue()),
        quantity(q),
        side(s),
        order_type(t),
        update_time_ns(creation_time_ns)
    {
    }

    OrderID GetOrderID() const {
        return order_id;
    }
    size_t instrument_id{0};
    OrderID order_id{0};
    FixedPrecisionPrice<uint64_t, 6> price{0.0};
    MarketData::Quantity quantity;
    MarketData::Side side;
    OrderType order_type;
    Timestamp update_time_ns{0};
};

struct ModifyOrder {
    ModifyOrder() = default;
    ~ModifyOrder() = default;
    ModifyOrder(const ModifyOrder& mo) = default;
    ModifyOrder(OrderID oid, InstrumentID inst_id, Side s, FixedPrecisionPrice<uint64_t, 6> p, Quantity q, uint64_t update_time_ns):
        order_id(oid),
        instrument_id(inst_id),
        price(p.rawValue()),
        side(s),
        quantity(q),
        update_time_ns(update_time_ns)
    {
    }

    OrderID GetOrderID() const {
        return order_id;
    }

    RawPrice ToPrice() const {
        return price;
    }
    OrderID order_id{0};
    InstrumentID instrument_id{0};
    FixedPrecisionPrice<uint64_t, 6> price{0.0};
    MarketData::Side side;
    Quantity quantity;
    Timestamp update_time_ns{0};
};

struct CancelOrder {
    CancelOrder(InstrumentID inst_id, OrderID id): 
    order_id(id),
    instrument_id(inst_id)
    {
    }
    OrderID order_id{0};
    InstrumentID instrument_id{0};
};

struct TradeExecution {
    TradeExecution(InstrumentID inst_id, OrderID sell_id, OrderID buy_id): 
    sell_order_id(sell_id),
    buy_order_id(buy_id),
    instrument_id(inst_id)
    {
    }
    OrderID sell_order_id{0};
    OrderID buy_order_id{0};
    InstrumentID instrument_id{0};
};

// struct alignas(CACHE_LINE_SIZE) CoreMessage {
struct CoreMessage {
    CoreMessage() = default;
    ~CoreMessage() {

    }
    CoreMessage& operator=(const CoreMessage& cm) {
        memcpy(&data, &cm.data, sizeof(data));
        data_type = cm.data_type;
        return *this;
    }
    CoreMessage(const CoreMessage& cm) {
        *this = cm;
    }
    union Message {
        Symbol symbol;
        AddOrder add_order;
        ModifyOrder update_order;
        CancelOrder cancel_order;
        Message(){}
    };
    DataType getDataType() const {
        return data_type;
    }
    DataType data_type{DataType::INVALID};
    Message data;
};

struct Packet {
    struct PacketStruct {
        Header header;
        CoreMessage messages[(UDP_BUFFER_SIZE- sizeof(Header))/sizeof(CoreMessage)];
        PacketStruct() {
            header.total_length = 0;
            header.num_messages = 0;
        }
    };
    static constexpr size_t MESSAGES_PER_PACKET = (UDP_BUFFER_SIZE- sizeof(Header))/sizeof(CoreMessage);
    union PacketData
    {
        char buffer[UDP_BUFFER_SIZE]={0};
        PacketStruct packet;

        PacketData():packet()
        {
            memset(buffer, 0, UDP_BUFFER_SIZE);
        }
        ~PacketData(){}
        PacketData& operator=(const PacketData& other) {
            memcpy(buffer, other.buffer, other.packet.header.total_length);
            return *this;
        }
    };
    PacketData data;

    Packet():data() {
    }

    ~Packet() {

    }

    Packet(const Packet& other) {
        // header = other.header;
        data = other.data;
    }

    bool CanAddMessage() const {
        return data.packet.header.total_length < UDP_BUFFER_SIZE - sizeof(CoreMessage);
        //return header.total_length < UDP_BUFFER_SIZE - sizeof(CoreMessage);
    }

    bool AddMessage(CoreMessage& msg) {
        auto sz_msg = sizeof(msg);
        auto cur_sz = GetSize();

        if(sz_msg + cur_sz <= UDP_BUFFER_SIZE) {
            data.packet.messages[data.packet.header.num_messages] = msg;
            //data.packet.messages[header.num_messages] = std::move(msg);
            //header.num_messages++;
            //header.total_length = sz_msg + cur_sz;
            data.packet.header.num_messages++;
            data.packet.header.total_length = sz_msg + cur_sz;
            return true;
        }
        return false;
    };

    size_t GetSize() {
        return data.packet.header.total_length;
        //return header.total_length;
    }

    size_t MessageCount() {
        return data.packet.header.num_messages;
        //return header.num_messages;
    }
    void clear() {
        data.packet.header.clear();
    }

    static bool FromBuffer(char *buffer, size_t len, Packet& packet) {
        if(len < sizeof(Header)) {
            return false;
        }
        packet.data.packet.header = *reinterpret_cast<Header*>(buffer);
        memcpy(&packet.data.packet.header, buffer, len);
        return true;
    }
};

#pragma pack(pop)
struct SymbolComp {
    bool operator()(const Symbol& s1, const Symbol& s2) {
        return memcmp(s1.symbol, s2.symbol, SYMBOL_MAX_LEN) == 0;
    }
};

struct ModifyOrderComp {
    bool operator()(const ModifyOrder& s1, const ModifyOrder& s2) {
        return s1.instrument_id == s2.instrument_id && s1.order_id == s2.order_id;
    }
};

struct AddOrderComp {
    bool operator()(const AddOrder& s1, const AddOrder& s2) {
        return s1.instrument_id == s2.instrument_id && s1.order_id == s2.order_id;
    }
};

}

#endif