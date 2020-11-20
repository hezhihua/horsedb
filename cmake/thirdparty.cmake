
add_custom_target(thirdparty)
include(ExternalProject)
 
 #iguana库
set(IGUANA_ROOT          ${CMAKE_BINARY_DIR}/thirdparty/iguana)
set(IGUANA_LIB_DIR       ${IGUANA_ROOT}/lib)
set(IGUANA_INCLUDE_DIR   ${IGUANA_ROOT}/include)
 
#set(IGUANA_URL           https://github.com/qicosmos/iguana.git)
set(IGUANA_URL           https://github.com.cnpmjs.org/qicosmos/iguana.git  )
set(IGUANA_CONFIGURE     cp -r ${IGUANA_ROOT}/src/iguana  ${CMAKE_SOURCE_DIR}/third-party/)
set(IGUANA_MAKE          cd ${IGUANA_ROOT}/src/iguana )
set(IGUANA_INSTALL       cd ${IGUANA_ROOT}/src/iguana )

ExternalProject_Add(iguana
        GIT_REPOSITORY        ${IGUANA_URL}
        PREFIX                ${IGUANA_ROOT}
        CONFIGURE_COMMAND     ${IGUANA_CONFIGURE}
        BUILD_COMMAND         ${IGUANA_MAKE}
        INSTALL_COMMAND       ${IGUANA_INSTALL}
        BUILD_ALWAYS          0
)

add_dependencies(thirdparty iguana)

 #sql-parser库
 set(PARSER_ROOT          ${CMAKE_BINARY_DIR}/thirdparty/sql-parser)
ExternalProject_Add(sql-parser
        GIT_REPOSITORY        https://github.com.cnpmjs.org/hezhihua/sql-parser.git
        GIT_TAG               main
        PREFIX                ${PARSER_ROOT}
        CONFIGURE_COMMAND     cd ${CMAKE_SOURCE_DIR} 
        BUILD_COMMAND         cd ${PARSER_ROOT}/src/sql-parser && make
        INSTALL_COMMAND       cd ${PARSER_ROOT}/src/sql-parser &&  cp -rf ${PARSER_ROOT}/src/sql-parser ${CMAKE_SOURCE_DIR}/third-party/  && cd ${CMAKE_SOURCE_DIR}/third-party/sql-parser && mkdir -p lib && mv libsqlparser.a lib
        BUILD_ALWAYS          1
)
add_dependencies(thirdparty sql-parser)


#snappy库
set(SNAPPY_ROOT          ${CMAKE_BINARY_DIR}/thirdparty/snappy)
ExternalProject_Add(snappy
        GIT_REPOSITORY        https://github.com.cnpmjs.org/google/snappy.git
        GIT_TAG               1.1.7
        PREFIX                ${SNAPPY_ROOT}
        CONFIGURE_COMMAND     cd ${SNAPPY_ROOT}/src/snappy && cmake -D CMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/third-party/snappy .
        BUILD_COMMAND         cd ${SNAPPY_ROOT}/src/snappy  && make
        INSTALL_COMMAND       cd ${SNAPPY_ROOT}/src/snappy  && make install
        BUILD_ALWAYS          0
)
add_dependencies(snappy sql-parser)
add_dependencies(thirdparty snappy)

#horse-raft库
set(RAFT_ROOT          ${CMAKE_BINARY_DIR}/thirdparty/horse-raft)
ExternalProject_Add(horse-raft
        GIT_REPOSITORY        https://github.com/hezhihua/horse-raft.git
        GIT_TAG               main
        PREFIX                ${RAFT_ROOT}
        CONFIGURE_COMMAND     cd ${RAFT_ROOT}/src/horse-raft && cmake -D CMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/third-party/horse-raft .
        BUILD_COMMAND         cd ${RAFT_ROOT}/src/horse-raft  && make
        INSTALL_COMMAND       cd ${RAFT_ROOT}/src/horse-raft  && make install
        BUILD_ALWAYS          0
)
add_dependencies(thirdparty horse-raft)