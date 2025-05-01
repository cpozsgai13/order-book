#ifndef _TRADE_H_
#define _TRADE_H_

#include "MarketDataDefinitions.h"
#include "FixedPrecisionPrice.h"
namespace MarketData
{
struct TradeSide
{
    OrderID order_id;
    FixedPrecisionPrice<RawPrice, 6> price;
    Quantity quantity;
};

struct Trade
{
    Trade(const TradeSide& bid, const TradeSide& ask) :
        bid_side(bid),
        ask_side(ask)
    {
    }
    TradeSide bid_side;
    TradeSide ask_side;

};
using Trades = std::vector<Trade>;

// std::ostream& operator<<(std::ostream& os, const Trade& trade)
// {
//     os << "TRADE " << trade.bid_side.order_id << " " << trade.bid_side.price << " " << trade.bid_side.quantity \
//         << " " << trade.ask_side.order_id << " " << trade.bid_side.price << " " << trade.bid_side.quantity;
//     return os;
// }

}
#endif