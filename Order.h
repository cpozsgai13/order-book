#ifndef _ORDER_H_
#define _ORDER_H_

#include "MarketDataDefinitions.h"
#include <vector>
#include <queue>
#include <memory>
#include <list>
#include <type_traits>
#include <cmath>

namespace MarketData
{


class Order
{
public:
    Order(OrderType t, Side s, OrderID oid, Price p, Quantity q, Timestamp time):
        order_type(t),
        side(s),
        order_id(oid),
        price(p),
        initial_quantity(q),
        remaining_quantity(q),
        creation_time_ns(time)
    {
    }

    OrderType GetOrderType() const
    {
        return order_type;
    }

    Side GetSide() const
    {
        return side;
    }
    
    OrderID GetOrderID() const
    {
        return order_id;
    }

    Price GetPrice() const
    {
        return price;
    }

    Quantity GetInitialQuantity() const
    {
        return initial_quantity;
    }

    Quantity GetRemainingQuantity() const
    {
        return remaining_quantity;
    }

    Quantity GetFilledQuantity() const
    {
        return initial_quantity - remaining_quantity;
    }
    
    uint64_t GetCreationTime() const
    {
        return order_id;
    }

    bool Fill(Quantity q)
    {
        if(q > GetRemainingQuantity())
        {
            return false;
        }
        else
        {
            remaining_quantity -= q;
        }
        return true;
    }

    bool Filled() const
    {
        return remaining_quantity == 0;
    }
private:
    OrderType order_type;
    Side side;
    OrderID order_id;
    Price price;
    Quantity initial_quantity;
    Quantity remaining_quantity;
    Timestamp creation_time_ns;
};

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::vector<OrderPtr>; 

struct OrderCompareBid {
    bool operator()(OrderPtr p1, OrderPtr p2) {
        return p1->GetPrice() < p2->GetPrice();
    }
};

struct OrderCompareAsk {
    bool operator()(const OrderPtr p1, const OrderPtr p2) const {
        return p1->GetPrice() > p2->GetPrice();
    }
};

}

#endif