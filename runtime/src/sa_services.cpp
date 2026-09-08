#include "sa_services.h"
#include <algorithm>
#include <sstream>
namespace browsergamert {
std::string AssetVfs::normalize(std::string p){for(char&c:p)if(c=='\\')c='/';while(!p.empty()&&p.front()=='/')p.erase(p.begin());return p;}
bool AssetVfs::put(std::string p,std::vector<uint8_t>b){p=normalize(std::move(p));if(p.empty())return false;files_[p]=std::move(b);return true;}
bool AssetVfs::read(std::string p,uint64_t off,uint64_t n,std::vector<uint8_t>&out)const{auto i=files_.find(normalize(std::move(p)));if(i==files_.end()||off>i->second.size())return false;auto end=std::min<uint64_t>(off+n,i->second.size());out.assign(i->second.begin()+off,i->second.begin()+end);return true;}
bool AssetVfs::has(std::string p)const{return files_.find(normalize(std::move(p)))!=files_.end();}
void LogicalInput::key(uint32_t c,bool d){if(c==37||c==65)state_.steering=d?-1.f:0.f;if(c==39||c==68)state_.steering=d?1.f:0.f;if(c==38||c==87)state_.throttle=d?1.f:0.f;if(c==40||c==83)state_.brake=d?1.f:0.f;if(c==13)state_.enter=d;}
void LogicalInput::gamepad(float s,float t,float b){state_.steering=s;state_.throttle=t;state_.brake=b;}
void LogicalInput::touch(bool a,bool r){state_.throttle=a?1.f:0.f;state_.brake=r?1.f:0.f;}
InputState LogicalInput::state()const{return state_;}
void BrowserAudio::unlock(){unlocked_=true;}
bool BrowserAudio::queue(std::string id){if(id.empty())return false;queued_.push_back(std::move(id));return true;}
std::vector<std::string> BrowserAudio::drain(){if(!unlocked_)return {};auto r=std::move(queued_);queued_.clear();return r;}
bool PersistentSaves::write(std::string s,const SaveState&v){if(s.empty())return false;slots_[std::move(s)]=v;return true;}
bool PersistentSaves::read(std::string s,SaveState&v)const{auto i=slots_.find(s);if(i==slots_.end())return false;v=i->second;return true;}
std::string PersistentSaves::exportSlot(std::string s)const{SaveState v;if(!read(std::move(s),v))return {};std::ostringstream o;o<<v.vehicle.model<<','<<v.vehicle.position.x<<','<<v.vehicle.position.y<<','<<v.vehicle.position.z<<','<<v.interior.id<<','<<(v.interior.active?1:0);return o.str();}
bool PersistentSaves::importSlot(std::string s,std::string d){std::istringstream i(std::move(d));char c;SaveState v;int a;if(!(i>>v.vehicle.model>>c>>v.vehicle.position.x>>c>>v.vehicle.position.y>>c>>v.vehicle.position.z>>c>>v.interior.id>>c>>a))return false;v.interior.active=a!=0;return write(std::move(s),v);}
int runServiceSmoke(){AssetVfs v;if(!v.put("models/test.img",{1,2,3,4,5}))return 1;std::vector<uint8_t>r;if(!v.read("/models/test.img",1,3,r)||r.size()!=3||r[0]!=2)return 2;LogicalInput in;in.key(87,true);if(in.state().throttle!=1)return 3;BrowserAudio a;a.queue("mission");if(!a.drain().empty())return 4;a.unlock();if(a.drain().size()!=1)return 5;PersistentSaves s;SaveState x; x.vehicle.model=411;x.interior={3,true};if(!s.write("slot",x))return 6;auto d=s.exportSlot("slot");PersistentSaves t;if(!t.importSlot("slot",d))return 7;SaveState y;if(!t.read("slot",y)||y.vehicle.model!=411||!y.interior.active)return 8;return 0;}
}
