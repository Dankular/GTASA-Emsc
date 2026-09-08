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
  async read(url,offset,size){const r=await this.fetcher(url,{headers:{Range:`bytes=${offset}-${offset+size-1}`}});if(!(r.ok||r.status===206))throw new Error(`asset read failed: ${r.status}`);return new Uint8Array(await r.arrayBuffer());}
}
export function installLifecycle({audio}={}){document.addEventListener('visibilitychange',()=>document.hidden?audio?.pause():audio?.resume());window.addEventListener('resize',()=>window.dispatchEvent(new CustomEvent('browser-game-resize',{detail:{width:innerWidth,height:innerHeight}})));}
