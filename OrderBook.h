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
#include <functional>

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

//  Custom OrderQueue class:  Given the fact that a priority_queue can only
//  pop the maximum from the front of the container, override the top method
//  to drop any items that are not in the order map (those orders were canceled)
using OrderComparator = std::function<bool(const OrderPtr& lhs, const OrderPtr& rhs)>;

struct QueueTimeComparator {
  bool operator()(const OrderPtr lhs, const OrderPtr rhs) {
    return lhs->GetCreationTime() > rhs->GetCreationTime();
  }
};

const auto QueueOrderIDComparator = [](const OrderPtr lhs, const OrderPtr rhs) -> bool {
  return lhs->GetOrderID() < rhs->GetOrderID();
};

class CustomOrderQueue {
public:
  CustomOrderQueue(OrderComparator cmp):
    order_cmp(cmp) {
  }
  CustomOrderQueue() = default;
  ~CustomOrderQueue() = default;

  void push_back(OrderPtr ptr) {
    order_queue.push(ptr);
    order_map[ptr->GetOrderID()] = ptr;
  }

  //  Simply remove from the map.  The top will not return
  //  an item from the priority queue until it is found in the
  //  order map;
  void erase(OrderPtr ptr) {
    auto order_id = ptr->GetOrderID();
    auto order_iter = order_map.find(order_id);
    if(order_iter != order_map.end()) {
      order_map.erase(order_id);
    }
  }

  bool empty() const {
    return order_map.empty();
  }

  //  This method must ignore any items in the priority queue
  //  that are not in the order map b/c priority queue only can
  //  access the top of the heap.
  OrderPtr front() {
    if(empty()) {
      return nullptr;
    }

    bool in_map = false;
    OrderPtr ptr{nullptr};

    while(!in_map) {
      ptr = order_queue.top();
      auto order_id = ptr->GetOrderID();
      auto count = order_map.count(order_id);
      if(count > 0) {
        in_map = true;
      } else {
        order_queue.pop();
      }
    }
    return in_map ? ptr : nullptr;
  }

  void pop_front() {
    //  First remove from the map then the list.
    auto order_id = front()->GetOrderID();
    order_map.erase(order_id);
    order_queue.pop();
  }

private:
  std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderComparator> order_queue;
  std::unordered_map<OrderID, OrderPtr> order_map;
  OrderComparator order_cmp;
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
    std::map<Price, CustomOrderQueue, std::greater<Price>> bid_queue_map;
    std::map<Price, CustomOrderQueue, std::less<Price>> ask_queue_map;

    Trades trades;
    bool CanMatch(Side s, Price p);
    void MatchOrders();
    void MatchIOCOrder(OrderPtr ptr);

    Symbol identity;
};


}
#endif
