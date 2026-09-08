import assert from 'node:assert/strict';
import { createTextureUploadDescriptor, uploadRgbaTexture } from '../emscripten/web/browser_services.js';
const d=createTextureUploadDescriptor({width:64,height:32,mipLevels:3,format:'rgba8unorm',bytesPerRow:256,hasAlpha:true});
assert.deepEqual(d,{width:64,height:32,mipLevels:3,format:'rgba8unorm',hasAlpha:true,bytesPerRow:256,usage:['TEXTURE_BINDING','COPY_DST']});
assert.throws(()=>createTextureUploadDescriptor({width:0,height:1}));
const calls=[]; const gl={TEXTURE_2D:1,UNPACK_FLIP_Y_WEBGL:2,TEXTURE_MIN_FILTER:3,TEXTURE_MAG_FILTER:4,TEXTURE_WRAP_S:5,TEXTURE_WRAP_T:6,LINEAR:7,LINEAR_MIPMAP_LINEAR:8,REPEAT:9,RGBA:10,UNSIGNED_BYTE:11,createTexture:()=>({}),bindTexture:(...x)=>calls.push(['bind',...x]),pixelStorei:(...x)=>calls.push(['flip',...x]),texParameteri:(...x)=>calls.push(['param',...x]),texImage2D:(...x)=>calls.push(['image',...x]),generateMipmap:(...x)=>calls.push(['mip',...x])};
uploadRgbaTexture(gl,{width:1,height:1,rgba:new Uint8Array([255,0,0,255])},{mipmaps:false}); assert.ok(calls.some(x=>x[0]==='image'));
console.log(JSON.stringify({txdBrowserSmoke:'passed',descriptor:d,webglUpload:'passed'}));
