import { UserInstallMount, WorldScene, createRenderWareFixture } from '../emscripten/web/browser_services.js';

// Synthetic IMG v2 container: this exercises the same mounted-archive path
// without copying or redistributing any proprietary San Andreas content.
const dff = createRenderWareFixture();
const image = new Uint8Array(4096);
const header = new DataView(image.buffer);
image.set(new TextEncoder().encode('VER2'), 0);
header.setUint32(4, 1, true);
header.setUint32(8, 1, true);
header.setUint32(12, 1, true);
image.set(new TextEncoder().encode('mounted-fixture.dff'), 16);
image.set(dff, 2048);
const file = new File([image], 'gta3.img');
Object.defineProperty(file, 'webkitRelativePath', { value: 'models/gta3.img' });
const mount = await UserInstallMount.fromFiles([file]);
const scene = new WorldScene();
const asset = await scene.loadMountedAsset(mount);
if (asset.entry !== 'mounted-fixture.dff' || asset.mesh.indices.length === 0) throw new Error('mounted DFF was not promoted to a scene asset');
scene.update(0, 0);
if (scene.visible?.[0]?.model !== asset.entry) throw new Error('mounted asset is not visible in streamed scene');
console.log(JSON.stringify({ ok: true, archive: asset.archive, entry: asset.entry, triangles: asset.mesh.indices.length / 3, instances: scene.visible.length }));
