#include "sa_img_archive.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace browsergamert {
bool ImgArchive::open(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  char magic[4]{}; uint32_t count=0;
  in.read(magic, 4); in.read(reinterpret_cast<char*>(&count), 4);
  if (!in || std::memcmp(magic, "VER2", 4) != 0 || count > 1000000) return false;
  std::vector<ImgEntry> parsed; parsed.reserve(count);
  for (uint32_t i=0; i<count; ++i) {
    // IMG v2 directory records are 32 bytes: block offset, block length,
    // and a 24-byte NUL-padded filename. There is no third integer field.
    uint32_t block=0, size=0; char name[24]{};
    in.read(reinterpret_cast<char*>(&block), 4);
    in.read(reinterpret_cast<char*>(&size), 4);
    in.read(name, sizeof(name));
    if (!in) return false;
    const auto end = std::find(name, name + sizeof(name), '\0');
    parsed.push_back({std::string(name, end), uint64_t(block) * 2048ull, uint64_t(size) * 2048ull});
  }
  path_ = path; entries_ = std::move(parsed); return true;
}

bool ImgArchive::read(const std::string& name, uint64_t offset, uint64_t size, std::vector<uint8_t>& out) const {
  const auto it = std::find_if(entries_.begin(), entries_.end(), [&](const ImgEntry& e){ return e.name == name; });
  if (it == entries_.end() || offset > it->size || size > it->size - offset) return false;
  std::ifstream in(path_, std::ios::binary); if (!in) return false;
  in.seekg(static_cast<std::streamoff>(it->offset + offset));
  out.resize(static_cast<size_t>(size));
  in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(size));
  return in.good() || in.eof() && in.gcount() == static_cast<std::streamsize>(size);
}

int runImgArchiveSmoke(const std::string& path) {
  ImgArchive archive; if (!archive.open(path) || archive.entries().empty()) return 1;
  std::vector<uint8_t> bytes; if (!archive.read(archive.entries().front().name, 0, std::min<uint64_t>(16, archive.entries().front().size), bytes)) return 2;
  return bytes.empty() ? 3 : 0;
}
}
