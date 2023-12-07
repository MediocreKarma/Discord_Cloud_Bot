cmake_minimum_required(VERSION 3.10)

set(OUT_TARGET main)

project(${OUT_TARGET})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

find_package(SQLite3)

include_directories(${SQLite3_INCLUDE_DIRS})

add_executable(${OUT_TARGET} main.cpp ../Server/SQL_DB.cpp)
target_link_libraries(${OUT_TARGET} dpp ${SQLite3_LIBRARIES})