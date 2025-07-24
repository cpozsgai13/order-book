#include "ExchangeDataProcessorThread.h"
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

auto createFileName=[]() -> std::string {
	std::stringstream ss;

	auto ts = std::chrono::system_clock::now().time_since_epoch().count();
	ss << "perf_" << formatTimestampLocal(ts) << ".csv";

	return ss.str();
};

ExchangeDataProcessorThread::ExchangeDataProcessorThread(ExchangeOrderBook& exch_order_book, 
	RingBufferSPSC<MarketData::Packet, RING_BUFFER_SIZE>& msg_queue, 
	const PerformanceMeta& perf_meta): 
	exchange_order_book(exch_order_book),
	packet_queue(msg_queue),
	perf_meta(perf_meta)
{
	initHandlers();

}

void ExchangeDataProcessorThread::initHandlers() {
	handler_map[DataType::SYMBOL] = [this](CoreMessage& cm) { this->processSymbol(cm);};
	handler_map[DataType::ADD_ORDER] = [this](CoreMessage& cm) { this->processAddOrder(cm);};
	handler_map[DataType::UPDATE_ORDER] = [this](CoreMessage& cm) { this->processUpdateOrder(cm);};
	handler_map[DataType::CANCEL_ORDER] = [this](CoreMessage& cm) { this->processCancelOrder(cm);};
}

void ExchangeDataProcessorThread::processSymbol(CoreMessage& cm) {
	Symbol& symbol = (Symbol&)cm.data.symbol;
	exchange_order_book.AddUpdateSymbol(symbol);
}

void ExchangeDataProcessorThread::processAddOrder(CoreMessage& cm) {
	AddOrder& order = (AddOrder&)cm.data.add_order;
	if(!perf_counter) {
		exchange_order_book.AddNewOrder(order.instrument_id, order);
	} else {
		perf_counter->addStat(CallAndMeasure(&ExchangeOrderBook::AddNewOrder, &exchange_order_book, order.instrument_id, order));
	}
}

void ExchangeDataProcessorThread::processUpdateOrder(CoreMessage& cm) {
	ModifyOrder& order = (ModifyOrder&)cm.data.update_order;
	if(!perf_counter) {
		exchange_order_book.UpdateOrder(order.instrument_id, order);
	} else {
		perf_counter->updateStat(CallAndMeasure(&ExchangeOrderBook::UpdateOrder, &exchange_order_book, order.instrument_id, order));
	}
}

void ExchangeDataProcessorThread::processCancelOrder(CoreMessage& cm) {
	CancelOrder& cancel = (CancelOrder&)cm.data.cancel_order;
	if(!perf_counter) {
		exchange_order_book.CancelOrder(cancel.instrument_id, cancel.order_id);
	} else {
		perf_counter->cancelStat(CallAndMeasure(&ExchangeOrderBook::CancelOrder, &exchange_order_book, cancel.instrument_id, cancel.order_id));
	}
}

bool ExchangeDataProcessorThread::start() {
	if(perf_meta.enabled) {
		perf_counter = std::make_unique<PerformanceCounter>(perf_meta.output_file, perf_meta.processor_speed);
	}  

  running.store(true);
  return run();
}

void ExchangeDataProcessorThread::stop() {
  running.store(false);

//   exchange_order_book.PrintAll();
}

bool ExchangeDataProcessorThread::run() {
    //  Wait until started
	while(running.load()) {
		if(!running) {
			break;
		}
		Packet packet;
		if(packet_queue.pop(packet))
		{
			if(packet.GetSize() <= UDP_BUFFER_SIZE) {
				processPacket(packet);
			} else {
				std::cout << "ExchangeDataProcessorThread received invalid packet of size " << (int)packet.GetSize() << std::endl;
			}
		}
	}
	if(perf_counter) {
		perf_counter->writeToFile(createFileName());
		perf_counter->printStats();
	}
	return true;
}

void ExchangeDataProcessorThread::processPacket(Packet& packet) {
	//	Parse the packet into individual messages.
	auto num_messages = packet.data.packet.header.num_messages;
	//std::cout << "Processor got packet with " << num_messages << std::endl;
	for(size_t i = 0; i < num_messages; ++i) {
		DataType dt = packet.data.packet.messages[i].getDataType();
		auto iter = handler_map.find(dt);
		if(iter != handler_map.end()) {
			iter->second(packet.data.packet.messages[i]);
		}
	}
}

}