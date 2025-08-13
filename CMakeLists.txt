cmake_minimum_required(VERSION 3.31)
project(CLI_Utilites)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(CLI_Utilites src/main.cpp
        src/battery.cpp
        include/battery.h
        src/cleaner.cpp
        include/cleaner.h
        src/wifi.cpp
        include/wifi.h
        src/devices.cpp
        include/devices.h)
add_executable(TEST src/main.cpp)

target_compile_options(CLI_Utilites PRIVATE -Wextra -Werror -fsanitize=address)
target_link_options(CLI_Utilites PRIVATE -fsanitize=address)
