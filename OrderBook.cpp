#include "OrderBook.h"
#include "CoreMessages.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "rtdsc.h"

namespace MarketData
{

auto printTrade = [](const Trade& trade) {
  std::cout << trade << std::endl;
};

auto formatTimestampUTC = [](std::chrono::system_clock::time_point tp) -> std::string {
  time_t t = std::chrono::system_clock::to_time_t(tp);
  struct tm *ts = gmtime(&t);

  std::stringstream ss;
  ss << std::put_time(ts, "%b %d %Y %H:%M:%S");
  return ss.str();
};

OrderBook::OrderBook(Symbol& sym): 
  identity(sym)
{
}

OrderBook::OrderBook()
{
}

OrderBook::~OrderBook()
{
}

bool OrderBook::AddOrder(OrderPtr order)
{
  if(!order) {
      return false;
  }
    
  OrderID order_id = order->GetOrderID();
  if(order_map.count(order_id) > 0)
  {
      return false;
  }
   
  auto price = order->GetPrice();
  Quantity q = order->GetInitialQuantity();
  if(order->GetOrderType() == OrderType::IOC)
  {
      if(!CanMatch(order->GetSide(), price))
      {
          return false;
      }
      MatchIOCOrder(order);
      return true;
  }

  OrderQueue order_queue;
  if(order->GetSide() == Side::BID)
  {
      auto& orders = bid_queue_map[price];

      //  There are no orders at this price.
      if(orders.empty()) {
          bid_volume_map.insert(std::make_pair(price, q));
      } else {
          //  Lookup the price and add the volume.
          bid_volume_map[price] += q;
      }

      orders.push_back(order);
  }
  else if(order->GetSide() == Side::ASK)
  {
      auto& orders = ask_queue_map[price];
      if(orders.empty()) {
          ask_volume_map.insert(std::make_pair(price, q));
      } else {
          ask_volume_map[price] += q;
      }

      orders.push_back(order);
  }

  order_map.insert(std::make_pair(order_id, order));

  MatchOrders();
  return true;
}

bool OrderBook::CancelOrder(OrderID order_id)
{
    if(order_map.count(order_id) == 0)
    {
        return false;
    }

    const auto& order = order_map.at(order_id);
    Side side = order->GetSide();
    auto price = order->GetPrice();
    Quantity q = order->GetRemainingQuantity();

    if(side == Side::BID)
    {
        //  Find the volume and reduce it for this order.  If volume is zero remove entry.
        auto bid_volume_iter = bid_volume_map.find(price);
        if(bid_volume_iter != bid_volume_map.end()) {
            Volume& vol = bid_volume_iter->second;
            vol -= q;
            if(vol == 0) {
                //  remove the entry.
                bid_volume_map.erase(bid_volume_iter);
            }
        }

        //  Remove the order from the queue, and if the queue is empty at this price,
        //  remove from the heap.
        auto bid_queue_iter = bid_queue_map.find(price);
        if(bid_queue_iter != bid_queue_map.end()) {
            OrderQueue& order_queue = bid_queue_iter->second;
            order_queue.erase(order);
            if(order_queue.empty()) {
                bid_queue_map.erase(bid_queue_iter);
            }
        }

    }
    else if(side == Side::ASK)
    {
        //  Find the volume and reduce it for this order.  If volume is zero remove entry.
        auto ask_volume_iter = ask_volume_map.find(price);
        if(ask_volume_iter != ask_volume_map.end()) {
            Volume& vol = ask_volume_iter->second;
            vol -= q;
            if(vol == 0) {
                //  remove the entry.
                ask_volume_map.erase(ask_volume_iter);
            }
        }

        //  Remove the order from the queue, and if the queue is empty at this price,
        //  remove from the heap.
        auto ask_queue_iter = ask_queue_map.find(price);
        if(ask_queue_iter != ask_queue_map.end()) {
            OrderQueue& order_queue = ask_queue_iter->second;
            order_queue.erase(order);
            if(order_queue.empty()) {
                ask_queue_map.erase(ask_queue_iter);
            }
        }
    }

    order_map.erase(order_id);
    return true;
}

bool OrderBook::UpdateOrder(ModifyOrder& order)
{
    if(!order_map.count(order.GetOrderID()))
    {
        return false;
    }
    const auto& entry = order_map.at(order.GetOrderID());
    CancelOrder(order.GetOrderID());
    AddOrder(ToOrderPtr(entry->GetOrderType(), order));
    return true;
}

Volume OrderBook::GetVolumeAtPrice(Price price, Side side)
{
    Volume volume = 0;
    if(side == Side::BID)
    {
        const auto iter = bid_volume_map.find(price);
        if(iter == bid_volume_map.end()) {
            return 0;
        } else {
            return iter->second;
        }
    }
    else if(side == Side::ASK)
    {
        const auto iter = ask_volume_map.find(price);
        if(iter == ask_volume_map.end()) {
            return 0;
        } else {
            return iter->second;
        }
    }
    return volume;
}

void OrderBook::MatchOrders()
{
    while(!ask_queue_map.empty() && !bid_queue_map.empty()) {
        auto bid_queue_iter = bid_queue_map.begin();
        auto best_bid = bid_queue_iter->first;

        auto ask_queue_iter = ask_queue_map.begin();
        auto best_ask = ask_queue_iter->first;
        
        if(best_bid < best_ask)
        {
            break;
        }
        auto& ask_queue = ask_queue_iter->second;
        auto& bid_queue = bid_queue_iter->second;

        while(!ask_queue.empty() && !bid_queue.empty()) {
            const auto& ask = ask_queue.front();
            auto ask_quantity = ask->GetRemainingQuantity();

            const auto& bid = bid_queue.front();
            auto bid_quantity = bid->GetRemainingQuantity();
            
            Quantity q = std::min(ask_quantity, bid_quantity);

            if(q == 0) {
                //return;
                throw std::runtime_error("Trade quantity cannot be zero");
            }
            //  Push the trade then update the containers
            trades.push_back(Trade{
                TradeSide{bid->GetOrderID(), bid->GetPrice(), q}, 
                TradeSide{ask->GetOrderID(), ask->GetPrice(), q}
            });

            ask->Fill(q);
            ask_volume_map[best_ask] -= q;
            if(ask->Filled()) {
                ask_queue.pop_front();
                order_map.erase(ask->GetOrderID());

                if(ask_volume_map[best_ask] == 0) {
                    //  Erase the price line.
                    ask_volume_map.erase(best_ask);
                }
            }

            bid->Fill(q);
            bid_volume_map[best_bid] -= q;
            if(bid->Filled()) {
                bid_queue.pop_front();
                order_map.erase(bid->GetOrderID());

                if(bid_volume_map[best_bid] == 0) {
                    //  Erase the price line.
                    bid_volume_map.erase(best_bid);
                }
            }
        }

        if(bid_queue.empty()) {
            bid_queue_map.erase(best_bid);
        }
        if(ask_queue.empty()) {
            ask_queue_map.erase(best_ask);
        }
    }
    return;
}

void OrderBook::MatchIOCOrder(OrderPtr order) {
  if(!order || order->GetOrderType() != OrderType::IOC) {
      return;
  }
  Price order_price = order->GetPrice();
  while(!order->Filled()) {
      Side side = order->GetSide();
      if(side == Side::BID) {
        if(ask_queue_map.empty()) {
          break;
        }
        auto ask_iter = ask_queue_map.begin();
        auto best_ask = ask_iter->first;
        auto& ask_queue = ask_iter->second;
        if(order_price < best_ask) {
          break;
        }
        while(!ask_queue.empty() && !order->Filled()) {
          auto ask = ask_queue.front();
          auto ask_quantity = ask->GetRemainingQuantity();
          Quantity quantity = std::min(ask_quantity, order->GetRemainingQuantity());
          ask->Fill(quantity);
          order->Fill(quantity);
          trades.push_back(Trade{
              TradeSide{order->GetOrderID(), order_price, quantity}, 
              TradeSide{ask->GetOrderID(), ask->GetPrice(), quantity}
          });

          if(ask->Filled()) {
            ask_queue.pop_front();
            order_map.erase(ask->GetOrderID());
            if(ask_queue.empty()) {
              ask_queue_map.erase(best_ask);
            }
          }
        }
        if(ask_queue.empty()) {
            ask_queue_map.erase(best_ask);
        }

      } else if(side == Side::ASK) {
        if(bid_queue_map.empty()) {
          break;
        }
        auto bid_iter = bid_queue_map.begin();
        auto best_bid = bid_iter->first;
        auto& bid_queue = bid_iter->second;
        if(order_price > best_bid) {
          break;
        }
        while(!bid_queue.empty() && !order->Filled()) {
          auto bid = bid_queue.front();
          auto bid_quantity = bid->GetRemainingQuantity();
          Quantity quantity = std::min(bid_quantity, order->GetRemainingQuantity());
          bid->Fill(quantity);
          order->Fill(quantity);
          trades.push_back(Trade{
              TradeSide{bid->GetOrderID(), bid->GetPrice(), quantity}, 
              TradeSide{order->GetOrderID(), order_price, quantity}
          });

          if(bid->Filled()) {
            bid_queue.pop_front();

            order_map.erase(bid->GetOrderID());
            if(bid_queue.empty()) {
              bid_queue_map.erase(best_bid);
            }
          }        
        }
        if(bid_queue.empty()) {
            bid_queue_map.erase(best_bid);
        }
    }

  }
}

bool OrderBook::CanMatch(Side s, Price p)
{
    if(s == Side::BID)
    {
        if(ask_queue_map.empty()) {
            return false;
        }

        auto best_ask = ask_queue_map.begin()->first;
        return (p >= best_ask);
    }
    else if(s == Side::ASK)
    {
        if(bid_queue_map.empty()) {
            return false;
        }
        
        auto best_bid = bid_queue_map.begin()->first;
        return (p  <= best_bid);
    }
    return false;
}

void OrderBook::Print()
{
    std::cout << *this << std::endl;
}

std::ostream &operator<<(std::ostream &os, OrderBook &book)
{
    if(book.empty()) {
        return os;
    }

    // TODO: insert return statement here
    int pad_left = 5;
    int vol_width = 6;
    int price_width = 6;
    int pad_empty = pad_left + vol_width + price_width + 2;
    std::string pad(pad_left, ' ');
    std::string empty_quote(pad_empty, ' ');
    int bid_book_size = book.bid_queue_map.size();
    int ask_book_size = book.ask_queue_map.size();
    int book_size = std::max(bid_book_size, ask_book_size);

    std::cout << std::setw(SYMBOL_MAX_LEN) << std::setfill(' ') << formatText(book.identity.symbol) << std::endl;
    if(book_size > 0) {
        std::stringstream ss;
        ss << "           BID     " << "|" << "      ASK";
        os << ss.str() << std::endl;
        ss.str("");

        ss << pad << std::setw(vol_width) << std::setfill(' ') << "VOL" << "  " << std::setw(price_width) << std::setfill(' ') << "PRICE" << "|";
        ss << std::setw(price_width) << std::setfill(' ') << "PRICE" << "  " << std::setw(vol_width) << std::setfill(' ') << "VOL";
        os << ss.str() << std::endl;
        auto bid_iter = begin(book.bid_queue_map);   
        auto ask_iter = begin(book.ask_queue_map);
        for(int i = 0; i < book_size; ++i) {
            ss.str("");
            if(i < bid_book_size) {
                auto bid_price = bid_iter->first;
                PrintBid(ss, book.bid_volume_map[bid_price], bid_price, 3);
                ++bid_iter;
            } else {
                ss << empty_quote;
            }
            ss << "|";
            if(i < ask_book_size) {
                auto ask_price = ask_iter->first;
                PrintAsk(ss, book.ask_volume_map[ask_price], ask_price, 3);
                ++ask_iter;
            } else {
                ss << empty_quote;
            }
            os << ss.str() << std::endl;
        }
    } else {
        os << std::setw(SYMBOL_MAX_LEN) << std::setfill(' ') << "Empty book" << std::endl;
    }

    bool print_trades = false;
    if(print_trades) {
        if(book.trades.size())
        {
            std::string sep(50, '-');
            os << sep << std::endl;
            os << "TRADES: " << book.trades.size()  << std::endl;
            for(const auto& trade: book.trades)
            {
                printTrade(trade);
            }
        }
    }
    return os;
}
}
