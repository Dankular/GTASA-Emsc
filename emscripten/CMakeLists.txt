cmake_minimum_required(VERSION 3.20)
project(GTASA_Emsc_Probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sa_port_probe src/port_probe.cpp)

if(EMSCRIPTEN)
    set_target_properties(sa_port_probe PROPERTIES SUFFIX ".js")
    target_link_options(sa_port_probe PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sENVIRONMENT=web,worker"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sUSE_WEBGL2=1"
        "-sFORCE_FILESYSTEM=1"
        "-sEXPORTED_RUNTIME_METHODS=['FS']"
    )
else()
    message(STATUS "Configuring the native probe; use emcmake for the browser artifact")
endif()

