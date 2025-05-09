#include "Trade.h"
#include <iostream>

namespace MarketData
{
std::ostream& operator<<(std::ostream& os, const Trade& trade)
{
    os << "TRADE: Bid(" << trade.bid_side.order_id << " " << (double)trade.bid_side.price << " " << trade.bid_side.quantity \
        << "), Ask(" << trade.ask_side.order_id << " " << (double)trade.ask_side.price << " " << trade.bid_side.quantity << ")";

    return os;
}

}
