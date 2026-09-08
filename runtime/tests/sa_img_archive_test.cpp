#include "sa_img_archive.h"
#include <cstdio>
#include <cstdlib>
int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\models\\gta3.img";
  const int result = browsergamert::runImgArchiveSmoke(path);
  std::printf("sa_img_archive_smoke=%d\n", result);
  return result;
}
