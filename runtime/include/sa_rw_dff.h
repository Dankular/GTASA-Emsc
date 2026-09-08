#pragma once
#include <cstdint>
#include <vector>

namespace browsergamert {
struct RwChunk { uint32_t type = 0, size = 0, version = 0, offset = 0; };
bool parseDffChunks(const std::vector<uint8_t>& bytes, std::vector<RwChunk>& out);
}
