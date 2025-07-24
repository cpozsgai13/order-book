cmake_minimum_required(VERSION 3.29)

set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(OrderBook)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(BOOST_ROOT "/usr/local")
set(BOOST_INCLUDEDIR "/usr/local/include")
set(BOOST_LIBRARYDIR "/usr/local/lib")

find_package(Boost 1.88.0 REQUIRED COMPONENTS filesystem system)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/messages ${CMAKE_SOURCE_DIR}/utils ${CMAKE_SOURCE_DIR}/consumer ${CMAKE_SOURCE_DIR}/producer)
file(GLOB SRCS ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/utils/*.cpp ${CMAKE_SOURCE_DIR}/consumer/*.cpp ${CMAKE_SOURCE_DIR}/producer/*.cpp)

add_executable(OrderBook ${SRCS})
set_target_properties(OrderBook PROPERTIES DEBUG_POSTFIX "Debug")
set_target_properties(OrderBook PROPERTIES RELEASE_POSTFIX "Rel")


#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=undefined,address")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=thread")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fsanitize=address")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -fno-stack-protector")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -DNDEBUG")

target_link_libraries(OrderBook rt pthread)
