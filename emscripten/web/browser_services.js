export class BrowserInput {
  constructor(target = window) { this.state={steering:0,throttle:0,brake:0,enter:false}; this.keys=new Set(); target.addEventListener('keydown',e=>this.key(e,true)); target.addEventListener('keyup',e=>this.key(e,false)); this.target=target; }
  key(e,down){ const c=e.key.toLowerCase(); if(['arrowleft','a'].includes(c))this.state.steering=down?-1:0; if(['arrowright','d'].includes(c))this.state.steering=down?1:0; if(['arrowup','w'].includes(c))this.state.throttle=down?1:0; if(['arrowdown','s'].includes(c))this.state.brake=down?1:0; if(c==='enter')this.state.enter=down; }
  pollGamepad(){ const p=navigator.getGamepads?.().find(Boolean); if(p){this.state.steering=p.axes?.[0]||0;this.state.throttle=Math.max(0,p.buttons?.[7]?.value||0);this.state.brake=Math.max(0,p.buttons?.[6]?.value||0);} return {...this.state}; }
  touch(accelerate,reverse){this.state.throttle=accelerate?1:0;this.state.brake=reverse?1:0;return {...this.state};}
}
export class BrowserAudioGate {
  constructor(){this.context=null;this.unlocked=false;}
  async unlock(){this.context ||= new AudioContext(); await this.context.resume(); this.unlocked=this.context.state==='running'; return this.unlocked;}
  async pause(){if(this.context)await this.context.suspend();}
  async resume(){if(this.context)await this.context.resume();}
}
export class SaveManager {
  constructor(name='gtasa-emsc',store='saves'){this.name=name;this.store=store;this.db=null;}
  async open(){this.db ||= await new Promise((resolve,reject)=>{const r=indexedDB.open(this.name,1);r.onupgradeneeded=()=>r.result.createObjectStore(this.store);r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});return this.db;}
  async put(slot,data){const db=await this.open();return new Promise((resolve,reject)=>{const r=db.transaction(this.store,'readwrite').objectStore(this.store).put(data,slot);r.onsuccess=()=>resolve();r.onerror=()=>reject(r.error);});}
  async get(slot){const db=await this.open();return new Promise((resolve,reject)=>{const r=db.transaction(this.store).objectStore(this.store).get(slot);r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});}
  async export(slot){const data=await this.get(slot);if(data===undefined)throw new Error('save not found');return new Blob([JSON.stringify(data)],{type:'application/json'});}
  async import(slot,file){await this.put(slot,JSON.parse(await file.text()));}
}
export class RangeVfs {
  constructor(fetcher=fetch){this.fetcher=fetcher;}
  async read(url,offset,size){
    if(!Number.isSafeInteger(offset)||offset<0||!Number.isSafeInteger(size)||size<0)throw new Error('invalid asset range');
    if(size===0)return new Uint8Array();
    const r=await this.fetcher(url,{headers:{Range:`bytes=${offset}-${offset+size-1}`}});
    if(!(r.ok||r.status===206))throw new Error(`asset read failed: ${r.status}`);
    const bytes=new Uint8Array(await r.arrayBuffer());
    // Static hosting commonly ignores Range and returns 200/full content.
    if(r.status===200&&bytes.length!==size){
      if(offset+size>bytes.length)throw new Error(`asset range exceeds response: ${offset}+${size}>${bytes.length}`);
      return bytes.slice(offset,offset+size);
    }
    if(bytes.length!==size)throw new Error(`short asset range: expected ${size}, got ${bytes.length}`);
    return bytes;
  }
}
// Renderer-neutral bridge for TXD metadata emitted by the native/WASM asset
// parser. The bridge does not guess compressed pixel layouts: those are handed
// to a decoder before GPU upload. This keeps WebGPU policy out of the archive parser.
export function createTextureUploadDescriptor(meta) {
  if (!meta || !Number.isInteger(meta.width) || !Number.isInteger(meta.height) || meta.width<=0 || meta.height<=0) throw new Error('invalid TXD metadata');
  const mipLevels=Number.isInteger(meta.mipLevels)&&meta.mipLevels>0?meta.mipLevels:1;
  const compressed=!!meta.compressed;
  return {width:meta.width,height:meta.height,mipLevels,format:compressed?'renderware-compressed':(meta.format||'rgba8unorm'),hasAlpha:!!meta.hasAlpha,bytesPerRow:compressed?0:(meta.bytesPerRow||meta.width*4),usage:['TEXTURE_BINDING','COPY_DST']};
}
export class ContentManifest {
  constructor(manifest,baseUrl='.',vfs=new RangeVfs()){this.manifest=manifest;this.baseUrl=baseUrl;this.vfs=vfs;this.entries=new Map();
    if(!manifest||manifest.version!==1||!Array.isArray(manifest.assets))throw new Error('unsupported content manifest');
    for(const entry of manifest.assets){
      if(!entry?.id||!entry.path||!Number.isSafeInteger(entry.size)||entry.size<0)throw new Error('invalid content manifest entry');
      if(entry.path.startsWith('/')||entry.path.split('/').includes('..')||this.entries.has(entry.id))throw new Error(`unsafe content path: ${entry.path}`);
      this.entries.set(entry.id,entry);
    }
  }
  static async load(url='./content-manifest.json',fetcher=fetch){const r=await fetcher(url);if(!r.ok)throw new Error(`content manifest failed: ${r.status}`);return new ContentManifest(await r.json(),new URL('.',new URL(url,globalThis.location?.href||'http://localhost/')).href,new RangeVfs(fetcher));}
  entry(id){const entry=this.entries.get(id);if(!entry)throw new Error(`content asset not found: ${id}`);return entry;}
  url(entry){return new URL(entry.path,this.baseUrl).href;}
  async read(id,offset=0,size=null){const entry=this.entry(id);const length=size??(entry.size-offset);if(offset+length>entry.size)throw new Error(`content range exceeds manifest: ${id}`);return this.vfs.read(this.url(entry),offset,length);}
  async readVerified(id){const entry=this.entry(id);const bytes=await this.read(id,0,entry.size);if(entry.sha256&&globalThis.crypto?.subtle){const digest=await crypto.subtle.digest('SHA-256',bytes);const hash=[...new Uint8Array(digest)].map(x=>x.toString(16).padStart(2,'0')).join('');if(hash!==entry.sha256.toLowerCase())throw new Error(`content hash mismatch: ${id}`);}return bytes;}
}
export function installLifecycle({audio}={}){document.addEventListener('visibilitychange',()=>document.hidden?audio?.pause():audio?.resume());window.addEventListener('resize',()=>window.dispatchEvent(new CustomEvent('browser-game-resize',{detail:{width:innerWidth,height:innerHeight}})));}
export class GameRuntimeController {
  constructor(module,input,draw=null){this.module=module;this.input=input;this.draw=draw;this.running=false;this.last=0;}
  start(){if(this.module._sa_runtime_init()!==0)throw new Error('WASM runtime init failed');this.running=true;this.last=performance.now();requestAnimationFrame(t=>this.frame(t));}
  frame(now){if(!this.running)return;const dt=Math.min((now-this.last)/1000,0.1);this.last=now;const s=this.input.pollGamepad();this.module._sa_runtime_tick(dt,s.steering,s.throttle,s.brake);this.draw?.({x:this.module._sa_runtime_vehicle_x(),y:this.module._sa_runtime_vehicle_y(),heading:this.module._sa_runtime_vehicle_heading(),interior:this.module._sa_runtime_interior_active()===1,input:s});requestAnimationFrame(t=>this.frame(t));}
  stop(){this.running=false;}
  enterVehicle(model=411){return this.module._sa_runtime_enter_vehicle(model)===0;}
  enterInterior(id=0){return this.module._sa_runtime_enter_interior(id)===0;}
  runMission(){return this.module._sa_runtime_run_mission()===0;}
  save(slot='browser-slot'){const ptr=this.module.stringToNewUTF8(slot);try{return this.module._sa_runtime_save(ptr)===0;}finally{this.module._free(ptr);}}
  load(slot='browser-slot'){const ptr=this.module.stringToNewUTF8(slot);try{return this.module._sa_runtime_load(ptr)===0;}finally{this.module._free(ptr);}}
  pedState(index=0){return {count:this.module._sa_runtime_ped_count(),task:this.module._sa_runtime_ped_task(index),health:this.module._sa_runtime_ped_health(index)};}
}
