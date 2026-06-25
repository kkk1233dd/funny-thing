cmake_minimum_required(VERSION 3.16)
project(DesktopCalendarWidget VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME "DesktopCalendar")

set(SOURCES
    src/main.cpp
    src/window.cpp
    src/calendar.cpp
    src/tasks.cpp
    src/countdown.cpp
    src/themes.cpp
    src/utils.cpp
)

set(HEADERS
    include/window.h
    include/themes.h
    include/utils.h
)

add_executable(${APP_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${APP_NAME} PRIVATE
    gdi32
    user32
    kernel32
    comctl32
    msimg32
    dwmapi
)

target_compile_definitions(${APP_NAME} PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MINGW)
    target_link_options(${APP_NAME} PRIVATE
        -mwindows
        -static-libgcc
        -static-libstdc++
    )
endif()

set_target_properties(${APP_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
