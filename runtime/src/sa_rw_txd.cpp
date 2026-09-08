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
          // SA's D3D native raster layout is: rasterFormat (72), fourcc (76),
          // width/height (80/82), depth/mips/type/compression (84..87),
          // followed immediately by the first mip's bytes.
          t.rasterFormat=u32(bytes.data()+p+72);
          t.width=u16(bytes.data()+p+80); t.height=u16(bytes.data()+p+82);
          t.depth=bytes[p+84]; t.mipLevels=bytes[p+85]; t.rasterType=bytes[p+86];
          t.compression=bytes[p+87];
          t.hasAlpha=(t.rasterFormat & 0x1000u)!=0 || t.compression==9 || t.compression==10;
          t.payloadOffset=p+88;
          t.payloadSize=ss-88;
          out.push_back(std::move(t));
        }
      }
    }
    at += 12 + size;
  }
  return !out.empty();
}

namespace {
uint16_t rgb565(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
void put565(uint16_t c, uint8_t* out) {
  out[0]=uint8_t(((c>>11)&31)*255/31); out[1]=uint8_t(((c>>5)&63)*255/63);
  out[2]=uint8_t((c&31)*255/31);
}
void putPixel(RwTexturePixels& out, uint32_t x, uint32_t y, const uint8_t* c) {
  if (x>=out.width || y>=out.height) return;
  std::memcpy(out.rgba.data() + (size_t(y)*out.width+x)*4, c, 4);
}
bool decodeBlockColor(const uint8_t* b, bool dxt1, RwTexturePixels& out, uint32_t bx, uint32_t by,
                      const uint8_t* alpha) {
  const uint16_t c0=rgb565(b), c1=rgb565(b+2); uint8_t colors[4][4]{};
  put565(c0,colors[0]); put565(c1,colors[1]); colors[0][3]=colors[1][3]=255;
  if (c0>c1 || !dxt1) {
    for(int k=0;k<3;k++) colors[2][k]=uint8_t((2*colors[0][k]+colors[1][k])/3);
    for(int k=0;k<3;k++) colors[3][k]=uint8_t((colors[0][k]+2*colors[1][k])/3);
    colors[2][3]=colors[3][3]=255;
  } else {
    for(int k=0;k<3;k++) colors[2][k]=uint8_t((colors[0][k]+colors[1][k])/2);
    colors[2][3]=255; colors[3][3]=0;
  }
  const uint32_t selectors=u32(b+4);
  for(uint32_t py=0;py<4;py++) for(uint32_t px=0;px<4;px++) {
    const uint32_t s=(selectors >> (2*(py*4+px)))&3; uint8_t c[4]; std::memcpy(c,colors[s],4);
    if(alpha) c[3]=alpha[py*4+px]; putPixel(out,bx*4+px,by*4+py,c);
  }
  return true;
}
}

bool decodeTxdTexturePixels(const std::vector<uint8_t>& bytes, const RwTextureMetadata& t, RwTexturePixels& out) {
  out={}; if (!t.width || !t.height || t.width>8192 || t.height>8192 ||
      t.payloadOffset>bytes.size() || t.payloadSize>bytes.size()-t.payloadOffset) return false;
  out.width=t.width; out.height=t.height; out.rgba.resize(size_t(t.width)*t.height*4);
  const uint8_t* src=bytes.data()+t.payloadOffset;
  // SA commonly stores DXT1/3/5 in the D3D native texture. Compression IDs
  // observed in the game are 1, 9 and 10 respectively.
  if (t.compression==1 || t.compression==9 || t.compression==10) {
    const size_t blockBytes=t.compression==1?8:16;
    const uint32_t bw=(t.width+3)/4,bh=(t.height+3)/4;
    if (t.payloadSize<size_t(bw)*bh*blockBytes) return false;
    for(uint32_t by=0;by<bh;by++) for(uint32_t bx=0;bx<bw;bx++) {
      const uint8_t* b=src+(size_t(by)*bw+bx)*blockBytes; uint8_t alpha[16]{}; const uint8_t* ap=nullptr;
      if(t.compression==9) { for(int i=0;i<8;i++) { const uint8_t v=b[i]; alpha[2*i]=uint8_t((v&15)*17); alpha[2*i+1]=uint8_t((v>>4)*17); } ap=alpha; b+=8; }
      else if(t.compression==10) { const uint8_t a0=b[0],a1=b[1]; uint64_t bits=0; for(int i=0;i<6;i++) bits|=uint64_t(b[2+i])<<(8*i); uint8_t vals[8]={a0,a1}; if(a0>a1){for(int i=2;i<8;i++)vals[i]=uint8_t(((8-i)*a0+(i-1)*a1)/7);}else{for(int i=2;i<8;i++) vals[i]=uint8_t(((6-i)*a0+(i-1)*a1)/5);vals[6]=0;vals[7]=255;} for(int i=0;i<16;i++)alpha[i]=vals[(bits>>(3*i))&7]; ap=alpha; b+=8; }
      decodeBlockColor(b,t.compression==1,out,bx,by,ap);
    }
    return true;
  }
  if (t.compression==0 && t.depth==32) {
    const size_t n=size_t(t.width)*t.height*4; if(t.payloadSize<n)return false;
    for(size_t i=0;i<n;i+=4){out.rgba[i]=src[i+2];out.rgba[i+1]=src[i+1];out.rgba[i+2]=src[i];out.rgba[i+3]=src[i+3];} return true;
  }
  return false;
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

int runTxdPixelSmoke(const std::string& imgPath) {
  ImgArchive archive; if (!archive.open(imgPath)) return 1;
  for (const auto& e : archive.entries()) {
    if (e.name.size()<4 || e.name.substr(e.name.size()-4)!=".txd" || e.size==0 || e.size>256u*1024u*1024u) continue;
    std::vector<uint8_t> bytes; if (!archive.read(e.name,0,e.size,bytes)) continue;
    std::vector<RwTextureMetadata> textures; if(!parseTxdMetadata(bytes,textures))continue;
    for(const auto& t:textures){RwTexturePixels px; if(decodeTxdTexturePixels(bytes,t,px)&&!px.rgba.empty())return 0;}
  }
  return 2;
}
}
