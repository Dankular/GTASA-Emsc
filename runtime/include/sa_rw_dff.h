#pragma once
#include <cstdint>
#include <vector>

namespace browsergamert {
struct RwChunk { uint32_t type = 0, size = 0, version = 0, offset = 0; };
struct RwRenderMesh {
  std::vector<float> positions; // xyz, one triplet per vertex
  std::vector<float> normals;   // xyz, empty when the source has no normals
  std::vector<float> texcoords; // uv, first texture-coordinate set
  std::vector<uint32_t> indices; // triangle list
  float bounds[6] = {0, 0, 0, 0, 0, 0}; // min xyz, max xyz
  uint32_t flags = 0;
};
bool parseDffChunks(const std::vector<uint8_t>& bytes, std::vector<RwChunk>& out);
// Extract the first non-native RenderWare geometry into a renderer-neutral mesh.
// GTA SA's DFF geometry payload is little-endian on disk.
bool parseDffRenderMesh(const std::vector<uint8_t>& bytes, RwRenderMesh& out);
}
