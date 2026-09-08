#include "sa_world_stream.h"
#include <cstdio>
int main(int argc,char**argv){
  const char* path=argc>1?argv[1]:"C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\models\\gta3.img";
  int r=browsergamert::runWorldStreamSmoke(path);
  std::printf("sa_world_stream_smoke=%d\n",r); return r;
}
