cmake_minimum_required(VERSION 3.2)

project(OrderBook)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/messages ${CMAKE_SOURCE_DIR}/utils ${CMAKE_SOURCE_DIR}/consumer ${CMAKE_SOURCE_DIR}/producer)
file(GLOB SRCS ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/utils/*.cpp ${CMAKE_SOURCE_DIR}/consumer/*.cpp ${CMAKE_SOURCE_DIR}/producer/*.cpp)

add_executable(OrderBook ${SRCS})
set_target_properties(OrderBook PROPERTIES DEBUG_POSTFIX "Debug")
set_target_properties(OrderBook PROPERTIES RELEASE_POSTFIX "Rel")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=undefined,address")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=thread")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=address")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fno-stack-protector")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -DNDEBUG")

target_link_libraries(OrderBook rt pthread)
