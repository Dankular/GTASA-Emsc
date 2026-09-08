#include "sa_platform.h"
#include <cassert>
#include <cstdio>
int main() {
  const auto p=browsergamert::platformInfo();
  assert(!p.name.empty() && (p.pathSeparator=='/' || p.pathSeparator=='\\'));
  const auto a=browsergamert::monotonicMilliseconds(); browsergamert::yieldThread(); const auto b=browsergamert::monotonicMilliseconds();
  assert(b>=a);
  std::printf("sa_platform=%.*s separator=%c wasm=%d\n",int(p.name.size()),p.name.data(),p.pathSeparator,p.wasm?1:0);
  return 0;
}
