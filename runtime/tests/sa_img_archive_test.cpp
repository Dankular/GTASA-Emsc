#include "sa_img_archive.h"
#include "sa_rw_dff.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\models\\gta3.img";
  int result = browsergamert::runImgArchiveSmoke(path);
  if (result == 0) {
    browsergamert::ImgArchive archive;
    if (!archive.open(path)) result = 10;
    std::vector<uint8_t> dff; std::vector<browsergamert::RwChunk> chunks;
    browsergamert::RwRenderMesh mesh; bool found = false; size_t candidates = 0;
    for (const auto& entry : archive.entries()) {
      if (entry.name.size() < 4 || entry.name.substr(entry.name.size()-4) != ".dff") continue;
      ++candidates;
      if (!archive.read(entry.name, 0, entry.size, dff) || !browsergamert::parseDffChunks(dff, chunks) || chunks.size() < 2) continue;
      if (browsergamert::parseDffRenderMesh(dff, mesh) && !mesh.positions.empty() && !mesh.indices.empty()) {
        std::printf("dff=%s chunks=%zu root=0x%X vertices=%zu triangles=%zu\n", entry.name.c_str(), chunks.size(), chunks.front().type, mesh.positions.size()/3, mesh.indices.size()/3);
        found = true; break;
      }
    }
    if (candidates == 0) result = 11;
    else if (!found) result = 12;
  }
  std::printf("sa_img_archive_smoke=%d\n", result);
  return result;
}
