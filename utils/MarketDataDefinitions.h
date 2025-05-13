#ifndef _MD_DEFINITIONS_H_
#define _MD_DEFINITIONS_H_

#include <vector>
#include <memory>
#include <list>
#include "FixedPrecisionPrice.h"
#include "Constants.h"
namespace MarketData
{
using InstrumentID = uint64_t;
using Timestamp = uint64_t;
using OrderID = uint64_t;
using RawPrice = uint64_t;
using Price = FixedPrecisionPrice<RawPrice, 6>;
using Quantity = uint64_t;
using Volume = uint64_t;

enum class Actions : uint8_t
{
    BUY = 0,
    SELL = 1,
    CANCEL = 2,
    MODIFY = 3,
    PRINT = 4,
    SYMBOL = 5,
    INVALID = 5
};

enum class Side : uint8_t
{
    BID = 0,
    ASK = 1,
    INVALID = 2
};

enum class OrderType : uint8_t
{
    IOC = 0,    // Immediate or Cancel
    GFD = 1,    // Good for Day
    MARKET = 2, // Market order - price = 0, fill at Top of Book
    INVALID = 3
};

// static std::string ToString(OrderType t)
// {
//     switch(t)
//     {
//         case OrderType::GFD:
//         {
//             return "GFD";
//         }
//         case OrderType::IOC:
//         {
//             return "GFD";
//         }
//         case OrderType::MARKET:
//         {
//             return "Market";
//         }
//         default:
//             break;
//     }
//     return "INVALID";
// }

}

#endif