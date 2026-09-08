#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace browsergamert {
// Renderer-neutral description of one RenderWare texture.  Pixel decoding is
// deliberately a later stage; this contract is enough for a WebGPU texture
// allocator to choose dimensions, mip policy, and an upload format.
struct RwTextureMetadata {
  std::string name;
  std::string mask;
  uint32_t width = 0, height = 0, depth = 0, mipLevels = 0;
  uint32_t rasterFormat = 0, compression = 0;
  bool hasAlpha = false;
  uint32_t rasterType = 0;
  uint64_t payloadOffset = 0, payloadSize = 0;
};

struct RwTextureUploadPlan {
  uint32_t width = 0, height = 0, mipLevels = 1;
  uint32_t bytesPerRow = 0;
  bool compressed = false, hasAlpha = false;
  std::string format; // WebGPU format or a decoder input format.
};

bool parseTxdMetadata(const std::vector<uint8_t>& bytes, std::vector<RwTextureMetadata>& out);
RwTextureUploadPlan makeTextureUploadPlan(const RwTextureMetadata& texture);
int runTxdMetadataSmoke(const std::string& imgPath);
}
