#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace browsergamert {
enum class RendererTier : uint8_t { LegacyWebGL2, WebGPUCore, WebGPUEnhanced };
struct Capabilities { bool webgpu=false, webgl2=false, wasmSimd=false, threads=false, sharedArrayBuffer=false, idbfs=true, opfs=false, gamepad=false, audio=false; RendererTier tier=RendererTier::LegacyWebGL2; std::string adapter; std::vector<std::string> optionalFeatures, missingRequired; };
struct DeviceState { bool lost=false; std::string reason; };
struct BufferDesc { uint64_t size=0; uint32_t usage=0; };
struct TextureDesc { uint32_t width=1,height=1,mipLevels=1; };
class RhiDevice { public: explicit RhiDevice(Capabilities c):caps_(std::move(c)){} const Capabilities& capabilities()const{return caps_;} const DeviceState& state()const{return state_;} void loseDevice(std::string reason); bool canSubmit()const{return !state_.lost;} private: Capabilities caps_; DeviceState state_; };
struct RenderPass { std::string name; std::vector<std::string> reads,writes; };
class RenderGraph { public: bool add(RenderPass pass,std::string* error=nullptr); std::vector<std::string> order()const; std::string dump()const; private: std::vector<RenderPass> passes_; };
struct StreamRequest { std::string id; uint8_t priority=3; uint64_t offset=0,size=0; };
class StreamScheduler { public: void request(StreamRequest request); bool next(StreamRequest& out); size_t pending()const{return queue_.size();} private: std::vector<StreamRequest> queue_; };
Capabilities probeCapabilities();
const char* tierName(RendererTier tier);
}
