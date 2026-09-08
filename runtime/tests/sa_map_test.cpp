#include "sa_map.h"
#include "sa_world_stream.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
int main() {
  std::vector<browsergamert::IdeDefinition> defs; std::vector<browsergamert::IplInstance> inst;
  assert(browsergamert::parseIdeText("objs\n100, tree, trees, 120.0, 1\nend\n",defs)); assert(defs.size()==1&&defs[0].id==100);
  assert(browsergamert::parseIplText("inst\n100, tree, 0, 10, 20, 30, 0, 0, 0, 1, 0\nend\n",inst)); assert(inst.size()==1&&inst[0].transform.x==10&&inst[0].transform.z==30);
  std::vector<uint8_t> col(28+16+36+8,0); std::memcpy(col.data(),"COLL",4); auto put=[&](size_t o,uint32_t v){std::memcpy(col.data()+o,&v,4);}; auto putf=[&](size_t o,float v){std::memcpy(col.data()+o,&v,4);};
  put(28,0); put(32,0); put(36,3); put(40,1); float p[9]={0,0,0,10,0,0,0,10,0}; for(int i=0;i<9;++i)putf(44+i*4,p[i]); uint16_t ids[4]={0,1,2,0}; std::memcpy(col.data()+80,ids,8);
  browsergamert::CollisionMesh mesh; assert(browsergamert::parseColCollision(col,mesh)); assert(mesh.faces.size()==1&&mesh.points.size()==3);
  std::puts("sa_map_smoke=0"); return 0;
}
