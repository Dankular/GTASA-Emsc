#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace browsergamert {
// Renderer-neutral description of one RenderWare texture. Pixel data remains
// owned by the asset mount; payloadOffset/payloadSize identify the first mip.
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

struct RwTexturePixels {
  uint32_t width = 0, height = 0;
  // Always tightly packed RGBA8, top-left origin, first mip level.
  std::vector<uint8_t> rgba;
};

bool parseTxdMetadata(const std::vector<uint8_t>& bytes, std::vector<RwTextureMetadata>& out);
RwTextureUploadPlan makeTextureUploadPlan(const RwTextureMetadata& texture);
bool decodeTxdTexturePixels(const std::vector<uint8_t>& bytes,
                            const RwTextureMetadata& texture,
                            RwTexturePixels& out);
int runTxdMetadataSmoke(const std::string& imgPath);
int runTxdPixelSmoke(const std::string& imgPath);
}
