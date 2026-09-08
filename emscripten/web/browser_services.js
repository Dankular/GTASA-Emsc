export class BrowserInput {
  constructor(target = window) { this.state={steering:0,throttle:0,brake:0,enter:false,cameraYaw:0,cameraPitch:0}; this.keys=new Set(); target.addEventListener('keydown',e=>this.key(e,true)); target.addEventListener('keyup',e=>this.key(e,false)); this.target=target; }
  key(e,down){ const c=e.key.toLowerCase(); if(['arrowleft','a'].includes(c))this.state.steering=down?-1:0; if(['arrowright','d'].includes(c))this.state.steering=down?1:0; if(['arrowup','w'].includes(c))this.state.throttle=down?1:0; if(['arrowdown','s'].includes(c))this.state.brake=down?1:0; if(c==='q')this.state.cameraYaw=down?-1:0; if(c==='e')this.state.cameraYaw=down?1:0; if(c==='r')this.state.cameraPitch=down?1:0; if(c==='f')this.state.cameraPitch=down?-1:0; if(c==='enter')this.state.enter=down; }
  pollGamepad(){ const p=navigator.getGamepads?.().find(Boolean); if(p){this.state.steering=p.axes?.[0]||0;this.state.throttle=Math.max(0,p.buttons?.[7]?.value||0);this.state.brake=Math.max(0,p.buttons?.[6]?.value||0);this.state.cameraYaw=p.axes?.[2]||0;this.state.cameraPitch=-(p.axes?.[3]||0);} return {...this.state}; }
  touch(accelerate,reverse){this.state.throttle=accelerate?1:0;this.state.brake=reverse?1:0;return {...this.state};}
  bindTouchControls({accelerate,reverse,left,right}={}){const bind=(el,fn)=>{if(!el)return;el.addEventListener('pointerdown',e=>{e.preventDefault();fn(true);});['pointerup','pointercancel','pointerleave'].forEach(t=>el.addEventListener(t,e=>{e.preventDefault();fn(false);}));};bind(accelerate,v=>this.state.throttle=v?1:0);bind(reverse,v=>this.state.brake=v?1:0);bind(left,v=>this.state.steering=v?-1:0);bind(right,v=>this.state.steering=v?1:0);return this;}
}
export class BrowserAudioGate {
  constructor(){this.context=null;this.unlocked=false;this.queue=[];this.radio=null;}
  async unlock(){this.context ||= new AudioContext(); await this.context.resume(); this.unlocked=this.context.state==='running'; return this.unlocked;}
  async pause(){if(this.context)await this.context.suspend();}
  async resume(){if(this.context)await this.context.resume();}
  schedule(id,when=0,channel='sfx'){if(!id)throw new Error('audio id required');const item={id,when:Math.max(0,when),channel};this.queue.push(item);this.queue.sort((a,b)=>a.when-b.when);return item;}
  playMission(id){return this.schedule(id,0,'mission');}
  playRadio(station,id){this.radio={station:String(station),id:String(id)};return this.schedule(id,0,'radio');}
  drain(now=0){const ready=this.queue.filter(x=>x.when<=now);this.queue=this.queue.filter(x=>x.when>now);return ready;}
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
export function installLifecycle({audio,canvas,runtime}={}){document.addEventListener('visibilitychange',()=>document.hidden?audio?.pause():audio?.resume());window.addEventListener('resize',()=>window.dispatchEvent(new CustomEvent('browser-game-resize',{detail:{width:innerWidth,height:innerHeight}})));canvas?.addEventListener('webglcontextlost',e=>{e.preventDefault();runtime?.contextLost('webgl-context-lost');});canvas?.addEventListener('webglcontextrestored',()=>runtime?.recoverContext());}
export class GameRuntimeController {
  constructor(module,input,draw=null){this.module=module;this.input=input;this.draw=draw;this.running=false;this.last=0;this.deviceLost=false;this.device=null;this.route={phase:'menu',elapsed:0,streamed:false,mission:false,interior:false,saved:false,audio:false};}
  attachDevice(device){this.device=device;device?.lost?.then(info=>this.contextLost(info?.message||'webgpu-device-lost'));return this;}
  contextLost(reason='device-lost'){this.deviceLost=true;this.stop();this.draw?.({deviceLost:true,reason});}
  recoverContext(){this.deviceLost=false;this.start();}
  start(){if(this.module._sa_runtime_init()!==0)throw new Error('WASM runtime init failed');this.running=true;this.last=performance.now();requestAnimationFrame(t=>this.frame(t));}
  frame(now){if(!this.running)return;const dt=Math.min((now-this.last)/1000,0.1);this.last=now;const s=this.input.pollGamepad();if(this.route.phase==='driving'){s.throttle=Math.max(s.throttle,.75);s.brake=0;}if(this.module._sa_runtime_tick_camera)this.module._sa_runtime_tick_camera(dt,s.steering,s.throttle,s.brake,s.cameraYaw,s.cameraPitch);else this.module._sa_runtime_tick(dt,s.steering,s.throttle,s.brake);this.route.elapsed+=dt;if(this.route.phase==='driving'&&this.route.elapsed>=2){this.route.interior=this.enterInterior(3);this.route.phase=this.route.interior?'interior':'failed';}if(this.route.phase==='interior'&&this.route.elapsed>=2.5){this.route.phase='complete';}this.draw?.({x:this.module._sa_runtime_vehicle_x(),y:this.module._sa_runtime_vehicle_y(),heading:this.module._sa_runtime_vehicle_heading(),speed:this.module._sa_runtime_vehicle_speed?.()||0,camera:this.module._sa_runtime_camera_x?{x:this.module._sa_runtime_camera_x(),y:this.module._sa_runtime_camera_y(),z:this.module._sa_runtime_camera_z(),lookX:this.module._sa_runtime_camera_look_x(),lookY:this.module._sa_runtime_camera_look_y(),lookZ:this.module._sa_runtime_camera_look_z()}:null,interior:this.module._sa_runtime_interior_active()===1,input:s,route:{...this.route}});requestAnimationFrame(t=>this.frame(t));}
  stop(){this.running=false;}
  enterVehicle(model=411){return this.module._sa_runtime_enter_vehicle(model)===0;}
  enterInterior(id=0){return this.module._sa_runtime_enter_interior(id)===0;}
  runMission(){return this.module._sa_runtime_run_mission()===0;}
  startRoute({audio=null}={}){if(!this.running)this.start();const entered=this.enterVehicle(411);const mission=this.runMission();this.route={phase:entered&&mission?'driving':'failed',elapsed:0,streamed:true,mission,interior:false,saved:false,audio:!!audio};if(audio&&mission)audio.playMission('mission-start');return entered&&mission;}
  save(slot='browser-slot'){const ptr=this.module.stringToNewUTF8(slot);try{return this.module._sa_runtime_save(ptr)===0;}finally{this.module._free(ptr);}}
  load(slot='browser-slot'){const ptr=this.module.stringToNewUTF8(slot);try{return this.module._sa_runtime_load(ptr)===0;}finally{this.module._free(ptr);}}
  async saveToBrowser(store,slot='browser-slot'){if(!this.save(slot))return false;await store.put(slot,{slot,x:this.module._sa_runtime_vehicle_x(),y:this.module._sa_runtime_vehicle_y(),heading:this.module._sa_runtime_vehicle_heading(),savedAt:Date.now()});return true;}
  async loadFromBrowser(store,slot='browser-slot'){const snapshot=await store.get(slot);return !!snapshot&&this.load(slot);}
  pedState(index=0){return {count:this.module._sa_runtime_ped_count(),task:this.module._sa_runtime_ped_task(index),health:this.module._sa_runtime_ped_health(index)};}
}

// Concrete WebGL2 fallback renderer. It consumes the renderer-neutral runtime
// state (vehicle transform + camera) and owns GPU resources on the browser side.
export class WebGL2Renderer {
  constructor(canvas){
    this.canvas=canvas; this.gl=canvas.getContext('webgl2',{antialias:true,alpha:false});
    if(!this.gl) throw new Error('WebGL2 is required for the Phase 1 renderer');
    const gl=this.gl;
    const vs=`#version 300 es\n in vec3 aPosition; in vec3 aNormal; uniform mat4 uMvp; uniform mat4 uModel; out vec3 vNormal; void main(){vNormal=mat3(uModel)*aNormal;gl_Position=uMvp*vec4(aPosition,1.0);}`;
    const fs=`#version 300 es\n precision mediump float; in vec3 vNormal; uniform vec4 uColor; out vec4 outColor; void main(){float l=max(dot(normalize(vNormal),normalize(vec3(-.4,.7,1.0))),.18);outColor=vec4(uColor.rgb*l,uColor.a);}`;
    const compile=(type,src)=>{const s=gl.createShader(type);gl.shaderSource(s,src);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw new Error(gl.getShaderInfoLog(s));return s;};
    this.program=gl.createProgram();gl.attachShader(this.program,compile(gl.VERTEX_SHADER,vs));gl.attachShader(this.program,compile(gl.FRAGMENT_SHADER,fs));gl.linkProgram(this.program);if(!gl.getProgramParameter(this.program,gl.LINK_STATUS))throw new Error(gl.getProgramInfoLog(this.program));
    this.vao=gl.createVertexArray();gl.bindVertexArray(this.vao);
    const verts=[]; const inds=[]; const faces=[[[0,0,-1],[0,1,2,3]],[[0,0,1],[4,7,6,5]],[[-1,0,0],[0,4,5,1]],[[1,0,0],[3,2,6,7]],[[0,-1,0],[0,3,7,4]],[[0,1,0],[1,5,6,2]]]; const p=[[-1,-.5,-.5],[1,-.5,-.5],[1,.5,-.5],[-1,.5,-.5],[-1,-.5,.5],[1,-.5,.5],[1,.5,.5],[-1,.5,.5]]; for(const [n,idx] of faces){const base=verts.length/6;for(const j of idx){verts.push(...p[j],...n);}inds.push(base,base+1,base+2,base,base+2,base+3);} this.count=inds.length;
    const vb=gl.createBuffer();gl.bindBuffer(gl.ARRAY_BUFFER,vb);gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(verts),gl.STATIC_DRAW);const ib=gl.createBuffer();gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,ib);gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(inds),gl.STATIC_DRAW);const pos=gl.getAttribLocation(this.program,'aPosition'),norm=gl.getAttribLocation(this.program,'aNormal');gl.enableVertexAttribArray(pos);gl.vertexAttribPointer(pos,3,gl.FLOAT,false,24,0);gl.enableVertexAttribArray(norm);gl.vertexAttribPointer(norm,3,gl.FLOAT,false,24,12);gl.bindVertexArray(null); this.mvp=gl.getUniformLocation(this.program,'uMvp');this.model=gl.getUniformLocation(this.program,'uModel');this.color=gl.getUniformLocation(this.program,'uColor'); gl.enable(gl.DEPTH_TEST);
  }
  resize(){const d=devicePixelRatio||1,w=Math.max(1,this.canvas.clientWidth*d),h=Math.max(1,this.canvas.clientHeight*d);if(this.canvas.width!==w||this.canvas.height!==h){this.canvas.width=w;this.canvas.height=h;}this.gl.viewport(0,0,w,h);}
  draw(s){const gl=this.gl;this.resize();const c=s.camera||{x:0,y:-8,z:4,lookX:s.x,lookY:s.y,lookZ:1};const view=lookAt(c.x,c.y,c.z,c.lookX,c.lookY,c.lookZ),proj=perspective(Math.PI/3,this.canvas.width/this.canvas.height,.1,1000),model=compose(s.x,s.y,0.8,s.heading*Math.PI/180);gl.clearColor(.035,.06,.09,1);gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);gl.useProgram(this.program);gl.bindVertexArray(this.vao);gl.uniformMatrix4fv(this.mvp,false,mul(proj,mul(view,model)));gl.uniformMatrix4fv(this.model,false,model);gl.uniform4f(this.color,s.interior ? .95 : .12,s.interior ? .55 : .65,.18,1);gl.drawElements(gl.TRIANGLES,this.count,gl.UNSIGNED_SHORT,0);gl.bindVertexArray(null);}
}
function mul(a,b){const o=new Float32Array(16);for(let r=0;r<4;r++)for(let c=0;c<4;c++)for(let k=0;k<4;k++)o[c*4+r]+=a[k*4+r]*b[c*4+k];return o;}
function perspective(f,asp,n,far){const t=1/Math.tan(f/2),o=new Float32Array(16);o[0]=t/asp;o[5]=t;o[10]=(far+n)/(n-far);o[11]=-1;o[14]=2*far*n/(n-far);return o;}
function lookAt(ex,ey,ez,cx,cy,cz){let z=[ex-cx,ey-cy,ez-cz],l=Math.hypot(...z);z=z.map(v=>v/l);let x=[z[1],-z[0],0],xl=Math.hypot(...x);x=x.map(v=>v/(xl||1));const y=[z[1]*x[2]-z[2]*x[1],z[2]*x[0]-z[0]*x[2],z[0]*x[1]-z[1]*x[0]],o=new Float32Array([x[0],y[0],z[0],0,x[1],y[1],z[1],0,x[2],y[2],z[2],0,-(x[0]*ex+x[1]*ey+x[2]*ez),-(y[0]*ex+y[1]*ey+y[2]*ez),-(z[0]*ex+z[1]*ey+z[2]*ez),1]);return o;}
function compose(x,y,z,a){const c=Math.cos(a),s=Math.sin(a);return new Float32Array([c,s,0,0,-s,c,0,0,0,0,1,0,x,y,z,1]);}
