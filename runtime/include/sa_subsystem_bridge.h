#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace browsergamert {
enum class SaSubsystem : uint8_t { ScriptDispatch, GlobalsAbi, Injection, Renderer, Platform, Audio, Input, Streaming, Saves, Vehicles, Interiors, Count };
enum class PortState : uint8_t { Mapped, BoundaryReady, NeedsNativeParity };
struct SubsystemStatus { SaSubsystem subsystem; PortState state; std::string_view owner; std::string_view nextTask; };
class SaSubsystemBridge {
 public:
  constexpr SaSubsystemBridge() : statuses_{{
    {SaSubsystem::ScriptDispatch,PortState::BoundaryReady,"script","replace plugin::Call fallbacks on smoke route"},
    {SaSubsystem::GlobalsAbi,PortState::BoundaryReady,"state","replace StaticRef/original-address globals"},
    {SaSubsystem::Injection,PortState::BoundaryReady,"bootstrap","remove VirtualProtect/JMP entry path"},
    {SaSubsystem::Renderer,PortState::BoundaryReady,"graphics","route RenderWare/D3D9 through portable RHI"},
    {SaSubsystem::Platform,PortState::BoundaryReady,"platform","replace Win32 filesystem/window/time services"},
    {SaSubsystem::Audio,PortState::BoundaryReady,"audio","implement browser unlock/resume and streaming"},
    {SaSubsystem::Input,PortState::BoundaryReady,"input","map keyboard/mouse/gamepad/touch"},
    {SaSubsystem::Streaming,PortState::BoundaryReady,"assets","implement random-access archive VFS"},
    {SaSubsystem::Saves,PortState::BoundaryReady,"storage","mount IDBFS/OPFS and import/export saves"},
    {SaSubsystem::Vehicles,PortState::NeedsNativeParity,"gameplay","run vehicle smoke route after detachment"},
    {SaSubsystem::Interiors,PortState::NeedsNativeParity,"gameplay","run interior transition smoke route after detachment"}
  }} {}
  constexpr const auto& statuses() const { return statuses_; }
  constexpr bool allBoundariesReady() const { for (const auto& s : statuses_) if (s.state == PortState::Mapped) return false; return true; }
 private: std::array<SubsystemStatus, static_cast<size_t>(SaSubsystem::Count)> statuses_;
};
constexpr const char* portStateName(PortState s) { switch(s) { case PortState::BoundaryReady:return "boundary-ready"; case PortState::NeedsNativeParity:return "needs-native-parity"; default:return "mapped"; } }
}
