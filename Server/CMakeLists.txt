cmake_minimum_required (VERSION 3.8)

set (PROJECT_NAME Server_MyDiscordDrive)

project (${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_executable(
  ${PROJECT_NAME} /
    server.cpp /
    requestHandler.cpp / 
    utils.cpp /
    SQL_DB.cpp /
    ClientThreads.cpp /
    sha256.cpp /
    ../Common/commons.cpp /
    ../Common/DirectoryTree/DirectoryTree.cpp
    fileTransfer.cpp / 
)

add_compile_options(-Wall)

configure_file(CMakeSMPTClient.txt smtpclientlibrary-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/smtpclientlibrary-download )
if(result)
  message(FATAL_ERROR "CMake step for smtpclientlibrary failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/smtpclientlibrary-download )
if(result)
  message(FATAL_ERROR "Build step for smtpclientlibrary failed: ${result}")
endif()
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/smtpclientlibrary-src
                 ${CMAKE_CURRENT_BINARY_DIR}/smtpclientlibrary-build
                 EXCLUDE_FROM_ALL)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/smtpclientlibrary-src/src)

find_package(SQLite3)

include_directories(${SQLite3_INCLUDE_DIRS})

target_link_libraries(
  ${PROJECT_NAME} smtpclient dpp ${SQLite3_LIBRARIES}
)