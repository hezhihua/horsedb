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


include_directories(./src)
include_directories(./src/include)
include_directories(./horse-rpc/src/include)
include_directories(./horse-raft/src/include)

include_directories(./horse-rpc/third-party/spdlog-1.x/include)
include_directories(./horse-raft/third-party/yaml-cpp-yaml-cpp-0.6.3/include)
include_directories(./horse-raft/third-party/rocksdb-6.11.4/include)

include_directories(./src/thrid/iguana)
include_directories(./src/thrid/sql-parser/src)


FILE(GLOB_RECURSE SRC_LIST 

src/server/mysql_handle.cpp 
src/server/horsedb_server.cpp 

src/kv/meta.cpp
src/kv/table.cpp
src/mysql/command.cpp
src/mysql/mysql_packet.cpp
src/mysql/socket_context.cpp

)

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/horse-rpc)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/horse-raft)

LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/horse-raft/lib)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/horse-rpc/lib)
LINK_DIRECTORIES(/root/svn/horsedb/src/lib)

link_libraries(horse-raft)
link_libraries(horse-rpc)
link_libraries(sqlparser)
link_libraries(rocksdb)
link_libraries(snappy)
link_libraries(yaml-cpp)

add_executable(horsedb-server ${SRC_LIST})
