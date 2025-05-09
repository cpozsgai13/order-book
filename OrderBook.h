#ifndef _ORDER_BOOK_H
#define _ORDER_BOOK_H

#include <chrono>
#include <list>
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <queue>
#include <vector>

#include "MarketDataDefinitions.h"
#include "Order.h"
#include "CoreMessages.h"
#include "Trade.h"
namespace MarketData
{

const auto ToOrderPtr=[](MarketData::OrderType orig_type, const ModifyOrder& order) -> OrderPtr {
    return std::make_shared<Order>(orig_type, order.side, order.order_id, order.price, order.quantity, order.update_time_ns);
};

class OrderBook
{
public:
    OrderBook(Symbol& symbol);
    OrderBook();
    ~OrderBook();
    bool AddOrder(OrderPtr order); 
    bool CancelOrder(OrderID order_id);
    bool UpdateOrder(ModifyOrder& order);
    Volume GetVolumeAtPrice(Price price, Side side);
    void Print();
private:
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareBid> best_bid_map{OrderCompareBid()};
    std::unordered_set<OrderID> best_bid_skip_list;
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareAsk> best_ask_map{OrderCompareAsk()};
    std::unordered_set<OrderID> best_ask_skip_list;

    std::unordered_map<OrderID, OrderPtr> order_map;
    std::unordered_map<Price, Volume, KeyHash<uint64_t, 6>> bid_volume_map;
    std::unordered_map<Price, Volume, KeyHash<uint64_t, 6>> ask_volume_map;
    std::map<Price, OrderQueue, std::greater<Price>> bid_queue_map;
    std::map<Price, OrderQueue, std::less<Price>> ask_queue_map;

    Trades trades;

    bool CanMatch(Side s, Price p);
    void MatchOrders();

    Symbol identity;
};

}
#endif