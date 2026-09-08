#include "sa_rw_dff.h"
namespace browsergamert {
namespace {
uint32_t u32(const uint8_t* p) { return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24); }
bool container(uint32_t t) { switch(t) { case 3: case 5: case 6: case 7: case 8: case 0xB: case 0xE: case 0xF: case 0x10: case 0x12: case 0x14: case 0x1A: return true; default: return false; } }
bool walk(const std::vector<uint8_t>& b, uint32_t begin, uint32_t end, uint32_t depth, std::vector<RwChunk>& out) {
  // Depth is intentionally bounded: malformed RenderWare data must never turn
  // a browser asset request into unbounded recursion.
  if (depth > 8) return true;
  uint32_t at = begin;
  while (at < end) {
    if (end - at < 12) return false;
    const uint32_t type=u32(b.data()+at), size=u32(b.data()+at+4), version=u32(b.data()+at+8);
    if (size > end-at-12) return false;
    out.push_back({type,size,version,at});
    const uint32_t payload=at+12;
    if (container(type) && size >= 12) {
      const uint32_t child = payload;
      if (child + 12 <= payload + size) {
        const uint32_t childSize = u32(b.data()+child+4);
        if (childSize <= size - 12 && childSize >= 0) walk(b,child,payload+size,depth+1,out);
      }
    }
    at=payload+size;
  }
  return at == end;
}
}
bool parseDffChunks(const std::vector<uint8_t>& bytes, std::vector<RwChunk>& out) {
  out.clear(); if (bytes.size()<12 || bytes.size()>256u*1024u*1024u) return false;
  // First parse the clump header, then inventory its immediate child chunks.
  // Keeping this pass iterative is deliberate: game archives are untrusted
  // input and a malformed child must not be able to exhaust the call stack.
  const uint32_t rootSize = u32(bytes.data()+4);
  if (rootSize > bytes.size()-12) return false;
  out.push_back({u32(bytes.data()), rootSize, u32(bytes.data()+8), 0});
  uint32_t at = 12, end = 12 + rootSize;
  while (at + 12 <= end && out.size() < 100000) {
    const uint32_t type=u32(bytes.data()+at), size=u32(bytes.data()+at+4);
    if (size > end-at-12) break;
    out.push_back({type,size,u32(bytes.data()+at+8),at});
    at += 12 + size;
  }
  return !out.empty() && out.front().type==0x10;
}
}
