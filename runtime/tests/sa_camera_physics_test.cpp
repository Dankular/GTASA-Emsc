#include "sa_adapters.h"
#include <cmath>
#include <cstdio>
using namespace browsergamert;
int main(){
  SaAdapters a; if(!a.initialize()||!a.enterVehicle(411,{})) return 1;
  a.tick(0.1f,{0,1,0,false,0,0});
  const float moving=a.vehicle().speed;
  if(moving<=0||a.vehicle().position.x<=0) return 2;
  const auto before=a.camera();
  a.tick(0.1f,{0,1,0,false,1,0});
  const auto after=a.camera();
  if(std::abs(after.position.x-before.position.x)<0.001f&&std::abs(after.position.y-before.position.y)<0.001f) return 3;
  a.tick(0.1f,{0,0,1,false,0,0});
  if(a.vehicle().speed>=moving) return 4;
  if(std::abs(after.lookAt.x-a.vehicle().position.x)>2.f) return 5;
  std::printf("camera_physics=0 speed=%.3f camera=(%.3f,%.3f,%.3f)\n",a.vehicle().speed,a.camera().position.x,a.camera().position.y,a.camera().position.z);
  return 0;
}
