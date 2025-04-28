
#g++ MatchOrderEngine.cpp OrderBook.cpp MatchOrder.cpp -std=c++17 -lboost_system -lrt -lpthread -g -O0 -o MatchOrderEngine
g++ OrderBook.cpp main.cpp -std=c++17 -lboost_system -lrt -lpthread -g -O0 -o OrderBookAppDebug
