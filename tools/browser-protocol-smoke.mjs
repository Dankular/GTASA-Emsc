import assert from 'node:assert/strict';
import { createBrowserServer } from './serve-browser.mjs';
const app=createBrowserServer({port:0}); const addr=await app.listen(); const base=`http://127.0.0.1:${addr.port}`;
try {
  const page=await fetch(`${base}/`); const html=await page.text(); assert.equal(page.status,200); assert.match(page.headers.get('content-type'),/text\/html/); assert.equal(page.headers.get('cache-control'),'no-store'); assert.equal(page.headers.get('cross-origin-opener-policy'),'same-origin'); assert.equal(page.headers.get('cross-origin-embedder-policy'),'require-corp'); assert.match(html,/id="menu"/); assert.match(html,/id="new-game"/);
  const wasm=await fetch(`${base}/sa_port_probe.wasm`,{headers:{Range:'bytes=0-7'}}); assert.equal(wasm.status,206); assert.equal(wasm.headers.get('content-type'),'application/wasm'); assert.match(wasm.headers.get('content-range'),/^bytes 0-7\//); assert.equal((await wasm.arrayBuffer()).byteLength,8);
  const badRange=await fetch(`${base}/sa_port_probe.wasm`,{headers:{Range:'bytes=999999999-'}}); assert.equal(badRange.status,416);
  const secondChunk=await fetch(`${base}/sa_port_probe.wasm`,{headers:{Range:'bytes=8-15'}}); assert.equal(secondChunk.status,206); assert.equal((await secondChunk.arrayBuffer()).byteLength,8); assert.match(secondChunk.headers.get('cache-control'),/immutable/);
  const missing=await fetch(`${base}/missing-runtime.js`); assert.equal(missing.status,404); assert.match(await missing.text(),/not found/);
  console.log(JSON.stringify({browserProtocolSmoke:'passed',base,range:'206',invalidRange:'416',missing:'404',isolated:true}));
} finally { await app.close(); }
