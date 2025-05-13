#include "MarketDataFileReader.h"
#include "CoreMessages.h"
#include "Order.h"
#include <chrono>
//#include "MarketDataLineParser.h"
#include <fstream>
#include <iostream>
namespace MarketData 
{
const auto parseActionLine=[](std::string& line) -> MarketData::Actions
{
    auto pos = line.find(' ');
    if(pos == std::string::npos)
    {
        if(line == "PRINT")
        {
            return MarketData::Actions::PRINT;
        }
        return MarketData::Actions::INVALID;
    }
    std::string action = line.substr(0, pos);
    line = line.substr(pos + 1);
    if(action == "BUY")
    {
        return MarketData::Actions::BUY;
    }
    else if(action == "SELL")
    {
        return MarketData::Actions::SELL;
    }
    else if(action == "CANCEL")
    {
        return MarketData::Actions::CANCEL;
    }
    else if(action == "MODIFY")
    {
        return MarketData::Actions::MODIFY;
    }
    else if(action == "PRINT")
    {
        return MarketData::Actions::PRINT;
    }
    return MarketData::Actions::INVALID;
};

template<typename T>
bool fromString(const std::string& is, T&);

template<typename T>
bool fromString(const std::string& is, Side s, OrderPtr);

template<>
inline bool fromString<MarketData::OrderType>(const std::string& s, OrderType& ot)
{
    if(s == "IOC")
    {
        ot = OrderType::IOC;
        return true;
    }
    if(s == "GFD")
    {
        ot = OrderType::GFD;
        return true;
    }
    return false;
}

template<>
inline bool fromString<Side>(const std::string& s, Side& side)
{
    if(s == "BUY")
    {
        side = Side::BID;
        return true;
    }
    if(s == "SELL")
    {
        side = Side::ASK;
        return true;
    }
    return false;
}

inline bool fromString(const std::string& input_string, Side side, OrderPtr& order)
{
    //  Read attributes from string
    std::string s;
    std::size_t start_pos = 0;
    auto end_pos = input_string.find_first_of(' ');
    if(end_pos == std::string::npos)
    {
        return false;
    }

    OrderType order_type;
    s = input_string.substr(start_pos, end_pos);
    if(!fromString<OrderType>(s, order_type))
    {
        return false;
    }

    //Price price;
    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    double price = std::stod(s.c_str());
    FixedPrecisionPrice<uint64_t, 6> fp{price};

    Quantity quantity;
    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    quantity = std::stoul(s);

    start_pos = end_pos + 1;
    s = input_string.substr(start_pos);
    OrderID order_id = stoull(s);
    uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();

    order = std::make_shared<Order>(order_type, side, order_id, fp, quantity, creation_time_ns);
    return true;
}

bool cancelFromString(const std::string& input_string, CoreMessage& cm)
{
    if(input_string.empty()) {
        return false;
    }

    std::size_t start_pos = 0;
    auto end_pos = input_string.find_first_of(' ');
    if(end_pos == std::string::npos)
    {
        return false;
    }
    std::string s = input_string.substr(start_pos, end_pos);

    for(auto c: s) {
        if(!isascii(c)) {
            return false;
        }
    }
    InstrumentID inst_id = std::stoull(s);

    start_pos = end_pos + 1;
    s = input_string.substr(start_pos);
    for(auto c: s) {
        if(!isascii(c)) {
            return false;
        }
    }
    OrderID order_id = std::stoull(s);
    cm.data.cancel_order = {inst_id, order_id};

    return true;
}

// template<>
// inline bool fromString<ModifyOrder>(const std::string& input_string, ModifyOrder& order)
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

/////////////////////////   Generic parser for any CoreMessage //////////////////////////
bool orderFromString(const std::string& input_string, Side side, CoreMessage& coreMessage)
{
    //  Read attributes from string
    std::string s;
    std::size_t start_pos = 0;
    auto end_pos = input_string.find_first_of(' ');
    if(end_pos == std::string::npos)
    {
        return false;
    }

    OrderType order_type;
    s = input_string.substr(start_pos, end_pos);
    if(!fromString<OrderType>(s, order_type))
    {
        return false;
    }

    InstrumentID id;
    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    id = std::stoul(s);

    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    double price = std::stod(s.c_str());
    FixedPrecisionPrice<uint64_t, 6> fp{price};

    Quantity quantity;
    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    quantity = std::stoul(s);

    start_pos = end_pos + 1;
    s = input_string.substr(start_pos);
    OrderID order_id = stoi(s);
    uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();

    coreMessage.data.add_order = {order_type, id, order_id, side, fp, quantity, creation_time_ns};
    coreMessage.data_type = DataType::ADD_ORDER;
    return true;
}

bool modifyFromString(const std::string& input_string, CoreMessage& coreMessage)
{
    //  Read attributes from string
    std::string s;
    std::size_t start_pos = 0;
    auto end_pos = input_string.find_first_of(' ');
    if(end_pos == std::string::npos)
    {
        return false;
    }

    s = input_string.substr(start_pos, end_pos - start_pos);
    InstrumentID id = std::stoul(s);

    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    OrderID order_id = stoul(s);

    Side side;
    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    if(!fromString<Side>(s, side))
    {
        return false;
    }

    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    double price = std::stod(s.c_str());
    FixedPrecisionPrice<uint64_t, 6> fp{price};

    Quantity quantity;
    start_pos = end_pos + 1;
    s = input_string.substr(start_pos);
    quantity = std::stoul(s);


    uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();
    coreMessage.data.update_order = {order_id, id, side, fp, quantity, creation_time_ns};
    coreMessage.data_type = DataType::UPDATE_ORDER;
    return true;
}

bool symbolFromString(const std::string& input_string, CoreMessage& coreMessage)
{
    //  Read attributes from string
    std::string s;
    std::size_t start_pos = 0;
    auto end_pos = input_string.find_first_of(' ');
    if(end_pos == std::string::npos)
    {
        return false;
    }
    s = input_string.substr(start_pos, end_pos - start_pos);
    InstrumentID id = std::stoul(s);

    start_pos = end_pos + 1;
    end_pos = input_string.find_first_of(' ', start_pos);
    s = input_string.substr(start_pos, end_pos - start_pos);
    double price = std::stod(s.c_str());

    FixedPrecisionPrice<uint64_t, 6> fp{price};

    start_pos = end_pos + 1;
    std::string sym = input_string.substr(start_pos);

    uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();
    coreMessage.data.symbol = {sym, id, fp};
    coreMessage.data_type = DataType::SYMBOL;
    return true;
}

std::vector<Packet> FileReader::loadDataFile(const std::string& path) {
    std::vector<Packet> packets;
    std::ifstream in_file(path);
    if(!in_file.is_open()) {
        return packets;
    }
  //  Read each line
  std::string line;
  Packet cur_packet;
  while(std::getline(in_file, line)) {
    Actions action = parseActionLine(line);
    switch(action)
    {
      case Actions::SYMBOL: 
      {
        CoreMessage cm{};
        if(symbolFromString(line, cm)) {
            if(cur_packet.CanAddMessage()) {
              cur_packet.AddMessage(std::move(cm));
            } else {
              packets.push_back(cur_packet);
              cur_packet.clear();
              cur_packet.AddMessage(std::move(cm));
            }
          }
          break;
        }
        case Actions::BUY:
        case Actions::SELL:
        {
          CoreMessage cm{};
          if(orderFromString(line, action == Actions::BUY ? Side::BID : Side::ASK, cm)) {
              if(cur_packet.CanAddMessage()) {
                cur_packet.AddMessage(std::move(cm));                    
              } else {
                packets.push_back(cur_packet);
                cur_packet.clear();
                cur_packet.AddMessage(std::move(cm));
              }
          }
          break;
        }
          case Actions::MODIFY:
          {
              CoreMessage cm{};
              if(modifyFromString(line, cm)) {
                if(cur_packet.CanAddMessage()) {
                  cur_packet.AddMessage(std::move(cm));
                } else {
                  packets.push_back(cur_packet);
                  cur_packet.clear();
                  cur_packet.AddMessage(std::move(cm));
                }
              }
              break;
          }
          case Actions::CANCEL:
          {
              OrderID order_id;
              InstrumentID instrument_id;
              CoreMessage cm{};
              if(cancelFromString(line, cm))
              {
                if(cur_packet.CanAddMessage()) {
                  cur_packet.AddMessage(std::move(cm));
                } else {
                  packets.push_back(cur_packet);
                  cur_packet.clear();
                  cur_packet.AddMessage(std::move(cm));
                }
              } 
              break;
          }
          default:
              break;
      }
  }
    if(cur_packet.MessageCount()) {
        packets.push_back(cur_packet);
    }
    in_file.close();

    return packets;
}

}

