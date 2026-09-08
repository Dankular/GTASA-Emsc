import { createBrowserServer } from './serve-browser.mjs';
import { UserInstallMount, WorldScene } from '../emscripten/web/browser_services.js';

const root = process.env.GTASA_ASSET_ROOT;
if (!root) throw new Error('GTASA_ASSET_ROOT must point to the user-owned San Andreas install');
const port = Number(process.env.PORT || 8110);
const app = createBrowserServer({ port, assetRoot: root });
await app.listen();
try {
  globalThis.location = { href: `http://127.0.0.1:${port}/` };
  const nativeFetch = globalThis.fetch;
  globalThis.fetch = (url, options) => nativeFetch(new URL(url, globalThis.location.href), options);
  const mount = await UserInstallMount.fromServer('/installed/models/gta3.img');
  const scene = new WorldScene();
  const asset = await scene.loadMountedAsset(mount);
  const state = scene.update(0, 0);
  if (!asset.mesh.positions.length || !asset.mesh.indices.length) throw new Error('decoded mesh is empty');
  console.log(JSON.stringify({
    realSceneSmoke: 'passed', source: 'user-owned GTA San Andreas install', archive: asset.archive,
    entry: asset.entry, vertices: asset.mesh.positions.length / 3,
    triangles: asset.mesh.indices.length / 3, streamedSector: state.sector,
  }));
} finally { app.close(); }
