#ifndef _MARKET_DATA_MESSAGE_GENERATOR_H_
#define _MARKET_DATA_MESSAGE_GENERATOR_H_

#include "CoreMessages.h"
#include <functional>
#include <unordered_map>
#include <random>
#include <vector>

namespace MarketData 
{

class MarketDataMessageGenerator {
  using GeneratorFn = std::function<bool(CoreMessage&, InstrumentID)>;
public:
  MarketDataMessageGenerator() = default;
  ~MarketDataMessageGenerator() = default;
  bool GenerateMessages(std::vector<Symbol>& symbol, uint32_t N,
    std::vector<Packet>& message_packets);
private:
  void init();
  std::unordered_map<DataType, GeneratorFn> generator_map;
  std::vector<OrderID> order_ids;
  bool GenerateAddOrder(CoreMessage& msg, InstrumentID id);
  bool GenerateUpdateOrder(CoreMessage& msg, InstrumentID id);
  bool GenerateCancelOrder(CoreMessage& msg, InstrumentID id);

  std::random_device dev;
};

}

#endif