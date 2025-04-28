#ifndef _MATCH_ORDER_ENGINE_H_
#define _MATCH_ORDER_ENGINE_H_

#include "OrderBook.h"
#include <unordered_map>

namespace MarketData
{

class MatchOrderEngine
{
public:
    MatchOrderEngine();
    void Initialize();
    bool AddOrder(OrderPtr order);
    bool UpdateOrder(const ModifyOrder& order);

    void PrintBook();
private:
    std::shared_ptr<OrderBook> order_book;
};


}

#endif