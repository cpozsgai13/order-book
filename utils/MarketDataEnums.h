#ifndef _MARKET_DATA_ENUMS_H_
#define _MARKET_DATA_ENUMS_H_
#include <string>

namespace MarketData
{
    enum class Actions : uint8_t
    {
        BUY = 0,
        SELL = 1,
        CANCEL = 2,
        MODIFY = 3,
        PRINT = 4,
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
        IOC = 0,    // Immediate or Cancel (Fill or Kill)
        GFD = 1,    // Good for Day
        INVALID = 2
    };

    static std::string ToString(OrderType t)
    {
        switch(t)
        {
            case OrderType::GFD:
            {
                return "GFD";
            }
            case OrderType::IOC:
            {
                return "GFD";
            }
            default:
                break;
        }
        return "INVALID";
    }
    
}

#endif