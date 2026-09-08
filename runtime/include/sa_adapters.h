#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace browsergamert {
struct Vec3 { float x=0,y=0,z=0; };
struct VehicleState { uint32_t model=400; Vec3 position{}; float heading=0; bool occupied=false; };
struct InteriorState { int32_t id=0; bool active=false; };
struct InputState { float steering=0, throttle=0, brake=0; bool enter=false; };
struct SaveState { VehicleState vehicle; InteriorState interior; };
class SaAdapters { public: bool initialize(); void tick(float dt,const InputState& input); bool dispatchScript(uint16_t opcode); bool enterVehicle(uint32_t model,Vec3 position); bool enterInterior(int32_t id); void leaveInterior(); bool save(const std::string& slot); bool load(const std::string& slot); const VehicleState& vehicle()const{return vehicle_;} const InteriorState& interior()const{return interior_;} bool initialized()const{return initialized_;} private: bool initialized_=false; VehicleState vehicle_{}; InteriorState interior_{}; std::unordered_map<std::string,SaveState> saves_; };
int runSaAdapterSmoke();
}
