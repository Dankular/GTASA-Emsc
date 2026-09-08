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
    auto it = std::find_if(archive.entries().begin(), archive.entries().end(), [](const browsergamert::ImgEntry& e) {
      return e.name.size() >= 4 && e.name.substr(e.name.size()-4) == ".dff";
    });
    if (it != archive.entries().end()) std::printf("candidate=%s size=%llu\n", it->name.c_str(), (unsigned long long)it->size), std::fflush(stdout);
    std::vector<uint8_t> dff; std::vector<browsergamert::RwChunk> chunks;
    if (it == archive.entries().end() || !archive.read(it->name, 0, it->size, dff) || !browsergamert::parseDffChunks(dff, chunks) || chunks.size() < 2) result = 11;
    else std::printf("dff=%s chunks=%zu root=0x%X\n", it->name.c_str(), chunks.size(), chunks.front().type);
  }
  std::printf("sa_img_archive_smoke=%d\n", result);
  return result;
}
