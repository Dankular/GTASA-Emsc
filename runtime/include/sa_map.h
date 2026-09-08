#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace browsergamert {
struct SaTransform { float x=0,y=0,z=0; float rx=0,ry=0,rz=0,rw=1; };
struct IdeDefinition { int32_t id=0; std::string model, texture; float drawDistance=0; uint32_t flags=0; };
struct IplInstance { int32_t id=0; std::string model; int32_t interior=0; SaTransform transform{}; int32_t lod=0; };
bool parseIdeText(const std::string& text, std::vector<IdeDefinition>& out);
bool parseIplText(const std::string& text, std::vector<IplInstance>& out);
}
