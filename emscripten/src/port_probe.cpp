#include <cstdio>
#include "browser_game_rt.h"

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

}

int main() {
    std::puts("GTASA-Emsc Phase 1 probe");
    auto caps = browsergamert::probeCapabilities();
    std::printf("runtime: tier=%s webgl2=%d webgpu=%d threads=%d\n",
        browsergamert::tierName(caps.tier), caps.webgl2, caps.webgpu, caps.threads);
    return 0;
}
