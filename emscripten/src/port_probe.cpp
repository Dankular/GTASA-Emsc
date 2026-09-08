#include <cstdio>
#include "browser_game_rt.h"
#include "sa_subsystem_bridge.h"
#include "sa_adapters.h"
#include "sa_services.h"

namespace {
browsergamert::SaAdapters g_runtime;
bool g_runtime_ready = false;
}

#ifdef __EMSCRIPTEN__
#    include <emscripten/emscripten.h>
#endif

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_port_phase() {
    return 1;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_port_is_emscripten() {
#    ifdef __EMSCRIPTEN__
    return 1;
#    else
    return 0;
#    endif
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_port_renderer_tier() {
    return static_cast<int>(browsergamert::probeCapabilities().tier);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_subsystems_boundary_ready() {
    return browsergamert::SaSubsystemBridge{}.allBoundariesReady() ? 1 : 0;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_adapter_smoke() { return browsergamert::runSaAdapterSmoke(); }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_services_smoke() { return browsergamert::runServiceSmoke(); }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_init() { g_runtime_ready = g_runtime.initialize(); return g_runtime_ready ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_tick(float dt, float steering, float throttle, float brake) {
    if (!g_runtime_ready && sa_runtime_init() != 0) return 1;
    g_runtime.tick(dt, {steering, throttle, brake, false});
    return 0;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_tick_camera(float dt, float steering, float throttle, float brake, float cameraYaw, float cameraPitch) {
    if (!g_runtime_ready && sa_runtime_init() != 0) return 1;
    g_runtime.tick(dt, {steering, throttle, brake, false, cameraYaw, cameraPitch});
    return 0;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_enter_vehicle(int model) { return g_runtime.enterVehicle(static_cast<uint32_t>(model), {}) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_enter_interior(int id) { return g_runtime.enterInterior(id) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_script_smoke() { return browsergamert::runSaAdapterSmoke(); }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_dispatch_script(int opcode) { return g_runtime.dispatchScript(static_cast<uint16_t>(opcode)) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_run_mission() {
    // Compact, deterministic mission used by the browser acceptance route:
    // spawn the player vehicle, spawn a pedestrian, assign its walk task,
    // lower health, then enter interior 3.
    const std::vector<uint16_t> mission{1, 411, 4, 7, 5, 1, 6, 80, 3, 3, 0};
    return g_runtime.runScript(mission) ? 0 : 1;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_spawn_ped(int model) { return g_runtime.spawnPed(static_cast<uint32_t>(model), {}) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_set_ped_task(int index, int task) { return g_runtime.setPedTask(static_cast<uint32_t>(index), static_cast<uint16_t>(task)) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_set_ped_health(int index, float health) { return g_runtime.setPedHealth(static_cast<uint32_t>(index), health) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_ped_count() { return static_cast<int>(g_runtime.peds().size()); }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_ped_task(int index) { return index >= 0 && static_cast<size_t>(index) < g_runtime.peds().size() ? g_runtime.peds()[static_cast<size_t>(index)].task : -1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_ped_health(int index) { return index >= 0 && static_cast<size_t>(index) < g_runtime.peds().size() ? g_runtime.peds()[static_cast<size_t>(index)].health : -1.f; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_save(const char* slot) { return slot && g_runtime.save(slot) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_load(const char* slot) { return slot && g_runtime.load(slot) ? 0 : 1; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_vehicle_x() { return g_runtime.vehicle().position.x; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_vehicle_y() { return g_runtime.vehicle().position.y; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_vehicle_heading() { return g_runtime.vehicle().heading; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_vehicle_speed() { return g_runtime.vehicle().speed; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_x() { return g_runtime.camera().position.x; }
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_y() { return g_runtime.camera().position.y; }
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_z() { return g_runtime.camera().position.z; }
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_look_x() { return g_runtime.camera().lookAt.x; }
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_look_y() { return g_runtime.camera().lookAt.y; }
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
float sa_runtime_camera_look_z() { return g_runtime.camera().lookAt.z; }

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int sa_runtime_interior_active() { return g_runtime.interior().active ? 1 : 0; }

}

int main() {
    std::puts("GTASA-Emsc browser runtime");
    auto caps = browsergamert::probeCapabilities();
    std::printf("runtime: tier=%s webgl2=%d webgpu=%d threads=%d state=menu-ready\n",
        browsergamert::tierName(caps.tier), caps.webgl2, caps.webgpu, caps.threads);
    return 0;
}
