#ifndef _MARKET_DATA_FILE_READER_HPP_
#define _MARKET_DATA_FILE_READER_HPP_

// #include <chrono>
// #include <string>
// #include "MarketDataObjects.h"

// namespace MatchEngine
// {

// template<typename T>
// bool fromString(const std::string& is, T&);

// template<>
// bool fromString<MatchEngine::OrderType>(const std::string& s, OrderType& ot)
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
// bool fromString<Side>(const std::string& s, Side& side)
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

// template<>
// bool fromString<BuyOrder>(const std::string& input_string, BuyOrder& order)
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

//     uint32_t price;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     price = std::stoul(s);

//     uint32_t quantity;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     quantity = std::stoul(s);

//     OrderID order_id;
//     start_pos = end_pos + 1;
//     s = input_string.substr(start_pos);
//     order_id = s;  //   If change type of order id this must be updated.

//     order = BuyOrder(order_type, price, quantity, order_id);
//     return true;
// }

// template<>
// bool fromString<SellOrder>(const std::string& input_string, SellOrder& order)
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

//     uint32_t price;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     price = std::stoul(s);

//     uint32_t quantity;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     quantity = std::stoul(s);

//     OrderID order_id;
//     start_pos = end_pos + 1;
//     s = input_string.substr(start_pos);
//     order_id = s;  //   If change type of order id this must be updated.

//     order = SellOrder(order_type, price, quantity, order_id);
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

//     OrderID order_id;
//     s = input_string.substr(start_pos, end_pos);
//     order_id = s;

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

//     uint32_t price;
//     start_pos = end_pos + 1;
//     end_pos = input_string.find_first_of(' ', start_pos);
//     if(end_pos == std::string::npos)
//     {
//         return false;
//     }
//     s = input_string.substr(start_pos, end_pos - start_pos);
//     price = std::stoul(s);

//     uint32_t quantity;
//     start_pos = end_pos + 1;
//     s = input_string.substr(start_pos);
//     quantity = std::stoul(s);

//     order = ModifyOrder(order_id, side, price, quantity);
//     return true;
// }

//}

#endif