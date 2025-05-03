#include "ExchangeOrderBook.h"
#include <chrono>

namespace MarketData 
{
ExchangeOrderBook::ExchangeOrderBook(const std::string& name):
exchange_name(name){}

bool ExchangeOrderBook::AddUpdateSymbol(Symbol& symbol) {
    instrument_map[symbol.instrument_id] = std::make_shared<OrderBook>(symbol);
    std::string sym = formatText(symbol.symbol, SYMBOL_MAX_LEN);
    symbol_map[sym] = symbol.instrument_id;
    return true;
}

bool ExchangeOrderBook::AddNewOrder(InstrumentID inst_id, AddOrder& order) {
  auto iter = instrument_map.find(inst_id);
  if(iter == instrument_map.end()) {
    return false;
  }
  auto now = std::chrono::system_clock::now().time_since_epoch().count();
  return iter->second->AddOrder(std::make_shared<Order>(order.order_type, order.side, order.order_id, order.price.rawValue(),
    order.quantity, now));
}

bool ExchangeOrderBook::UpdateOrder(InstrumentID inst_id, ModifyOrder& order) {
  auto iter = instrument_map.find(inst_id);
  if(iter == instrument_map.end()) {
    return false;
  }
  auto order_id = order.order_id;

  return iter->second->UpdateOrder(order);
}

bool ExchangeOrderBook::CancelOrder(InstrumentID inst_id, OrderID orderID) {
  auto iter = instrument_map.find(inst_id);
  if(iter == instrument_map.end()) {
    return false;
  }
  return iter->second->CancelOrder(orderID);
}

bool ExchangeOrderBook::PrintBook(const std::string& symbol) {
  //  Get inst_id from symbol.
  auto iter = symbol_map.find(symbol);
  if(iter == symbol_map.end()) {
    return false;
  }

  InstrumentID inst_id = iter->second;

  auto book_iter = instrument_map.find(inst_id);
  if(book_iter == instrument_map.end()) {
    return false;
  }
  book_iter->second->Print();
}


}