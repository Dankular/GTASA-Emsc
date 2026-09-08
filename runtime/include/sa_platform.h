#pragma once
#include <cstdint>
#include <string_view>

namespace browsergamert {
struct PlatformInfo {
  std::string_view name;
  char pathSeparator;
  bool littleEndian;
  bool wasm;
};

// The runtime consumes this small contract instead of including Win32,
// POSIX, or Emscripten headers in gameplay/asset code.
PlatformInfo platformInfo();
uint64_t monotonicMilliseconds();
void yieldThread();
}
