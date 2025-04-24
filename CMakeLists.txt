cmake_minimum_required(VERSION 3.10)

project(My_window_manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(X11 REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    $(X11_INCLUDE_DIR)
)

set(SOURCES
    main.cpp
    src/Daemon.cpp
    src/tools.cpp
    src/XServerConnection.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
PRIVATE ${X11_LIBRARIES}
)