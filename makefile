
SUBDIRS := producer consumer messages utils
PROG := OrderBookApp
SRC := ExchangeOrderBook.cpp ExchangeDataProcessor.cpp OrderBook.cpp main.cpp

OBJS := $(SRC:.cpp=.o)
PRODUCER_OBJS := producer/MulticastProducer.o producer/MulticastSender.o producer/TCPSender.o
CONSUMER_OBJS := consumer/MulticastConsumer.o consumer/MulticastReceiver.o consumer/TCPReceiver.o
UTIL_OBJS := utils/MarketDataFileReader.o utils/PerformanceCounter.o

CC := g++
CXXFLAGS := -Wextra -I. -I./messages -I./utils -I./producer -I./consumer
LIBS := -lboost_system -lrt -lpthread 

DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG -Wall

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: PROG = OrderBookAppDebug
debug: all

release: CXXFLAGS += $(RELEASE_FLAGS)
release: all
#OBJ := $(patsubst %.o,utils/%.o,producer/%.o,consumer/%.o)

all:  root subdirs
	$(CC) $(CXXFLAGS) $(OBJS) $(PRODUCER_OBJS) $(CONSUMER_OBJS) $(UTIL_OBJS) $(LIBS) -o $(PROG)

root: $(OBJS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

subdirs:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir; \
	done

clean:
	rm -f $(OBJS) $(PROG)
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
