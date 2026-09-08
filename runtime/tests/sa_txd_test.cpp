#include "sa_rw_txd.h"
#include <cstdio>
int main(int argc, char** argv) {
  const char* path=argc>1?argv[1]:"C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\models\\gta3.img";
  const int r=browsergamert::runTxdMetadataSmoke(path);
  std::printf("sa_txd_metadata_smoke=%d\n",r); return r;
}
