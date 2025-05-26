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

using OrderIterator = std::list<OrderPtr>::iterator;
class OrderQueue {
public:
    void push_back(OrderPtr ptr) {
      orders.push_back(ptr);
      order_map.insert(std::make_pair(ptr->GetOrderID(), std::prev(orders.end())));
    }

    void erase(OrderPtr ptr) {
      auto order_id = ptr->GetOrderID();
      auto order_iter = order_map.find(order_id);
      if(order_iter != order_map.end()) {
        orders.erase(order_iter->second);
        order_map.erase(order_id);
      }
    }

    bool empty() const {
      return order_map.empty();
    }

    OrderPtr front() {
      return orders.front();
    }

    void pop_front() {
      //  First remove from the map then the list.
      auto order_id = front()->GetOrderID();
      order_map.erase(order_id);
      orders.pop_front();
    }
private:
    std::unordered_map<OrderID, OrderIterator> order_map;
    std::list<OrderPtr> orders;
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
    friend std::ostream& operator<<(std::ostream&os, OrderBook& book);
    bool empty() const {
      return order_map.empty();
    }
private:
    std::unordered_map<OrderID, OrderPtr> order_map;
    std::unordered_map<Price, Volume, KeyHash<uint64_t, 6>> bid_volume_map;
    std::unordered_map<Price, Volume, KeyHash<uint64_t, 6>> ask_volume_map;
    std::map<Price, OrderQueue, std::greater<Price>> bid_queue_map;
    std::map<Price, OrderQueue, std::less<Price>> ask_queue_map;

    Trades trades;

    bool CanMatch(Side s, Price p);
    void MatchOrders();
    void MatchIOCOrder(OrderPtr ptr);

    Symbol identity;
};


}
#endif
