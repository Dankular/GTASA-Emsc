#include "sa_world_stream.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>

namespace browsergamert {
namespace {
uint32_t u32(const uint8_t* p) { return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24); }
float f32(const uint8_t* p) { float v; std::memcpy(&v,p,4); return v; }
bool finite(float v) { return v==v && v > -1.0e7f && v < 1.0e7f; }
bool suffix(const std::string& s,const char* x) { return s.size()>=4 && s.compare(s.size()-4,4,x)==0; }
}

bool parseColCollision(const std::vector<uint8_t>& b, CollisionMesh& out) {
  out = {};
  if (b.size() < 40) return false;
  if (std::memcmp(b.data(), "COLL", 4) && std::memcmp(b.data(), "COL2", 4) && std::memcmp(b.data(), "COL3", 4) && std::memcmp(b.data(), "COL4", 4)) return false;
  // SA's compact COL3 files use a variable-length model name and a sequence
  // of sphere records. They are commonly found in veh_mods.col and do not use
  // the fixed COL1 header layout below.
  if (!std::memcmp(b.data(), "COL3", 4) && b.size() >= 24) {
    const uint32_t recordSize=u32(b.data()+4);
    const uint8_t* name=b.data()+8; size_t nameEnd=8;
    while (nameEnd < b.size() && nameEnd < 128 && b[nameEnd]) ++nameEnd;
    if (nameEnd < b.size() && nameEnd+4 <= b.size()) {
      const uint32_t spheres=u32(b.data()+nameEnd+1);
      if (spheres && spheres <= 10000 && nameEnd+5+uint64_t(spheres)*16 <= b.size() && (!recordSize || recordSize <= b.size())) {
        out.vertices=spheres; out.triangles=0;
        out.bounds.minX=out.bounds.minY=out.bounds.minZ=std::numeric_limits<float>::max();
        out.bounds.maxX=out.bounds.maxY=out.bounds.maxZ=std::numeric_limits<float>::lowest();
        for (uint32_t i=0;i<spheres;++i) {
          const uint8_t* p=b.data()+nameEnd+5+size_t(i)*16; float x=f32(p),y=f32(p+4),z=f32(p+8),r=f32(p+12);
          // Some COL3 producers store an extra packed material word in the
          // sphere record. Keep the count as authoritative and only consume
          // coordinates when they are sane; malformed optional bounds must
          // not make the whole model unstreamable.
          if (!finite(x)||!finite(y)||!finite(z)||!finite(r)||r<0||r>1.0e6f) { x=y=z=0; r=0; }
          out.bounds.minX=std::min(out.bounds.minX,x-r); out.bounds.minY=std::min(out.bounds.minY,y-r); out.bounds.minZ=std::min(out.bounds.minZ,z-r);
          out.bounds.maxX=std::max(out.bounds.maxX,x+r); out.bounds.maxY=std::max(out.bounds.maxY,y+r); out.bounds.maxZ=std::max(out.bounds.maxZ,z+r);
        }
        return true;
      }
    }
  }
  // All SA COL revisions retain the name then sphere/box/vertex/face counts.
  // COL2+ inserts a model id before the counts, so accept both layouts and
  // validate counts against the file size before exposing any geometry.
  const uint32_t layouts[] = {28u, 30u};
  for (uint32_t h : layouts) {
    if (h + 16 > b.size()) continue;
    uint32_t spheres=u32(b.data()+h), boxes=u32(b.data()+h+4), verts=u32(b.data()+h+8), faces=u32(b.data()+h+12);
    if (spheres > 100000 || boxes > 100000 || verts > 2000000 || faces > 2000000) continue;
    const uint64_t minimum = uint64_t(h)+16 + uint64_t(spheres)*20 + uint64_t(boxes)*28 + uint64_t(verts)*12 + uint64_t(faces)*8;
    if (minimum > b.size()) continue;
    out.vertices=verts; out.triangles=faces;
    // Bounds are represented by the vertex cloud. This avoids trusting the
    // optional header bounds across COL revisions and gives collision queries
    // one consistent representation in native and WASM builds.
    const size_t vertexAt = size_t(h)+16 + size_t(spheres)*20 + size_t(boxes)*28;
    if (verts) {
      out.bounds.minX=out.bounds.minY=out.bounds.minZ=std::numeric_limits<float>::max();
      out.bounds.maxX=out.bounds.maxY=out.bounds.maxZ=std::numeric_limits<float>::lowest();
      for (uint32_t i=0;i<verts;++i) {
        const uint8_t* p=b.data()+vertexAt+size_t(i)*12; float x=f32(p),y=f32(p+4),z=f32(p+8);
        if (!finite(x)||!finite(y)||!finite(z)) return false;
        out.bounds.minX=std::min(out.bounds.minX,x); out.bounds.minY=std::min(out.bounds.minY,y); out.bounds.minZ=std::min(out.bounds.minZ,z);
        out.bounds.maxX=std::max(out.bounds.maxX,x); out.bounds.maxY=std::max(out.bounds.maxY,y); out.bounds.maxZ=std::max(out.bounds.maxZ,z);
      }
    }
    return true;
  }
  return false;
}

bool WorldStream::open(const std::string& path) { pending_.clear(); loaded_.clear(); return archive_.open(path); }
bool WorldStream::request(const std::string& name, int priority) {
  if (!archive_.entries().size() || loaded(name)) return false;
  auto it=std::find_if(archive_.entries().begin(),archive_.entries().end(),[&](const ImgEntry&e){return e.name==name;});
  if (it==archive_.entries().end()) return false;
  auto q=std::find_if(pending_.begin(),pending_.end(),[&](const StreamRequest&r){return r.name==name;});
  if(q!=pending_.end()){q->priority=std::max(q->priority,priority);return true;}
  pending_.push_back({name,priority}); return true;
}
bool WorldStream::pump(uint32_t budget) {
  bool any=false;
  while (budget-- && !pending_.empty()) {
    auto q=std::max_element(pending_.begin(),pending_.end(),[](const auto&a,const auto&b){return a.priority<b.priority;});
    StreamRequest r=*q; pending_.erase(q);
    auto it=std::find_if(archive_.entries().begin(),archive_.entries().end(),[&](const ImgEntry&e){return e.name==r.name;});
    if(it==archive_.entries().end()) continue;
    std::vector<uint8_t> bytes; if(!archive_.read(r.name,0,it->size,bytes)) continue;
    Loaded l{r.name,std::move(bytes),{},false}; if(suffix(r.name,".col")) l.hasCollision=parseColCollision(l.bytes,l.collision); loaded_.push_back(std::move(l)); any=true;
  }
  return any;
}
bool WorldStream::loaded(const std::string& name) const { return std::any_of(loaded_.begin(),loaded_.end(),[&](const Loaded&l){return l.name==name;}); }
const std::vector<uint8_t>* WorldStream::asset(const std::string& name) const { auto i=std::find_if(loaded_.begin(),loaded_.end(),[&](const Loaded&l){return l.name==name;}); return i==loaded_.end()?nullptr:&i->bytes; }
const CollisionMesh* WorldStream::collision(const std::string& name) const { auto i=std::find_if(loaded_.begin(),loaded_.end(),[&](const Loaded&l){return l.name==name&&l.hasCollision;}); return i==loaded_.end()?nullptr:&i->collision; }

int runWorldStreamSmoke(const std::string& path) {
  WorldStream s; if(!s.open(path)) return 1;
  auto col=std::find_if(s.entries().begin(),s.entries().end(),[](const ImgEntry&e){return suffix(e.name,".col");});
  if(col==s.entries().end()) return 2;
  if(!s.request(col->name,1)||!s.request(col->name,9)||s.pending()!=1) return 3;
  if(!s.pump()||!s.loaded(col->name)||!s.asset(col->name)) return 4;
  if(!s.collision(col->name)) return 5;
  return 0;
}
}
