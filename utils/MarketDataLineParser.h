#ifndef _MD_LINE_PARSER_H_
#define _MD_LINE_PARSER_H_

#include "MarketDataDefinitions.h"
#include "CoreMessages.h"
#include "Order.h"
#include <chrono>

namespace MarketData
{

// template<typename T>
// bool fromString(const std::string& is, T&);

// template<typename T>
// inline bool fromString(const std::string& is, Side s, OrderPtr);

// template<>
// inline bool fromString<MarketData::OrderType>(const std::string& s, OrderType& ot)
// {
//     if(s == "IOC")
//     {
//         ot = OrderType::IOC;
//         return true;
//     }
//     if(s == "GFD")
//     {
//         ot = OrderType::GFD;
//         return true;
//     }
//     return false;
// }

// template<>
// inline bool fromString<Side>(const std::string& s, Side& side)
// {
//     if(s == "BUY")
//     {
//         side = Side::BID;
//         return true;
//     }
//     if(s == "SELL")
//     {
//         side = Side::ASK;
//         return true;
//     }
//     return false;
// }

// inline bool fromString(const std::string& input_string, Side side, OrderPtr& order)
// {
//     //  Read attributes from string
//     std::string s;
//     std::size_t start_pos = 0;
//     auto end_pos = input_string.find_first_of(' ');
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }

//     OrderType order_type;
//     s = input_string.substr(start_pos, end_pos);
//     if(!fromString<OrderType>(s, order_type))
//     {
//         return false;
//     }

//     Price price;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     price = std::stoul(s);

//     Quantity quantity;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     quantity = std::stoul(s);

//     start_pos = end_pos + 1;
//     s = input_string.substr(start_pos);
//     OrderID order_id = stoi(s);
//     uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();

//     order = std::make_shared<Order>(order_type, side, order_id, price, quantity, creation_time_ns);
//     return true;
// }

// template<>
// bool fromString<OrderID>(const std::string& input_string, OrderID& order_id)
// {
//     if(input_string.empty()) {
//         return false;
//     }

//     for(auto c: input_string) {
//         if(!isascii(c)) {
//             return false;
//         }
//     }

//     order_id = std::stoull(input_string);
//     return true;
// }

// template<>
// bool fromString<ModifyOrder>(const std::string& input_string, ModifyOrder& order)
// {
//     //  Read attributes from string
//     std::string s;
//     std::size_t start_pos = 0;
//     auto end_pos = input_string.find_first_of(' ');
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }

//     s = input_string.substr(start_pos, end_pos - start_pos);
//     OrderID order_id = stoi(s);

//     Side side;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     if(!fromString<Side>(s, side))
//     {
//         return false;
//     }

//     Price price;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     price = std::stoul(s);

//     Quantity quantity;
//     start_pos = end_pos + 1;
//     s = input_string.substr(start_pos);
//     quantity = std::stoul(s);


//     uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();
//     order = ModifyOrder(order_id, side, price, quantity, creation_time_ns);
//     return true;
// }
    
// const auto parseActionLine=[](std::string& line) -> MarketData::Actions
// {
//     auto pos = line.find(' ');
//     if(pos == std::string::npos)
//     {
//         if(line == "PRINT")
//         {
//             return MarketData::Actions::PRINT;
//         }
//         return MarketData::Actions::INVALID;
//     }
//     std::string action = line.substr(0, pos);
//     line = line.substr(pos + 1);
//     if(action == "BUY")
//     {
//         return MarketData::Actions::BUY;
//     }
//     else if(action == "SELL")
//     {
//         return MarketData::Actions::SELL;
//     }
//     else if(action == "CANCEL")
//     {
//         return MarketData::Actions::CANCEL;
//     }
//     else if(action == "MODIFY")
//     {
//         return MarketData::Actions::MODIFY;
//     }
//     else if(action == "PRINT")
//     {
//         return MarketData::Actions::PRINT;
//     }
//     return MarketData::Actions::INVALID;
// };



}

#endif