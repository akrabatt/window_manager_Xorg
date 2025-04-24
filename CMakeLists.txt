cmake_minimum_required(VERSION 3.10)

project(My_window_manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    main.cpp
    ./src/Daemon.cpp
    ./src/tools.cpp
    ./src/XServerConnection.cpp
    )

add_executable(My_window_manager ${SOURCES})

set(CMAKE_BUILD_TYPE Debug)