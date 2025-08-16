cmake_minimum_required(VERSION 3.31)
project(CLI_Utilites)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(CLI_Utilites src/main.cpp
        src/BatteryMonitor.cpp
        src/Cleaner.cpp
        src/WifiMonitor.cpp
        src/DeviceWatcher.cpp
        include/PrintUtilites.h
)

target_compile_options(CLI_Utilites PRIVATE -Wextra -Werror)
target_include_directories(CLI_Utilites PRIVATE interface)
target_include_directories(CLI_Utilites PRIVATE include)
target_link_libraries(CLI_Utilites
        PRIVATE
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework CoreGraphics"
)
