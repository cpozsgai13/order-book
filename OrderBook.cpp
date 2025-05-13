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

auto formatTimestampLocal = [](uint64_t timestamp_ns) -> std::string {
    time_t t = timestamp_ns/1'000'000'000;
    auto micros = (timestamp_ns/1'000) % 1'000'000;
    struct tm *ts = localtime(&t);

    std::stringstream ss;
    ss << std::put_time(ts, "%H:%M:%S.") << std::setw(3) << std::setfill('0') << micros;
    return ss.str();
};

auto BetterPrice =[](const Order& o1, const Order& o2) -> bool
{
    if(o1.GetSide() == Side::BID)
    {
        return o1.GetPrice() >= o2.GetPrice();
    }
    else if(o1.GetSide() == Side::ASK)
    {
        return o1.GetPrice() < o2.GetPrice();
    }
};

auto FilterBidQueue=[](
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareBid>& bid_map, 
    std::unordered_set<OrderID>& skip_list) ->
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareBid>
{
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareBid> bid_map_out{OrderCompareBid()}; 
    while(!bid_map.empty()) {
        auto& bid = bid_map.top();

        if(skip_list.count(bid->GetOrderID()) == 0) {
            //  Add to the list.
            bid_map_out.push(bid);
        }
        bid_map.pop();
    }
    return bid_map_out;
};

auto FilterAskQueue=[](
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareAsk>& ask_map, 
    std::unordered_set<OrderID>& skip_list) ->
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareAsk>
{
    std::priority_queue<OrderPtr, std::vector<OrderPtr>, OrderCompareAsk> ask_map_out{OrderCompareAsk()}; 
    while(!ask_map.empty()) {
        auto& ask = ask_map.top();

        if(skip_list.count(ask->GetOrderID()) == 0) {
            //  Add to the list.
            ask_map_out.push(ask);
        }
        ask_map.pop();
    }
    return ask_map_out;
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
    //std::cout << "OrderBook " << this << " deleted" << std::endl;
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
        best_bid_map.push(order);
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

        best_ask_map.push(order);
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
            auto iter = std::find_if(begin(order_queue), end(order_queue), [&order_id](const OrderPtr& ptr) { return ptr->GetOrderID() == order_id;});
            if(iter != order_queue.end()) {
                order_queue.erase(iter);
            }
            if(order_queue.empty()) {
                bid_queue_map.erase(bid_queue_iter);

                //  To remove from pri queue, add to skip list. 
                best_bid_skip_list.insert(order_id);
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
            auto iter = std::find_if(begin(order_queue), end(order_queue), [&order_id](const OrderPtr& ptr) { return ptr->GetOrderID() == order_id;});
            if(iter != order_queue.end()) {
                order_queue.erase(iter);
            }
            if(order_queue.empty()) {
                ask_queue_map.erase(ask_queue_iter);

                //  To remove from pri queue, add to skip list. 
                best_ask_skip_list.insert(order_id);
            }
        }
    }
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
    while(true)
    {
        if(!best_bid_skip_list.empty()) {
            best_bid_map = FilterBidQueue(best_bid_map, best_bid_skip_list);
            best_bid_skip_list.clear();
        }

        if(!best_ask_skip_list.empty()) {
            best_ask_map = FilterAskQueue(best_ask_map, best_ask_skip_list);
            best_ask_skip_list.clear();
        }

        if(best_bid_map.empty() || best_ask_map.empty())
        {
            break;
        }
        OrderPtr best_ask_order = best_ask_map.top();
        auto best_ask = best_ask_order->GetPrice();

        OrderPtr best_bid_order = best_bid_map.top();
        auto best_bid = best_bid_order->GetPrice();

        if(best_bid < best_ask)
        {
            break;
        }

        //  Find the order queue at this price.
        auto ask_queue_iter = ask_queue_map.find(best_ask);
        auto ask_queue = ask_queue_iter->second;

        auto bid_queue_iter = bid_queue_map.find(best_bid);
        auto bid_queue = bid_queue_iter->second;

        while(!ask_queue.empty() && !bid_queue.empty()) {
        // while(ask_queue.size() > 0 && bid_queue.size() > 0) {
            auto& ask = ask_queue.front();
            auto ask_quantity = ask->GetRemainingQuantity();

            auto& bid = bid_queue.front();
            auto bid_quantity = bid->GetRemainingQuantity();
            
            Quantity q = std::min(ask_quantity, bid_quantity);

            if(q == 0) {
                //  There is a problem
                throw std::runtime_error("Trade quantity cannot be zero");
            }
            //  Dump book on each trade.
#ifdef DEBUG
            Print();
            trades.clear();
            printTrade(Trade{
                TradeSide{bid->GetOrderID(), bid->GetPrice(), q}, 
                TradeSide{ask->GetOrderID(), ask->GetPrice(), q}});
#else
            //  Push the trade then update the containers
            trades.push_back(Trade{
                TradeSide{bid->GetOrderID(), bid->GetPrice(), q}, 
                TradeSide{ask->GetOrderID(), ask->GetPrice(), q}
            });
#endif

            ask->Fill(q);
            ask_volume_map[best_ask] -= q;
            if(ask->Filled()) {
                ask_queue.erase(ask_queue.begin());
                order_map.erase(ask->GetOrderID());

                if(ask_volume_map[best_ask] == 0) {
                    //  Erase the price line.
                    best_ask_map.pop();
                    ask_queue_map.erase(best_ask);
                    ask_volume_map.erase(best_ask);
                }
            }

            bid->Fill(q);
            bid_volume_map[best_bid] -= q;
            if(bid->Filled()) {
                bid_queue.erase(bid_queue.begin());
                order_map.erase(bid->GetOrderID());

                if(bid_volume_map[best_bid] == 0) {
                    //  Erase the price line.
                    best_bid_map.pop();
                    bid_queue_map.erase(best_bid);
                    bid_volume_map.erase(best_bid);
                }
            }
        }
    }
    return;
}

bool OrderBook::CanMatch(Side s, Price p)
{
    if(s == Side::BID)
    {
        if(best_ask_map.empty())
        {
            return false;
        }

        auto best_ask = best_ask_map.top()->GetPrice();
        return (p >= best_ask);
    }
    else if(s == Side::ASK)
    {
        if(best_bid_map.empty())
        {
            return false;
        }

        auto best_bid = best_bid_map.top()->GetPrice();
        return (p  <= best_bid);
    }
    return false;
}

//  Print the order book in this format:
//  ------------------------------------------
//      BID_VOL BID_PRICE | ASK_PRICE ASK_VOL     
//  If a side is empty then fill with spaces
void OrderBook::Print()
{
    int pad_left = 5;
    int vol_width = 6;
    int price_width = 6;
    int pad_empty = pad_left + vol_width + price_width + 1;
    std::string pad(pad_left, ' ');
    std::string empty_quote(pad_empty, ' ');
    int bid_book_size = bid_queue_map.size();
    int ask_book_size = ask_queue_map.size();
    int book_size = std::max(bid_book_size, ask_book_size);

    std::cout << std::setw(SYMBOL_MAX_LEN) << std::setfill(' ') << formatText(identity.symbol) << std::endl;
    if(book_size > 0) {
        std::stringstream ss;
        ss << "           BID    " << "|" << "      ASK";
        std::cout << ss.str() << std::endl;
        ss.str("");

        ss << pad << std::setw(vol_width) << std::setfill(' ') << "VOL" << " " << std::setw(price_width) << std::setfill(' ') << "PRICE" << "|";
        ss << std::setw(price_width) << std::setfill(' ') << "PRICE" << " " << std::setw(vol_width) << std::setfill(' ') << "VOL";
        std::cout << ss.str() << std::endl;
        auto bid_iter = begin(bid_queue_map);   
        auto ask_iter = begin(ask_queue_map);
        for(int i = 0; i < book_size; ++i) {
            ss.str("");
            if(i < bid_book_size) {
                auto bid_price = bid_iter->first;
                ss << pad << std::setw(6) << std::setfill(' ') << std::right << bid_volume_map[bid_price] << " " << std::setw(6) << std::setfill(' ') << std::right << (double) bid_price;
                ++bid_iter;
            } else {
                ss << empty_quote;
            }
            ss << "|";
            if(i < ask_book_size) {
                auto ask_price = ask_iter->first;
                ss << std::setw(6) << std::setfill(' ') << std::right << (double)ask_price << " " << std::setw(6) << std::setfill(' ') << std::right << (double) ask_volume_map[ask_price];
                ++ask_iter;
            } else {
                ss << empty_quote;
            }
            std::cout << ss.str() << std::endl;
        }
    } else {
        std::cout << std::setw(SYMBOL_MAX_LEN) << std::setfill(' ') << "Empty book" << std::endl;
    }

    if(trades.size())
    {
        std::string sep(50, '-');
        std::cout << sep << std::endl;
        std::cout << "TRADES: " << trades.size()  << std::endl;
        for(const auto& trade: trades)
        {
            printTrade(trade);
        }
    }

}
}