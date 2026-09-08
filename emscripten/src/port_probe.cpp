#include <cstdio>

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

}

int main() {
    std::puts("GTASA-Emsc Phase 1 probe");
    return 0;
}

