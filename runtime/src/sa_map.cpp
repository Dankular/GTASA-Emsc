#include "sa_map.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace browsergamert {
namespace {
std::vector<std::string> fields(std::string line) {
  const auto comment=line.find('#'); if(comment!=std::string::npos) line.resize(comment);
  std::vector<std::string> out; std::string cur;
  for(char c:line) { if(c==',') { while(!cur.empty()&&std::isspace((unsigned char)cur.back()))cur.pop_back(); size_t p=0; while(p<cur.size()&&std::isspace((unsigned char)cur[p]))++p; if(p<cur.size())out.push_back(cur.substr(p)); else out.emplace_back(); cur.clear(); } else cur.push_back(c); }
  while(!cur.empty()&&std::isspace((unsigned char)cur.back()))cur.pop_back(); size_t p=0; while(p<cur.size()&&std::isspace((unsigned char)cur[p]))++p; if(p<cur.size())out.push_back(cur.substr(p));
  return out;
}
bool number(const std::string& s, float& out) { char* e=nullptr; out=std::strtof(s.c_str(),&e); return e&&*e=='\0'; }
bool integer(const std::string& s, int32_t& out) { char* e=nullptr; long v=std::strtol(s.c_str(),&e,10); if(!e||*e) return false; out=(int32_t)v; return true; }
}
bool parseIdeText(const std::string& text, std::vector<IdeDefinition>& out) {
  out.clear(); bool active=false; size_t begin=0;
  while(begin<=text.size()) { size_t end=text.find('\n',begin); if(end==std::string::npos)end=text.size(); auto f=fields(text.substr(begin,end-begin)); begin=end+1; if(f.empty())continue; std::string section=f[0]; std::transform(section.begin(),section.end(),section.begin(),[](char c){return (char)std::tolower((unsigned char)c);}); if(section=="objs"||section=="tobj") {active=true;continue;} if(section=="end") {active=false;continue;} if(!active||f.size()<3)continue; IdeDefinition d; if(!integer(f[0],d.id))continue; d.model=f[1]; d.texture=f[2]; if(f.size()>3)number(f[3],d.drawDistance); if(f.size()>4) { int32_t flags=0; if(integer(f[4],flags)) d.flags=(uint32_t)flags; } out.push_back(std::move(d)); }
  return !out.empty();
}
bool parseIplText(const std::string& text, std::vector<IplInstance>& out) {
  out.clear(); bool active=false; size_t begin=0;
  while(begin<=text.size()) { size_t end=text.find('\n',begin); if(end==std::string::npos)end=text.size(); auto f=fields(text.substr(begin,end-begin)); begin=end+1; if(f.empty())continue; std::string section=f[0]; std::transform(section.begin(),section.end(),section.begin(),[](char c){return (char)std::tolower((unsigned char)c);}); if(section=="inst") {active=true;continue;} if(section=="end") {active=false;continue;} if(!active||f.size()<10)continue;
    IplInstance i; if(!integer(f[0],i.id))continue; i.model=f[1]; if(!integer(f[2],i.interior))continue; float* v=&i.transform.x; bool ok=true; for(size_t n=0;n<7 && n+3<f.size();++n)ok=number(f[3+n],v[n])&&ok; if(!ok)continue; if(f.size()>10)integer(f[10],i.lod); out.push_back(std::move(i));
  }
  return !out.empty();
}
}
