#include "ExchangeDataProcessor.h"
#include "LogUtils.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <tuple>
#include "rtdsc.h"
#include "PerformanceCounter.h"

namespace MarketData
{

ExchangeDataProcessor::ExchangeDataProcessor(ExchangeOrderBook& exch_order_book, 
	std::queue<Packet>& msg_queue, std::mutex& mut, std::condition_variable& cnd): 
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
	Symbol& symbol = (Symbol&)cm.data;

	InstrumentID inst_id = symbol.instrument_id;
	std::string inst = formatText(symbol.symbol, SYMBOL_MAX_LEN);
	exchange_order_book.AddUpdateSymbol(symbol);
}

void ExchangeDataProcessor::processAddOrder(CoreMessage& cm) {
	AddOrder& order = (AddOrder&)cm.data;
	if(!measuring) {
		exchange_order_book.AddNewOrder(order.instrument_id, order);
	} else {
		perf_counter.addStat(CallAndMeasure(&ExchangeOrderBook::AddNewOrder, &exchange_order_book, order.instrument_id, order));
	}
}

void ExchangeDataProcessor::processUpdateOrder(CoreMessage& cm) {
	ModifyOrder& order = (ModifyOrder&)cm.data;
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
  }
}

bool ExchangeDataProcessor::run() {
    //  Wait until started
    {
      std::unique_lock<std::mutex> lock(m);
      cond.wait(lock, [this]{return running.load();});
    }

	while(running.load()) {
		std::unique_lock<std::mutex> lock(m);

		cond.wait(lock, [this]{return !running || !packet_queue.empty();});

		if(!running) {
			break;
		}

		auto packet = packet_queue.front();
		packet_queue.pop();
		if(packet.GetSize()) {
			std::cout << "ExchangeDataProcessor received packet of size " << packet.GetSize() << std::endl;
			processPacket(std::move(packet));
		} else {
			std::cout << "ExchangeDataProcessor received invalid packet of size " << packet.GetSize() << std::endl;
		}
	}
    std::cout << "Processor left main run loop" << std::endl;
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