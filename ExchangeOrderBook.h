#ifndef _EXCHANGE_ORDER_BOOK_H
#define _EXCHANGE_ORDER_BOOK_H

#include "OrderBook.h"
#include "CoreMessages.h"
#include <unordered_map>
#include <string.h>
#include <string>
#include <boost/container/flat_map.hpp>

namespace MarketData
{

static constexpr size_t FLAT_STRING_SIZE = 20;
using FlatString = char[FLAT_STRING_SIZE];

struct FlatStringComparator {
    bool operator()(const FlatString& f1, const FlatString& f2) {
        return memcmp(&f1, &f2, FLAT_STRING_SIZE) == 0;
    }
};

class ExchangeOrderBook {
public:
    ExchangeOrderBook(const std::string& name);
    ~ExchangeOrderBook() = default;
    // bool AddOrderBook(const FlatString& key, OrderBook&& order_book) {
    //     instrument_map[key] = order_book;
    // }

    bool AddNewOrder(InstrumentID inst_id, AddOrder& order);

    bool UpdateOrder(InstrumentID inst_id, ModifyOrder& order);

    bool CancelOrder(InstrumentID inst_id, OrderID orderID);

    bool AddUpdateSymbol(Symbol& symbol);

    bool PrintBook(const std::string& symbol);
private:
    boost::container::flat_map<InstrumentID, std::unique_ptr<OrderBook>> instrument_map;
    std::unordered_map<std::string, InstrumentID> symbol_map;
    std::string exchange_name;
};

}
#endif
