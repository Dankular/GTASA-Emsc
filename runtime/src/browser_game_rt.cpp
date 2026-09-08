#include "browser_game_rt.h"
#include <algorithm>
#include <sstream>
namespace browsergamert {
Capabilities probeCapabilities(){
  Capabilities c;
#if defined(__EMSCRIPTEN__)
  c.webgl2=true; c.wasmSimd=true; c.audio=true; c.gamepad=true;
#endif
  return c;
}
const char* tierName(RendererTier t){switch(t){case RendererTier::WebGPUEnhanced:return "webgpu-enhanced";case RendererTier::WebGPUCore:return "webgpu-core";default:return "legacy-webgl2";}}
void RhiDevice::loseDevice(std::string reason){state_.lost=true;state_.reason=std::move(reason);}
bool RenderGraph::add(RenderPass p,std::string* e){if(p.name.empty()){if(e)*e="render pass requires a name";return false;}for(const auto&x:passes_)if(x.name==p.name){if(e)*e="duplicate render pass: "+p.name;return false;}for(const auto&r:p.writes)if(std::find(p.reads.begin(),p.reads.end(),r)!=p.reads.end()){if(e)*e="pass reads and writes resource: "+r;return false;}passes_.push_back(std::move(p));return true;}
std::vector<std::string> RenderGraph::order()const{std::vector<std::string>r;for(const auto&p:passes_)r.push_back(p.name);return r;}
std::string RenderGraph::dump()const{std::ostringstream o;for(const auto&p:passes_)o<<p.name<<"\n";return o.str();}
void StreamScheduler::request(StreamRequest r){queue_.push_back(std::move(r));std::stable_sort(queue_.begin(),queue_.end(),[](const auto&a,const auto&b){return a.priority<b.priority;});}
bool StreamScheduler::next(StreamRequest& out){if(queue_.empty())return false;out=std::move(queue_.front());queue_.erase(queue_.begin());return true;}
}
