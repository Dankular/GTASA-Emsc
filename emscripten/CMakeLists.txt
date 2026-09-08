cmake_minimum_required(VERSION 3.20)
project(GTASA_Emsc_Probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sa_port_probe src/port_probe.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/browser_game_rt.cpp)
target_include_directories(sa_port_probe PRIVATE ../runtime/include)
target_compile_definitions(sa_port_probe PRIVATE BROWSER_GAME_RT_PROBE=1)

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
        "-sEXPORTED_FUNCTIONS=['_main','_sa_port_phase','_sa_port_is_emscripten','_sa_port_renderer_tier']"
    )
else()
    message(STATUS "Configuring the native probe; use emcmake for the browser artifact")
endif()
