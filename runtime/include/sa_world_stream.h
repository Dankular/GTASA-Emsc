#pragma once
#include "sa_img_archive.h"
#include <cstdint>
#include <string>
#include <vector>

namespace browsergamert {
struct CollisionBounds { float minX=0,minY=0,minZ=0,maxX=0,maxY=0,maxZ=0; };
struct CollisionMesh { CollisionBounds bounds{}; uint32_t vertices=0, triangles=0; };
struct StreamRequest { std::string name; int priority=0; };

// Small, deterministic replacement for the game's sector streamer. Requests are
// prioritized and bounded; the browser can feed the same queue from RangeVfs.
class WorldStream {
public:
  bool open(const std::string& imgPath);
  bool request(const std::string& name, int priority);
  bool pump(uint32_t budget=1);
  bool loaded(const std::string& name) const;
  const std::vector<uint8_t>* asset(const std::string& name) const;
  const CollisionMesh* collision(const std::string& name) const;
  size_t pending() const { return pending_.size(); }
  size_t loadedCount() const { return loaded_.size(); }
  const std::vector<ImgEntry>& entries() const { return archive_.entries(); }
private:
  struct Loaded { std::string name; std::vector<uint8_t> bytes; CollisionMesh collision{}; bool hasCollision=false; };
  ImgArchive archive_;
  std::vector<StreamRequest> pending_;
  std::vector<Loaded> loaded_;
};
bool parseColCollision(const std::vector<uint8_t>& bytes, CollisionMesh& out);
int runWorldStreamSmoke(const std::string& imgPath);
}
