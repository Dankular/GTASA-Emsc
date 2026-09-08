#include "sa_world_stream.h"
#include <algorithm>
#include <cmath>
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
        out.vertices=spheres; out.triangles=0; out.points.reserve(spheres);
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
          out.points.push_back({x,y,z});
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
    out.vertices=verts; out.triangles=faces; out.points.reserve(verts); out.faces.reserve(faces);
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
        out.points.push_back({x,y,z});
        out.bounds.minX=std::min(out.bounds.minX,x); out.bounds.minY=std::min(out.bounds.minY,y); out.bounds.minZ=std::min(out.bounds.minZ,z);
        out.bounds.maxX=std::max(out.bounds.maxX,x); out.bounds.maxY=std::max(out.bounds.maxY,y); out.bounds.maxZ=std::max(out.bounds.maxZ,z);
      }
    }
    const size_t faceAt=vertexAt+size_t(verts)*12;
    for(uint32_t i=0;i<faces;++i) {
      const uint8_t* p=b.data()+faceAt+size_t(i)*8;
      uint32_t a=uint32_t(p[0])|(uint32_t(p[1])<<8), c=uint32_t(p[2])|(uint32_t(p[3])<<8), d=uint32_t(p[4])|(uint32_t(p[5])<<8);
      if(a>=verts||c>=verts||d>=verts) { out.faces.clear(); break; }
      out.faces.push_back({a,c,d});
    }
    return true;
  }
  return false;
}

bool WorldStream::open(const std::string& path) { pending_.clear(); loaded_.clear(); definitions_.clear(); instances_.clear(); return archive_.open(path); }
bool WorldStream::loadIde(const std::string& path) { std::ifstream f(path,std::ios::binary); if(!f)return false; std::string text((std::istreambuf_iterator<char>(f)),{}); return parseIdeText(text,definitions_); }
bool WorldStream::loadIpl(const std::string& path) { std::ifstream f(path,std::ios::binary); if(!f)return false; std::string text((std::istreambuf_iterator<char>(f)),{}); return parseIplText(text,instances_); }
bool WorldStream::request(const std::string& name, int priority) {
  return requestAt(name, priority, 0, 0);
}
bool WorldStream::requestAt(const std::string& name, int priority, int32_t sectorX, int32_t sectorY) {
  if (!archive_.entries().size() || loaded(name)) return false;
  auto it=std::find_if(archive_.entries().begin(),archive_.entries().end(),[&](const ImgEntry&e){return e.name==name;});
  if (it==archive_.entries().end()) return false;
  auto q=std::find_if(pending_.begin(),pending_.end(),[&](const WorldStreamRequest&r){return r.name==name;});
  if(q!=pending_.end()){q->priority=std::max(q->priority,priority);q->sectorX=sectorX;q->sectorY=sectorY;return true;}
  pending_.push_back({name,priority,sectorX,sectorY}); return true;
}
bool WorldStream::requestSector(int32_t sectorX, int32_t sectorY, int priority) {
  // IMG directories do not contain placement metadata. Until IPL/IDE world
  // placement is attached, a sector request conservatively schedules the
  // collision set and tags each request with the requested sector.
  bool any=false;
  for (const auto& e : archive_.entries()) {
    if (!suffix(e.name, ".col")) continue;
    any = requestAt(e.name, priority, sectorX, sectorY) || any;
  }
  return any;
}
bool WorldStream::pump(uint32_t budget) {
  bool any=false;
  while (budget-- && !pending_.empty()) {
    auto q=std::max_element(pending_.begin(),pending_.end(),[](const auto&a,const auto&b){
      return a.priority<b.priority || (a.priority==b.priority && a.name>b.name);
    });
    WorldStreamRequest r=*q; pending_.erase(q);
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

namespace {
bool overlaps(const CollisionBounds& a, const CollisionBounds& b) {
  return a.minX<=b.maxX && a.maxX>=b.minX && a.minY<=b.maxY && a.maxY>=b.minY &&
         a.minZ<=b.maxZ && a.maxZ>=b.minZ;
}
}
size_t WorldStream::queryAabb(const CollisionBounds& area, std::vector<CollisionHit>& out) const {
  out.clear();
  for (const auto& l : loaded_) if (l.hasCollision && overlaps(area, l.collision.bounds))
    out.push_back({l.name, 0.0f, l.collision.bounds});
  return out.size();
}
bool WorldStream::raycast(const CollisionVec3& o, const CollisionVec3& d, float maxDistance, CollisionHit& out) const {
  bool found=false; float best=maxDistance;
  for (const auto& l : loaded_) if (l.hasCollision) {
    float tmin=0.0f, tmax=maxDistance;
    const float origin[3]={o.x,o.y,o.z}, dir[3]={d.x,d.y,d.z};
    const float mn[3]={l.collision.bounds.minX,l.collision.bounds.minY,l.collision.bounds.minZ};
    const float mx[3]={l.collision.bounds.maxX,l.collision.bounds.maxY,l.collision.bounds.maxZ};
    bool hit=true;
    for (int axis=0; axis<3; ++axis) {
      if (std::abs(dir[axis]) < 1.0e-8f) { if (origin[axis]<mn[axis] || origin[axis]>mx[axis]) { hit=false; break; } }
      else { float a=(mn[axis]-origin[axis])/dir[axis], b=(mx[axis]-origin[axis])/dir[axis]; if(a>b) std::swap(a,b); tmin=std::max(tmin,a); tmax=std::min(tmax,b); if(tmin>tmax) { hit=false; break; } }
    }
    if (hit && tmin<best) {
      float exact=tmin;
      if (!l.collision.faces.empty()) {
        exact=std::numeric_limits<float>::infinity();
        for(const auto& f:l.collision.faces) {
          if(f.a>=l.collision.points.size()||f.b>=l.collision.points.size()||f.c>=l.collision.points.size())continue;
          const auto&a=l.collision.points[f.a]; const auto&b=l.collision.points[f.b]; const auto&c=l.collision.points[f.c];
          const CollisionVec3 e1{b.x-a.x,b.y-a.y,b.z-a.z},e2{c.x-a.x,c.y-a.y,c.z-a.z};
          const CollisionVec3 p{d.y*e2.z-d.z*e2.y,d.z*e2.x-d.x*e2.z,d.x*e2.y-d.y*e2.x};
          const float det=e1.x*p.x+e1.y*p.y+e1.z*p.z; if(std::abs(det)<1e-7f)continue; const float inv=1.0f/det;
          const CollisionVec3 t{o.x-a.x,o.y-a.y,o.z-a.z}; const float u=(t.x*p.x+t.y*p.y+t.z*p.z)*inv; if(u<0||u>1)continue;
          const CollisionVec3 q{t.y*e1.z-t.z*e1.y,t.z*e1.x-t.x*e1.z,t.x*e1.y-t.y*e1.x}; const float v=(d.x*q.x+d.y*q.y+d.z*q.z)*inv; if(v<0||u+v>1)continue;
          const float distance=(e2.x*q.x+e2.y*q.y+e2.z*q.z)*inv; if(distance>=0&&distance<exact)exact=distance;
        }
        if(!std::isfinite(exact)||exact>=best)continue;
      }
      best=exact; out={l.name,exact,l.collision.bounds}; found=true;
    }
  }
  return found;
}
std::pair<int32_t,int32_t> WorldStream::sectorFor(float x, float y, float size) {
  if (!(size>0.0f)) size=300.0f;
  return {static_cast<int32_t>(std::floor(x/size)), static_cast<int32_t>(std::floor(y/size))};
}

int runWorldStreamSmoke(const std::string& path) {
  WorldStream s; if(!s.open(path)) return 1;
  auto col=std::find_if(s.entries().begin(),s.entries().end(),[](const ImgEntry&e){return suffix(e.name,".col");});
  if(col==s.entries().end()) return 2;
  if(!s.requestSector(0,0,1)||!s.request(col->name,1)||!s.request(col->name,9)||s.pending()==0) return 3;
  if(!s.pump()||!s.loaded(col->name)||!s.asset(col->name)) return 4;
  const auto* mesh=s.collision(col->name); if(!mesh) return 5;
  std::vector<CollisionHit> hits;
  if(s.queryAabb(mesh->bounds,hits)==0) return 6;
  auto sector=WorldStream::sectorFor(mesh->bounds.minX,mesh->bounds.minY);
  if(sector.first != static_cast<int32_t>(std::floor(mesh->bounds.minX/300.0f))) return 7;
  CollisionVec3 origin{mesh->bounds.minX-1.0f,(mesh->bounds.minY+mesh->bounds.maxY)*0.5f,(mesh->bounds.minZ+mesh->bounds.maxZ)*0.5f};
  CollisionHit hit{};
  if(!s.raycast(origin,{1,0,0},100000000.0f,hit)) return 8;
  return 0;
}
}
