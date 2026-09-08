#include "sa_platform.h"
#include <chrono>
#include <thread>

namespace browsergamert {
PlatformInfo platformInfo() {
#if defined(__EMSCRIPTEN__)
  return {"emscripten", '/', true, true};
#elif defined(_WIN32)
  return {"windows", '\\', true, false};
#elif defined(__linux__)
  return {"linux", '/', true, false};
#elif defined(__APPLE__)
  return {"macos", '/', true, false};
#else
  return {"unknown", '/', true, false};
#endif
}

uint64_t monotonicMilliseconds() {
  const auto now=std::chrono::steady_clock::now().time_since_epoch();
  return uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

void yieldThread() {
#if defined(__EMSCRIPTEN__)
  // A browser frame is the scheduling boundary; sleeping would block it.
  return;
#else
  std::this_thread::yield();
#endif
}
}
