
SRC := MulticastProducer.cpp MulticastSender.cpp TCPSender.cpp
OBJ := $(SRC:.cpp=.o)
CC := g++
CXXFLAGS := --std=c++17 -Wextra -I../ -I../messages -I../utils

DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

release: CXXFLAGS += $(RELEASE_FLAGS)
release: all

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) 

