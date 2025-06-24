#ifndef _ORDER_H_
#define _ORDER_H_

#include "MarketDataDefinitions.h"
#include "CoreMessages.h"
#include <vector>
#include <memory>
#include <list>
#include <type_traits>

namespace MarketData
{
//using Price = FixedPrecisionPrice<RawPrice, 6>;

class Order
{
public:
    Order(OrderType t, Side s, OrderID oid, FixedPrecisionPrice<uint64_t, 6> p, Quantity q, Timestamp time):
        order_type(t),
        side(s),
        order_id(oid),
        price(p.rawValue()),
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

std::ostream& operator<<(std::ostream& os, const FixedPrecisionPrice<uint64_t, 6>& price);
std::ostream& PrintBid(std::ostream& os, const Volume& vol, const FixedPrecisionPrice<uint64_t, 6>& price, int precision);
std::ostream& PrintAsk(std::ostream& os, const Volume& vol, const FixedPrecisionPrice<uint64_t, 6>& price, int precision);

}

#endif