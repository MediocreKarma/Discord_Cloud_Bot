cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME Client_MyDiscordDrive)

project (${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_executable(
    ${PROJECT_NAME} /
    client.cpp /
    GUI_Objects/Buttons.cpp /
    GUI_Objects/RoundedRectangleShape.cpp /
    GUI_Objects/TextBox.cpp /
    GUI_Objects/utils.cpp /
    login.cpp /
    ../Common/commons.cpp /
    ../Common/DirectoryTree/DirectoryTree.cpp /
    gui.cpp /
)

add_compile_options(-Wall)

target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system dpp)