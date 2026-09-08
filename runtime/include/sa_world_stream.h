#pragma once
#include "sa_img_archive.h"
#include "sa_map.h"
#include <cstdint>
#include <utility>
#include <string>
#include <vector>

namespace browsergamert {
struct CollisionBounds { float minX=0,minY=0,minZ=0,maxX=0,maxY=0,maxZ=0; };
struct CollisionVec3 { float x=0,y=0,z=0; };
struct CollisionFace { uint32_t a=0,b=0,c=0; };
struct CollisionMesh { CollisionBounds bounds{}; uint32_t vertices=0, triangles=0; std::vector<CollisionVec3> points; std::vector<CollisionFace> faces; };
struct WorldStreamRequest { std::string name; int priority=0; int32_t sectorX=0; int32_t sectorY=0; };
struct CollisionHit { std::string name; float distance=0; CollisionBounds bounds{}; };

// Small, deterministic replacement for the game's sector streamer. Requests are
// prioritized and bounded; the browser can feed the same queue from RangeVfs.
class WorldStream {
public:
  bool open(const std::string& imgPath);
  bool loadIde(const std::string& path);
  bool loadIpl(const std::string& path);
  bool request(const std::string& name, int priority);
  // Associate a request with a world sector. The sector is retained for
  // deterministic scheduling/telemetry while priority remains authoritative.
  bool requestAt(const std::string& name, int priority, int32_t sectorX, int32_t sectorY);
  bool requestSector(int32_t sectorX, int32_t sectorY, int priority);
  bool pump(uint32_t budget=1);
  bool loaded(const std::string& name) const;
  const std::vector<uint8_t>* asset(const std::string& name) const;
  const CollisionMesh* collision(const std::string& name) const;
  size_t queryAabb(const CollisionBounds& area, std::vector<CollisionHit>& out) const;
  bool raycast(const CollisionVec3& origin, const CollisionVec3& direction,
               float maxDistance, CollisionHit& out) const;
  static std::pair<int32_t,int32_t> sectorFor(float x, float y, float sectorSize=300.0f);
  size_t pending() const { return pending_.size(); }
  size_t loadedCount() const { return loaded_.size(); }
  const std::vector<ImgEntry>& entries() const { return archive_.entries(); }
  const std::vector<IdeDefinition>& definitions() const { return definitions_; }
  const std::vector<IplInstance>& instances() const { return instances_; }
private:
  struct Loaded { std::string name; std::vector<uint8_t> bytes; CollisionMesh collision{}; bool hasCollision=false; };
  ImgArchive archive_;
  std::vector<WorldStreamRequest> pending_;
  std::vector<Loaded> loaded_;
  std::vector<IdeDefinition> definitions_;
  std::vector<IplInstance> instances_;
};
bool parseColCollision(const std::vector<uint8_t>& bytes, CollisionMesh& out);
int runWorldStreamSmoke(const std::string& imgPath);
}
