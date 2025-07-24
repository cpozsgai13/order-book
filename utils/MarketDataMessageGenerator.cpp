#include "MarketDataMessageGenerator.h"
#include <algorithm>
#include <chrono>

namespace MarketData {

//  Minimum number of orders to be added before randomly generating updates or cancels
static constexpr int MIN_ORDER_SIZE = 5;

struct SymbolGreater {
  bool operator()(const Symbol& s1, const Symbol& s2) {
    return s1.instrument_id > s2.instrument_id;
  }
};

struct SymbolLess {
  bool operator()(const Symbol& s1, const Symbol& s2) {
    return s1.instrument_id < s2.instrument_id;
  }
};

template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
T randomRange(T tmin, T tmax) {
  static std::random_device dev;
  static std::mt19937 e2(dev());

  std::uniform_int_distribution<T> range_generator{tmin, tmax};
  //std::uniform_int_distribution<> range_generator{tmin, tmax};
  return range_generator(e2);
}

template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
T randomRangeStep(T tmin, T tmax, T step) {
  return randomRange(std::max((T)1, tmin / step), tmax / step)*step;
}

void MarketDataMessageGenerator::init() {
    // generator_map.insert(std::make_pair(DataType::ADD_ORDER, std::bind(&MarketDataMessageGenerator::GenerateAddOrder, this, std::placeholders::_1, std::placeholders::_2)));
    // generator_map.insert(std::make_pair(DataType::UPDATE_ORDER, std::bind(&MarketDataMessageGenerator::GenerateUpdateOrder, this, std::placeholders::_1,std::placeholders::_2)));
    // generator_map.insert(std::make_pair(DataType::CANCEL_ORDER, std::bind(&MarketDataMessageGenerator::GenerateCancelOrder, this, std::placeholders::_1, std::placeholders::_2)));
	generator_map[DataType::ADD_ORDER] = [this](CoreMessage& cm, InstrumentID id) { return this->GenerateAddOrder(cm, id);};
	generator_map[DataType::UPDATE_ORDER] = [this](CoreMessage& cm, InstrumentID id) { return this->GenerateUpdateOrder(cm, id);};
	generator_map[DataType::CANCEL_ORDER] = [this](CoreMessage& cm, InstrumentID id) { return this->GenerateCancelOrder(cm, id);};
}

bool MarketDataMessageGenerator::GenerateMessages(std::vector<Symbol>& symbol_set, uint32_t N,
  std::vector<Packet>& message_packets) {
  if(symbol_set.empty()) {
    return false;
  }

  std::mt19937 e2(dev());    
  order_ids.reserve(N/2);
  if(generator_map.empty()) {
    init();
  }

  Packet cur_packet;
  int type_min = (int)DataType::ADD_ORDER;
  int type_max = (int)DataType::CANCEL_ORDER;
  std::uniform_int_distribution<> type_generator{type_min, type_max};

  size_t instrument_min = 0;
  size_t instrument_max = symbol_set.size() - 1;

  std::uniform_int_distribution<size_t> sym_index_generator{instrument_min, instrument_max};

  for(uint32_t i = 0; i < N; ++i) {
    CoreMessage msg;

    //  Select the symbol via random index
    int index = sym_index_generator(e2);
    DataType new_type = order_ids.size() < MIN_ORDER_SIZE ? DataType::ADD_ORDER : (DataType)type_generator(e2);

    auto iter = generator_map.find(new_type);
    if(iter == generator_map.end()) {
      return false;
    }
    iter->second(msg, symbol_set[index].instrument_id);

    if(cur_packet.CanAddMessage()) {
      cur_packet.AddMessage(msg);
    } else {
      message_packets.push_back(cur_packet);
      cur_packet.clear();
      cur_packet.AddMessage(msg);
    }
  }

  if(cur_packet.MessageCount() > 0) {
    message_packets.push_back(cur_packet);
  }
  return true;
}

bool MarketDataMessageGenerator::GenerateAddOrder(CoreMessage& msg, InstrumentID id) {
  msg.data_type = DataType::ADD_ORDER;
  OrderID order_id = randomRange<uint64_t>(1, 1 << 8);
  Side s = (Side)randomRange<uint8_t>(0, 1);
  OrderType ot = (OrderType)randomRange<uint8_t>(0, 1);

  RawPrice max_price = 1'000'000'000;
  RawPrice p = randomRangeStep<uint64_t>(1UL, max_price, 5'000UL);

  Quantity max_quantity = 1'000;
  Quantity q = randomRangeStep<uint64_t>(1UL, max_quantity, 5UL);

  uint64_t creation_time_ns = std::chrono::system_clock::now().time_since_epoch().count();

  msg.data.add_order = AddOrder{ot, id, order_id, s, Price(p), q, creation_time_ns};
  order_ids.push_back(order_id);
  return true;
}

bool MarketDataMessageGenerator::GenerateUpdateOrder(CoreMessage& msg, InstrumentID id) {
  msg.data_type = DataType::UPDATE_ORDER;

  size_t order_index = randomRange<uint64_t>(0, order_ids.size() - 1);
  OrderID order_id = order_ids[order_index];
  Side s = (Side)randomRange<uint8_t>(0, 1);

  RawPrice max_price = 1'000'000'000;
  RawPrice p = randomRangeStep<uint64_t>(1UL, max_price, 5'000UL);

  Quantity max_quantity = 1'000;
  Quantity q = randomRangeStep<uint64_t>(1UL, max_quantity, 5UL);
  
  uint64_t update_time_ns = std::chrono::system_clock::now().time_since_epoch().count();
  msg.data.update_order = ModifyOrder{order_id, id, s, Price(p), q, update_time_ns};
  return true;
}

bool MarketDataMessageGenerator::GenerateCancelOrder(CoreMessage& msg, InstrumentID id) {
  msg.data_type = DataType::CANCEL_ORDER;
  size_t order_index = randomRange<uint64_t>(0, order_ids.size() - 1);
  OrderID order_id = order_ids[order_index];
  msg.data.cancel_order = CancelOrder{id, order_id};
  return true;
}

}