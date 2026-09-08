#include "sa_rw_txd.h"
#include "sa_img_archive.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace browsergamert {
namespace {
uint32_t u32(const uint8_t* p) { return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24); }
uint16_t u16(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1])<<8); }
std::string fixedString(const uint8_t* p, size_t n) { size_t z=0; while (z<n && p[z]) ++z; return std::string(reinterpret_cast<const char*>(p), z); }
bool range(size_t at, size_t n, size_t total) { return at <= total && n <= total-at; }
}

bool parseTxdMetadata(const std::vector<uint8_t>& bytes, std::vector<RwTextureMetadata>& out) {
  out.clear();
  if (bytes.size() < 12 || bytes.size() > 256u*1024u*1024u || u32(bytes.data()) != 0x16) return false;
  const uint32_t rootSize=u32(bytes.data()+4);
  if (rootSize > bytes.size()-12) return false;
  // Texture dictionary (0x16) contains texture native (0x15) chunks. Walk
  // headers iteratively and parse only the first struct child of each texture.
  size_t at=12, end=12+rootSize;
  while (at+12<=end && out.size()<65536) {
    const uint32_t type=u32(bytes.data()+at), size=u32(bytes.data()+at+4);
    if (size > end-at-12) return false;
    if (type==0x15 && size>=24) {
      const size_t child=at+12;
      if (u32(bytes.data()+child)==1) {
        const uint32_t ss=u32(bytes.data()+child+4);
        const size_t p=child+12;
        if (ss>=88 && ss<=size-12 && range(p,ss,bytes.size())) {
          RwTextureMetadata t;
          // D3D RenderWare texture struct: platform, filter/addressing,
          // 32-byte name, 32-byte mask, raster format and raster info.
          if (ss < 88) return false;
          t.name=fixedString(bytes.data()+p+8,32);
          t.mask=fixedString(bytes.data()+p+40,32);
          t.rasterFormat=u32(bytes.data()+p+72);
          t.width=u16(bytes.data()+p+76); t.height=u16(bytes.data()+p+78);
          t.depth=bytes[p+80]; t.mipLevels=bytes[p+81]; t.rasterType=bytes[p+82];
          t.compression=bytes[p+83];
          t.hasAlpha=(t.rasterFormat & 0x1000u)!=0 || t.compression==0x0Fu;
          t.payloadOffset=p+ss; // precise pixel location is decoder-specific
          t.payloadSize=0;
          out.push_back(std::move(t));
        }
      }
    }
    at += 12 + size;
  }
  return !out.empty();
}

RwTextureUploadPlan makeTextureUploadPlan(const RwTextureMetadata& t) {
  RwTextureUploadPlan p; p.width=t.width; p.height=t.height; p.mipLevels=t.mipLevels?t.mipLevels:1; p.hasAlpha=t.hasAlpha;
  p.compressed = t.compression!=0;
  if (p.compressed) { p.format="renderware-compressed"; p.bytesPerRow=0; return p; }
  // Common SA 32-bit raster; other formats are handed to the browser decoder.
  if ((t.rasterFormat & 0x0F00u)==0x0500u || t.depth==32) { p.format=t.hasAlpha?"rgba8unorm":"rgba8unorm"; p.bytesPerRow=t.width*4; }
  else if (t.depth==16) { p.format=t.hasAlpha?"rgba8unorm":"rgba8unorm"; p.bytesPerRow=t.width*2; }
  else { p.format="renderware-raster"; p.bytesPerRow=0; }
  return p;
}

int runTxdMetadataSmoke(const std::string& imgPath) {
  ImgArchive archive; if (!archive.open(imgPath)) return 1;
  size_t candidates=0;
  for (const auto& e: archive.entries()) {
    if (e.name.size()<4 || e.name.substr(e.name.size()-4)!=".txd") continue;
    ++candidates; if (e.size==0 || e.size>256u*1024u*1024u) continue;
    std::vector<uint8_t> bytes; if (!archive.read(e.name,0,e.size,bytes)) continue;
    std::vector<RwTextureMetadata> textures;
    if (parseTxdMetadata(bytes,textures) && !textures.empty() && textures.front().width && textures.front().height) return 0;
  }
  return candidates ? 3 : 2;
}
}
