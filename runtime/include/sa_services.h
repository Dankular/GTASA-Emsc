#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "sa_adapters.h"
namespace browsergamert {
class AssetVfs { public: bool put(std::string path,std::vector<uint8_t> bytes); bool read(std::string path,uint64_t offset,uint64_t size,std::vector<uint8_t>& out) const; bool has(std::string path) const; private: std::unordered_map<std::string,std::vector<uint8_t>> files_; static std::string normalize(std::string path); };
class LogicalInput { public: void key(uint32_t code,bool down); void gamepad(float steering,float throttle,float brake); void touch(bool accelerate,bool reverse); InputState state() const; private: InputState state_{}; };
class BrowserAudio { public: void unlock(); bool unlocked() const{return unlocked_;} bool queue(std::string id); std::vector<std::string> drain(); private: bool unlocked_=false; std::vector<std::string> queued_; };
class PersistentSaves { public: bool write(std::string slot,const SaveState& state); bool read(std::string slot,SaveState& state) const; std::string exportSlot(std::string slot) const; bool importSlot(std::string slot,std::string data); private: std::unordered_map<std::string,SaveState> slots_; };
int runServiceSmoke();
}
