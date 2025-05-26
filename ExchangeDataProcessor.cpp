#include "ExchangeDataProcessor.h"
#include "LogUtils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <tuple>
#include <chrono>
#include "rtdsc.h"
#include "PerformanceCounter.h"

namespace MarketData
{

auto formatTimestampLocal = [](uint64_t timestamp_ns) -> std::string {
    time_t t = timestamp_ns/1'000'000'000;
    struct tm *ts = localtime(&t);
    std::stringstream ss;
    ss << std::put_time(ts, "%H%M%S");
    return ss.str();
};

auto createFileName=[]() -> std::string {
	std::stringstream ss;

	auto ts = std::chrono::system_clock::now().time_since_epoch().count();
	ss << "perf_" << formatTimestampLocal(ts) << ".csv";

	return ss.str();
};

ExchangeDataProcessor::ExchangeDataProcessor(ExchangeOrderBook& exch_order_book, 
	RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& msg_queue, std::mutex& mut, std::condition_variable& cnd): 
	exchange_order_book(exch_order_book),
	packet_queue(msg_queue),
	m(mut),
	cond(cnd)
{
	initHandlers();
}

void ExchangeDataProcessor::initHandlers() {
	handler_map.insert(std::make_pair(DataType::SYMBOL, std::bind(&ExchangeDataProcessor::processSymbol, this, std::placeholders::_1)));
	handler_map.insert(std::make_pair(DataType::ADD_ORDER, std::bind(&ExchangeDataProcessor::processAddOrder, this, std::placeholders::_1)));
	handler_map.insert(std::make_pair(DataType::UPDATE_ORDER, std::bind(&ExchangeDataProcessor::processUpdateOrder, this, std::placeholders::_1)));
	handler_map.insert(std::make_pair(DataType::CANCEL_ORDER, std::bind(&ExchangeDataProcessor::processCancelOrder, this, std::placeholders::_1)));
}

void ExchangeDataProcessor::processSymbol(CoreMessage& cm) {
	Symbol& symbol = (Symbol&)cm.data.symbol;
	exchange_order_book.AddUpdateSymbol(symbol);
}

void ExchangeDataProcessor::processAddOrder(CoreMessage& cm) {
	AddOrder& order = (AddOrder&)cm.data.add_order;
	if(!measuring) {
		exchange_order_book.AddNewOrder(order.instrument_id, order);
	} else {
		perf_counter.addStat(CallAndMeasure(&ExchangeOrderBook::AddNewOrder, &exchange_order_book, order.instrument_id, order));
	}
}

void ExchangeDataProcessor::processUpdateOrder(CoreMessage& cm) {
	ModifyOrder& order = (ModifyOrder&)cm.data.update_order;
	if(!measuring) {
		exchange_order_book.UpdateOrder(order.instrument_id, order);
	} else {
		perf_counter.updateStat(CallAndMeasure(&ExchangeOrderBook::UpdateOrder, &exchange_order_book, order.instrument_id, order));
	}
}

void ExchangeDataProcessor::processCancelOrder(CoreMessage& cm) {
	CancelOrder& cancel = (CancelOrder&)cm.data;
	if(!measuring) {
		exchange_order_book.CancelOrder(cancel.instrument_id, cancel.order_id);
	} else {
		perf_counter.cancelStat(CallAndMeasure(&ExchangeOrderBook::CancelOrder, &exchange_order_book, cancel.instrument_id, cancel.order_id));
	}
}

void ExchangeDataProcessor::start() {
  std::lock_guard<std::mutex> lock(m);
  running.store(true);
  cond.notify_one();
}

void ExchangeDataProcessor::stop() {
  std::lock_guard<std::mutex> lock(m);
  running.store(false);
  cond.notify_one();

  exchange_order_book.PrintAll();
  if(measuring) {
	perf_counter.printStats();
	perf_counter.writeToFile(createFileName());
  }
}

bool ExchangeDataProcessor::run() {
    //  Wait until started
    {
      std::unique_lock<std::mutex> lock(m);
      cond.wait(lock, [this]{return running.load();});
    }

	Packet packet;

	while(running.load()) {
		if(!running) {
			break;
		}
		if(packet_queue.pop(packet))
		{
			if(packet.GetSize() <= UDP_BUFFER_SIZE) {
				processPacket(std::move(packet));
			} else {
				std::cout << "ExchangeDataProcessor received invalid packet of size " << packet.GetSize() << std::endl;
			}
		}
	}
	return true;
}

void ExchangeDataProcessor::processPacket(Packet&& packet) {
	//	Parse the packet into individual messages.
	auto num_messages = packet.header.num_messages;
	for(size_t i = 0; i < num_messages; ++i) {
		CoreMessage& cm = packet.data.packet.messages[i];
		DataType dt = cm.getDataType();
		auto iter = handler_map.find(dt);
		if(iter != handler_map.end()) {
			iter->second(cm);
		}

	}
}

}