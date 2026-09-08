#include "sa_rw_dff.h"
#include <algorithm>
#include <cmath>
#include <cstring>
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

bool f32(const std::vector<uint8_t>& b, uint32_t at, float& out) {
  if (at > b.size() || b.size() - at < 4) return false;
  uint32_t bits = u32(b.data() + at);
  static_assert(sizeof(float) == sizeof(uint32_t));
  std::memcpy(&out, &bits, sizeof(out));
  return true;
}

bool geometry(const std::vector<uint8_t>& b, const RwChunk& c, RwRenderMesh& out) {
  if (c.type != 0x0F || c.offset > b.size() || c.size > b.size() - c.offset - 12) return false;
  uint32_t begin = c.offset + 12, end = begin + c.size;
  if (c.size < 16) return false;
  // PC SA DFFs wrap geometry data in a RenderWare Struct child. Keep the
  // direct-payload form too for portable fixtures.
  if (end - begin >= 12 && u32(b.data()+begin) == 1) {
    const uint32_t structSize = u32(b.data()+begin+4);
    if (structSize > end - begin - 12) return false;
    begin += 12;
    end = begin + structSize;
  }
  uint32_t at = begin;
  const uint32_t flags = u32(b.data()+at); at += 4;
  const uint32_t triangles = u32(b.data()+at); at += 4;
  const uint32_t vertices = u32(b.data()+at); at += 4;
  const uint32_t morphs = u32(b.data()+at); at += 4;
  if (vertices == 0 || vertices > 10000000u || triangles > 20000000u || morphs == 0 || morphs > 16u) return false;
  // Native geometry has a different payload and cannot be interpreted as the
  // portable float arrays below.
  if (flags & 0x01000000u) return false;
  const bool prelit = (flags & 0x8u) != 0;
  uint32_t uvSets = (flags >> 16) & 0xFFu;
  if (uvSets == 0 && (flags & 0x4u)) uvSets = 1;
  if (uvSets > 8) return false;
  auto take = [&](uint64_t n) -> bool { return n <= uint64_t(end - at) ? (at += uint32_t(n), true) : false; };
  if (prelit && !take(uint64_t(vertices) * 4u)) return false;
  // UV coordinates are stored as two float32 values per vertex and set.
  uint32_t uvBytes = uint32_t(uint64_t(vertices) * uvSets * 8u);
  if (uvBytes > end-at) return false;
  uint32_t uvAt = at;
  if (!take(uvBytes)) return false;
  // Triangle records precede the morph target. Keep the winding as authored;
  // the browser renderer can choose its front-face convention at upload time.
  uint64_t triBytes = uint64_t(triangles) * 8u;
  if (triBytes > end-at) return false;
  uint32_t triAt = at; at += uint32_t(triBytes);
  if (!take(16)) return false; // bounding sphere
  const uint32_t hasVertices = u32(b.data()+at); at += 4;
  const uint32_t hasNormals = u32(b.data()+at); at += 4;
  if (!hasVertices || uint64_t(vertices)*12u > uint64_t(end-at)) return false;
  out = {};
  out.flags = flags;
  out.positions.resize(uint64_t(vertices)*3u);
  out.indices.resize(uint64_t(triangles)*3u);
  if (uvSets) out.texcoords.resize(uint64_t(vertices)*2u);
  float minv[3] = {INFINITY, INFINITY, INFINITY}, maxv[3] = {-INFINITY, -INFINITY, -INFINITY};
  for (uint32_t i=0; i<vertices; ++i) {
    for (uint32_t k=0; k<3; ++k) { float v; if (!f32(b, at, v)) return false; at += 4; out.positions[i*3+k]=v; minv[k]=std::min(minv[k],v); maxv[k]=std::max(maxv[k],v); }
  }
  if (hasNormals) {
    if (uint64_t(vertices)*12u > uint64_t(end-at)) return false;
    out.normals.resize(uint64_t(vertices)*3u);
    for (float& v : out.normals) { if (!f32(b,at,v)) return false; at += 4; }
  }
  for (uint32_t i=0; i<triangles; ++i) {
    uint16_t a = uint16_t(b[triAt] | (b[triAt+1]<<8)); uint16_t bb = uint16_t(b[triAt+2] | (b[triAt+3]<<8)); uint16_t d = uint16_t(b[triAt+4] | (b[triAt+5]<<8)); triAt += 8;
    if (a >= vertices || bb >= vertices || d >= vertices) return false;
    out.indices[i*3] = a; out.indices[i*3+1] = bb; out.indices[i*3+2] = d;
  }
  for (uint32_t i=0; i<vertices && uvSets; ++i) { float u,v; if (!f32(b,uvAt,u)||!f32(b,uvAt+4,v)) return false; uvAt += uvSets*8; out.texcoords[i*2]=u; out.texcoords[i*2+1]=v; }
  for (int k=0;k<3;++k) { out.bounds[k]=minv[k]; out.bounds[3+k]=maxv[k]; }
  return true;
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
  // Recursively inventory nested clump/frame/geometry-list children so
  // callers can locate geometry without knowing the DFF's chunk ordering.
  // A malformed nested extension must not discard the valid root inventory;
  // the parser still validates every geometry payload before exposing it.
  (void)walk(bytes, 0, uint32_t(bytes.size()), 0, out);
  return !out.empty() && out.front().type==0x10;
}

bool parseDffRenderMesh(const std::vector<uint8_t>& bytes, RwRenderMesh& out) {
  out = {};
  std::vector<RwChunk> chunks;
  if (!parseDffChunks(bytes, chunks)) return false;
  for (const auto& c : chunks) if (c.type == 0x0F && geometry(bytes, c, out)) return true;
  // Some SA exporters wrap the geometry list in extension chunks that do not
  // advertise themselves as containers. A bounded 4-byte header scan keeps
  // those assets loadable while still requiring a fully validated geometry.
  for (uint32_t at = 0; at + 12 <= bytes.size(); at += 4) {
    if (u32(bytes.data()+at) != 0x0F) continue;
    const uint32_t size = u32(bytes.data()+at+4);
    if (size <= bytes.size() - at - 12 && geometry(bytes, {0x0F,size,u32(bytes.data()+at+8),at}, out)) return true;
  }
  return false;
}
}
