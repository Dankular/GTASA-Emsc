cmake_minimum_required(VERSION 3.20)
project(GTASA_Emsc_Probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sa_port_probe src/port_probe.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/browser_game_rt.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_subsystem_bridge.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_adapters.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_services.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_world_stream.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_img_archive.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_rw_dff.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_rw_txd.cpp)
target_sources(sa_port_probe PRIVATE ../runtime/src/sa_platform.cpp)
target_include_directories(sa_port_probe PRIVATE ../runtime/include)
target_compile_definitions(sa_port_probe PRIVATE BROWSER_GAME_RT_PROBE=1)

if(NOT EMSCRIPTEN)
    add_executable(sa_adapter_test ../runtime/tests/sa_adapter_test.cpp ../runtime/src/sa_adapters.cpp)
    target_include_directories(sa_adapter_test PRIVATE ../runtime/include)
    add_executable(sa_services_test ../runtime/tests/sa_services_test.cpp ../runtime/src/sa_services.cpp ../runtime/src/sa_adapters.cpp)
    target_include_directories(sa_services_test PRIVATE ../runtime/include)
    add_executable(sa_img_archive_test ../runtime/tests/sa_img_archive_test.cpp ../runtime/src/sa_img_archive.cpp ../runtime/src/sa_rw_dff.cpp)
    target_include_directories(sa_img_archive_test PRIVATE ../runtime/include)
    target_sources(sa_img_archive_test PRIVATE ../runtime/src/sa_rw_txd.cpp)
    add_executable(sa_txd_test ../runtime/tests/sa_txd_test.cpp ../runtime/src/sa_img_archive.cpp ../runtime/src/sa_rw_txd.cpp)
    target_include_directories(sa_txd_test PRIVATE ../runtime/include)
    add_executable(sa_world_stream_test ../runtime/tests/sa_world_stream_test.cpp ../runtime/src/sa_world_stream.cpp ../runtime/src/sa_img_archive.cpp)
    target_include_directories(sa_world_stream_test PRIVATE ../runtime/include)
    add_executable(sa_camera_physics_test ../runtime/tests/sa_camera_physics_test.cpp ../runtime/src/sa_adapters.cpp ../runtime/src/sa_world_stream.cpp ../runtime/src/sa_img_archive.cpp)
    target_include_directories(sa_camera_physics_test PRIVATE ../runtime/include)
    add_executable(sa_platform_test ../runtime/tests/sa_platform_test.cpp ../runtime/src/sa_platform.cpp)
    target_include_directories(sa_platform_test PRIVATE ../runtime/include)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(sa_port_probe PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
        "-sEXPORTED_RUNTIME_METHODS=['FS','stringToNewUTF8']"
        "-sEXPORTED_FUNCTIONS=['_main','_free','_sa_port_phase','_sa_port_is_emscripten','_sa_port_renderer_tier','_sa_subsystems_boundary_ready','_sa_adapter_smoke','_sa_services_smoke','_sa_runtime_init','_sa_runtime_tick','_sa_runtime_tick_camera','_sa_runtime_enter_vehicle','_sa_runtime_enter_interior','_sa_runtime_script_smoke','_sa_runtime_dispatch_script','_sa_runtime_run_mission','_sa_runtime_spawn_ped','_sa_runtime_set_ped_task','_sa_runtime_set_ped_health','_sa_runtime_ped_count','_sa_runtime_ped_task','_sa_runtime_ped_health','_sa_runtime_save','_sa_runtime_load','_sa_runtime_vehicle_x','_sa_runtime_vehicle_y','_sa_runtime_vehicle_heading','_sa_runtime_vehicle_speed','_sa_runtime_camera_x','_sa_runtime_camera_y','_sa_runtime_camera_z','_sa_runtime_camera_look_x','_sa_runtime_camera_look_y','_sa_runtime_camera_look_z','_sa_runtime_interior_active']"
    )
else()
    message(STATUS "Configuring the native probe; use emcmake for the browser artifact")
endif()
