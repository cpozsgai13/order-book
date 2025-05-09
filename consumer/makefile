
SRC := MulticastConsumer.cpp MulticastReceiver.cpp TCPReceiver.cpp
OBJ := $(SRC:.cpp=.o)
CC := g++
CXXFLAGS := --std=c++17 -Wextra -I../messages -I../ -I../utils

DEBUG_FLAGS = -g -O0 -DDEBUG
# DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
#RELEASE_FLAGS = -O3 -DNDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
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

