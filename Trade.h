#ifndef _TRADE_H_
#define _TRADE_H_

#include "MarketDataDefinitions.h"
#include "CoreMessages.h"
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

std::ostream& operator<<(std::ostream& os, const Trade& trade);
}
#endif