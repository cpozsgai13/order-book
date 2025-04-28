#ifndef _EXCHANGE_ORDER_BOOK_H
#define _EXCHANGE_ORDER_BOOK_H

#include "OrderBook.h"
#include <unordered_map>
#include <string.h>
#include <boost/container/flat_map.hpp>

namespace MatchEngine
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
    bool AddOrderBook(const FlatString& key, OrderBook&& order_book) {
        instrument_map[key] = order_book;
    }

    bool AddOrder(const FlatString& key, OrderPtr order) {
        instrument_map[key].AddOrder(order);
    }

    bool UpdateOrder(const FlatString& key, ModifyOrder& order) {
        instrument_map[key].UpdateOrder(order);
    }

    bool CancelOrder(const FlatString& key, OrderID orderID) {
        instrument_map[key].CancelOrder(orderID);
    }

private:
    boost::container::flat_map<FlatString, OrderBook, FlatStringComparator> instrument_map;
};

}
#endif