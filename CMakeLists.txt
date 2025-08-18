cmake_minimum_required(VERSION 3.31)
project(cliutils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cliutils src/main.cpp
        src/BatteryMonitor.cpp
        src/Cleaner.cpp
        src/WifiMonitor.cpp
        src/DeviceWatcher.cpp
        src/CodeCounter.cpp
        include/utils/Structs.h
        interface/ISystemInfo.h
        include/monitors/SystemInfo.h
        src/SystemInfo.cpp
)

target_compile_options(cliutils PRIVATE -Wextra -Werror)
target_include_directories(cliutils PRIVATE interface)
target_include_directories(cliutils PRIVATE include/monitors include/utils)
target_link_libraries(cliutils
        PRIVATE
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework CoreGraphics"
)

install(TARGETS cliutils RUNTIME DESTINATION /usr/local/bin)
