#include "sa_adapters.h"
#include <cmath>
namespace browsergamert {
bool SaAdapters::initialize(){initialized_=true;vehicle_={};camera_={};interior_={};peds_.clear();saves_.clear();updateCamera(1.f,{});return true;}
void SaAdapters::tick(float dt,const InputState&i){
 if(!initialized_||dt<=0)return;
 dt=std::min(dt,0.1f);
 // Owned, deterministic rear-wheel-style arcade dynamics. Inputs are normalized.
 const float throttle=std::max(-1.f,std::min(1.f,i.throttle));
 const float brake=std::max(0.f,std::min(1.f,i.brake));
 const float acceleration=18.f*throttle-28.f*brake;
 vehicle_.speed += acceleration*dt;
 const float drag=std::pow(0.18f,dt);
 vehicle_.speed*=drag;
 vehicle_.speed=std::max(-12.f,std::min(42.f,vehicle_.speed));
 const float steer=std::max(-1.f,std::min(1.f,i.steering));
 const float steerRate=(2.2f+std::min(std::abs(vehicle_.speed)/12.f,1.f)*1.8f)*steer;
 vehicle_.heading+=steerRate*dt*(vehicle_.speed>=0.f?1.f:-1.f);
 const float r=vehicle_.heading;
 vehicle_.velocity={std::cos(r)*vehicle_.speed,std::sin(r)*vehicle_.speed,0};
 vehicle_.position.x+=vehicle_.velocity.x*dt;
 vehicle_.position.y+=vehicle_.velocity.y*dt;
 updateCamera(dt,i);
 for(auto& p:peds_) if(p.active&&p.task==1) p.position.x+=dt*1.5f;
}
void SaAdapters::updateCamera(float dt,const InputState&i){
 const float blend=1.f-std::exp(-std::max(0.01f,dt)*8.f);
 camera_.yaw += i.cameraYaw*dt*1.8f;
 camera_.pitch=std::max(-0.35f,std::min(0.75f,camera_.pitch+i.cameraPitch*dt));
 const float yaw=vehicle_.heading+3.14159265f+camera_.yaw;
 const float cp=std::cos(camera_.pitch);
 const Vec3 desired{vehicle_.position.x-std::cos(yaw)*camera_.distance*cp,vehicle_.position.y-std::sin(yaw)*camera_.distance*cp,vehicle_.position.z+camera_.height+std::sin(camera_.pitch)*camera_.distance};
 camera_.position.x+=(desired.x-camera_.position.x)*blend; camera_.position.y+=(desired.y-camera_.position.y)*blend; camera_.position.z+=(desired.z-camera_.position.z)*blend;
 const Vec3 desiredLook{vehicle_.position.x,vehicle_.position.y,vehicle_.position.z+1.1f};
 camera_.lookAt.x+=(desiredLook.x-camera_.lookAt.x)*blend; camera_.lookAt.y+=(desiredLook.y-camera_.lookAt.y)*blend; camera_.lookAt.z+=(desiredLook.z-camera_.lookAt.z)*blend;
}
bool SaAdapters::dispatchScript(uint16_t op){return initialized_&&op!=static_cast<uint16_t>(ScriptOp::End)&&op<=static_cast<uint16_t>(ScriptOp::SetPedHealth);}
bool SaAdapters::runScript(const std::vector<uint16_t>& code){if(!initialized_||code.size()>256)return false;size_t pc=0;bool ended=false;while(pc<code.size()){auto op=static_cast<ScriptOp>(code[pc++]);if(op==ScriptOp::End){ended=true;break;}auto arg=[&](uint16_t&out){if(pc>=code.size())return false;out=code[pc++];return true;};uint16_t a=0;switch(op){case ScriptOp::SpawnVehicle:if(!arg(a)||!enterVehicle(a,{0,0,0}))return false;break;case ScriptOp::EnterVehicle:if(!enterVehicle(vehicle_.model,vehicle_.position))return false;break;case ScriptOp::EnterInterior:if(!arg(a)||!enterInterior(a))return false;break;case ScriptOp::SpawnPed:if(!arg(a)||!spawnPed(a,{static_cast<float>(peds_.size()),0,0}))return false;break;case ScriptOp::SetPedTask:if(!arg(a)||peds_.empty()||!setPedTask(static_cast<uint32_t>(peds_.size()-1),a))return false;break;case ScriptOp::SetPedHealth:if(!arg(a)||peds_.empty()||!setPedHealth(static_cast<uint32_t>(peds_.size()-1),static_cast<float>(a)))return false;break;default:return false;}}return ended;}
bool SaAdapters::enterVehicle(uint32_t m,Vec3 p){if(!initialized_||m==0)return false;vehicle_.model=m;vehicle_.position=p;vehicle_.occupied=true;return true;}
bool SaAdapters::enterInterior(int32_t id){if(!initialized_||id<0)return false;interior_.id=id;interior_.active=true;return true;}
void SaAdapters::leaveInterior(){interior_.active=false;interior_.id=0;}
bool SaAdapters::spawnPed(uint32_t model,Vec3 p){if(!initialized_||model==0||peds_.size()>=64)return false;peds_.push_back({model,p,100.f,0,true});return true;}
bool SaAdapters::setPedTask(uint32_t index,uint16_t task){if(!initialized_||index>=peds_.size()||!peds_[index].active)return false;peds_[index].task=task;return true;}
bool SaAdapters::setPedHealth(uint32_t index,float health){if(!initialized_||index>=peds_.size()||health<0)return false;peds_[index].health=health;return true;}
bool SaAdapters::save(const std::string&s){if(!initialized_||s.empty())return false;saves_[s]={vehicle_,interior_};return true;}
bool SaAdapters::load(const std::string&s){auto i=saves_.find(s);if(!initialized_||i==saves_.end())return false;vehicle_=i->second.vehicle;interior_=i->second.interior;return true;}
int runSaAdapterSmoke(){SaAdapters a;if(!a.initialize())return 1;if(!a.dispatchScript(1))return 2;std::vector<uint16_t> mission{1,411,4,7,5,1,6,80,3,3,0};if(!a.runScript(mission))return 3;a.tick(1,{0,1,0,false});if(a.vehicle().position.x<=0||a.vehicle().speed<=0)return 4;if(a.camera().position.x==0&&a.camera().position.y==0)return 5;if(a.peds().size()!=1||a.peds()[0].health!=80||a.peds()[0].position.x<=0)return 6;if(!a.save("smoke"))return 7;a.leaveInterior();if(!a.load("smoke")||!a.interior().active)return 8;return 0;}
}
