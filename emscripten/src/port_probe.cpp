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
int sa_runtime_ped_count() { return static_cast<int>(g_runtime.peds().size()); }

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
int sa_runtime_interior_active() { return g_runtime.interior().active ? 1 : 0; }

}

int main() {
    std::puts("GTASA-Emsc Phase 1 probe");
    auto caps = browsergamert::probeCapabilities();
    std::printf("runtime: tier=%s webgl2=%d webgpu=%d threads=%d\n",
        browsergamert::tierName(caps.tier), caps.webgl2, caps.webgpu, caps.threads);
    return 0;
}
