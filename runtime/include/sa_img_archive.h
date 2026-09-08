#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace browsergamert {
struct ImgEntry { std::string name; uint64_t offset=0; uint64_t size=0; };
class ImgArchive {
public:
  bool open(const std::string& path);
  const std::vector<ImgEntry>& entries() const { return entries_; }
  bool read(const std::string& name, uint64_t offset, uint64_t size, std::vector<uint8_t>& out) const;
private:
  std::string path_;
  std::vector<ImgEntry> entries_;
};
int runImgArchiveSmoke(const std::string& path);
}
