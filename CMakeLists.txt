cmake_minimum_required(VERSION 2.8)
#add_compile_options(-g -std=c++1z -Wno-deprecated -Wall -pthread -Wl,--no-as-needed)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++1z -Wno-deprecated -Wall -pthread -Wno-switch -Wno-char-subscripts -Wno-reorder -Wno-unused-function -Wno-unused-local-typedefs" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-builtin-macro-redefined -D__FILE__='\"$(notdir $(abspath $<))\"'")
message(STATUS "optional:-g -std=c++1z -Wno-deprecated -Wall -pthread") 

set(TARS_VERSION "2.4.8")
add_definitions(-DTARS_VERSION="${TARS_VERSION}")

set_target_properties(${TARGET} PROPERTIES
COMPILE_FLAGS "-pthread"
LINK_FLAGS "-pthread")

include(cmake/thirdparty.cmake)

include_directories(./src)
include_directories(./src/include)
include_directories(./third-party/horse-raft/third-party/horse-rpc/include)
include_directories(./third-party/horse-raft/include)

include_directories(./third-party/horse-raft/third-party/spdlog/include)
include_directories(./third-party/horse-raft/third-party/yaml-cpp/include)
include_directories(./third-party/horse-raft/third-party/rocksdb/include)

include_directories(./third-party/iguana)
include_directories(./third-party/sql-parser/src)


FILE(GLOB_RECURSE SRC_LIST 

src/server/mysql_handle.cpp 
src/server/horsedb_server.cpp 

src/kv/meta.cpp
src/kv/table.cpp
src/mysql/command.cpp
src/mysql/mysql_packet.cpp
src/mysql/socket_context.cpp

)


LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/horse-raft/third-party/rocksdb/lib)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/horse-raft/third-party/yaml-cpp/lib)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/horse-raft/lib)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/horse-raft/third-party/horse-rpc/lib)


LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/snappy/lib64)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/third-party/sql-parser/lib)

link_libraries(horse-raft)
link_libraries(horse-rpc)
link_libraries(rocksdb)
link_libraries(yaml-cpp)

link_libraries(snappy)
link_libraries(dl)
link_libraries(z)
link_libraries(bz2)
link_libraries(sqlparser)

add_executable(horsedb-server ${SRC_LIST})
