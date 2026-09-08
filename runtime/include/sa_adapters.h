#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "sa_world_stream.h"
namespace browsergamert {
struct Vec3 { float x=0,y=0,z=0; };
struct VehicleState { uint32_t model=400; Vec3 position{}; Vec3 velocity{}; float heading=0; float speed=0; bool occupied=false; };
struct PedState { uint32_t model=7; Vec3 position{}; float health=100.f; uint16_t task=0; bool active=false; };
struct InteriorState { int32_t id=0; bool active=false; };
struct InputState { float steering=0, throttle=0, brake=0; bool enter=false; float cameraYaw=0, cameraPitch=0; };
struct CameraState { Vec3 position{}; Vec3 lookAt{}; float yaw=0; float pitch=0.15f; float distance=7.f; float height=3.f; };
struct SaveState { VehicleState vehicle; InteriorState interior; };
enum class ScriptOp : uint16_t { End=0, SpawnVehicle=1, EnterVehicle=2, EnterInterior=3, SpawnPed=4, SetPedTask=5, SetPedHealth=6 };
class SaAdapters { public: bool initialize(); void tick(float dt,const InputState& input); bool dispatchScript(uint16_t opcode); bool runScript(const std::vector<uint16_t>& code); bool enterVehicle(uint32_t model,Vec3 position); bool enterInterior(int32_t id); void leaveInterior(); bool spawnPed(uint32_t model,Vec3 position); bool setPedTask(uint32_t index,uint16_t task); bool setPedHealth(uint32_t index,float health); bool save(const std::string& slot); bool load(const std::string& slot); const VehicleState& vehicle()const{return vehicle_;} const CameraState& camera()const{return camera_;} const InteriorState& interior()const{return interior_;} const std::vector<PedState>& peds()const{return peds_;} bool initialized()const{return initialized_;} WorldStream& worldStream(){return world_stream_;} private: void updateCamera(float dt,const InputState& input); bool initialized_=false; VehicleState vehicle_{}; CameraState camera_{}; InteriorState interior_{}; std::vector<PedState> peds_; std::unordered_map<std::string,SaveState> saves_; WorldStream world_stream_; };
int runSaAdapterSmoke();
}
