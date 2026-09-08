import assert from 'node:assert/strict';
import { createTextureUploadDescriptor } from '../emscripten/web/browser_services.js';
const d=createTextureUploadDescriptor({width:64,height:32,mipLevels:3,format:'rgba8unorm',bytesPerRow:256,hasAlpha:true});
assert.deepEqual(d,{width:64,height:32,mipLevels:3,format:'rgba8unorm',hasAlpha:true,bytesPerRow:256,usage:['TEXTURE_BINDING','COPY_DST']});
assert.throws(()=>createTextureUploadDescriptor({width:0,height:1}));
console.log(JSON.stringify({txdBrowserSmoke:'passed',descriptor:d}));
