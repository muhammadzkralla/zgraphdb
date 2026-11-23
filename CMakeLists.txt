cmake_minimum_required(VERSION 3.16)
project(zgraphdb LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(zgraphdb
    src/model/term.cpp
    src/storage/dictionary.cpp
    src/storage/triple_store.cpp
    src/storage/indexes.cpp
    src/api/db.cpp
)

target_include_directories(zgraphdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(zgraphdb-cli tools/zgraphdb-cli.cpp)
target_link_libraries(zgraphdb-cli PRIVATE zgraphdb)

add_executable(zgraphdb-bench tools/zgraphdb-bench.cpp)
target_link_libraries(zgraphdb-bench PRIVATE zgraphdb)

enable_testing()
add_executable(zgraphdb-tests tests/test_skeleton.cpp)
target_link_libraries(zgraphdb-tests PRIVATE zgraphdb)
target_compile_definitions(zgraphdb-tests PRIVATE ZGRAPHDB_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples")
add_test(NAME skeleton COMMAND zgraphdb-tests)
