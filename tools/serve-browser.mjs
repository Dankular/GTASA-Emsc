import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

export function createBrowserServer({root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../emscripten/web'),port=8080,assetRoot=process.env.GTASA_ASSET_ROOT||null}={}) {
  const mime={'.html':'text/html; charset=utf-8','.js':'text/javascript; charset=utf-8','.json':'application/json; charset=utf-8','.wasm':'application/wasm','.webmanifest':'application/manifest+json','.txt':'text/plain; charset=utf-8'};
  const server=http.createServer((req,res)=>{
    const clean=decodeURIComponent((req.url||'/').split('?')[0]);
    const rel=clean==='/'?'/index.html':clean;
    // Optional local developer mount. It exposes only user-owned installed
    // assets under /installed and never copies them into this repository.
    const isInstalled=rel.toLowerCase().startsWith('/installed/');
    const base=isInstalled&&assetRoot?path.resolve(assetRoot):path.resolve(root);
    const requested=isInstalled?rel.slice('/installed/'.length):'.'+rel;
    const file=path.resolve(base,requested);
    if (!file.startsWith(base+path.sep)) { res.writeHead(403); res.end('forbidden'); return; }
    if (isInstalled&&!assetRoot) { res.writeHead(404); res.end('installed asset mount disabled'); return; }
    if (!fs.existsSync(file)||!fs.statSync(file).isFile()) { res.writeHead(404,{'Content-Type':'text/plain'}); res.end('not found'); return; }
    const stat=fs.statSync(file), ext=path.extname(file).toLowerCase(), headers={'Content-Type':mime[ext]||'application/octet-stream','Cross-Origin-Opener-Policy':'same-origin','Cross-Origin-Embedder-Policy':'require-corp','Cross-Origin-Resource-Policy':'same-origin','Accept-Ranges':'bytes','Vary':'Range'};
    if (ext==='.html') headers['Cache-Control']='no-store'; else headers['Cache-Control']='public, max-age=31536000, immutable';
    const range=req.headers.range;
    if (range) {
      const match=/^bytes=(\d+)-(\d*)$/.exec(range);
      if (!match) { res.writeHead(416,{...headers,'Content-Range':`bytes */${stat.size}`}); res.end(); return; }
      const start=Number(match[1]), end=match[2]?Number(match[2]):stat.size-1;
      if (start>=stat.size||end<start) { res.writeHead(416,{...headers,'Content-Range':`bytes */${stat.size}`}); res.end(); return; }
      const bounded=Math.min(end,stat.size-1); res.writeHead(206,{...headers,'Content-Length':bounded-start+1,'Content-Range':`bytes ${start}-${bounded}/${stat.size}`}); fs.createReadStream(file,{start,end:bounded}).pipe(res); return;
    }
    res.writeHead(200,{...headers,'Content-Length':stat.size}); fs.createReadStream(file).pipe(res);
  });
  return {server,listen:()=>new Promise(resolve=>server.listen(port,'127.0.0.1',()=>resolve(server.address()))),close:()=>new Promise(resolve=>server.close(resolve))};
}

if (process.argv[1]===fileURLToPath(import.meta.url)) {
  const port=Number(process.env.PORT||process.argv[2]||8080); const app=createBrowserServer({port}); await app.listen(); console.log(`browser-server=http://127.0.0.1:${port}`);
}
