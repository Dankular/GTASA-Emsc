#include "sa_adapters.h"
#include <cmath>
namespace browsergamert {
bool SaAdapters::initialize(){initialized_=true;vehicle_={};interior_={};saves_.clear();return true;}
void SaAdapters::tick(float dt,const InputState&i){if(!initialized_||dt<=0)return;vehicle_.heading+=i.steering*dt*90.f;const float r=vehicle_.heading*.0174532925f;vehicle_.position.x+=(i.throttle-i.brake)*std::cos(r)*dt*12.f;vehicle_.position.y+=(i.throttle-i.brake)*std::sin(r)*dt*12.f;}
bool SaAdapters::dispatchScript(uint16_t op){return initialized_&&op!=0;}
bool SaAdapters::enterVehicle(uint32_t m,Vec3 p){if(!initialized_||m==0)return false;vehicle_.model=m;vehicle_.position=p;vehicle_.occupied=true;return true;}
bool SaAdapters::enterInterior(int32_t id){if(!initialized_||id<0)return false;interior_.id=id;interior_.active=true;return true;}
void SaAdapters::leaveInterior(){interior_.active=false;interior_.id=0;}
bool SaAdapters::save(const std::string&s){if(!initialized_||s.empty())return false;saves_[s]={vehicle_,interior_};return true;}
bool SaAdapters::load(const std::string&s){auto i=saves_.find(s);if(!initialized_||i==saves_.end())return false;vehicle_=i->second.vehicle;interior_=i->second.interior;return true;}
int runSaAdapterSmoke(){SaAdapters a;if(!a.initialize())return 1;if(!a.dispatchScript(1))return 2;if(!a.enterVehicle(411,{0,0,0}))return 3;a.tick(1,{0,1,0,false});if(a.vehicle().position.x<=0)return 4;if(!a.enterInterior(3))return 5;if(!a.save("smoke"))return 6;a.leaveInterior();if(!a.load("smoke")||!a.interior().active)return 7;return 0;}
}
