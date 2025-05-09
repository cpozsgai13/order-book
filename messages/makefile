
SRC := MessagePacker.cpp
OBJ := $(SRC:.cpp=.o)
CC := g++
CXXFLAGS := --std=c++17 -Wextra -I. -I../ -I../utils

# DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG 
#RELEASE_FLAGS = -O3 -DNDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

release: CXXFLAGS += $(RELEASE_FLAGS)
release: all

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) 

