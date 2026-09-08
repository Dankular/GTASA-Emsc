import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { ContentManifest } from '../emscripten/web/browser_services.js';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const fixture = new Uint8Array(await fs.readFile(path.join(root, 'emscripten/web/asset-probe.txt')));
const manifest = JSON.parse(await fs.readFile(path.join(root, 'emscripten/web/content-manifest.json'), 'utf8'));
const fetcher = async (_url, options = {}) => {
  const range = options.headers?.Range || options.headers?.range;
  let start = 0, end = fixture.length - 1;
  if (range) [, start, end] = /^bytes=(\d+)-(\d+)$/.exec(range).map(Number);
  // Deliberately return 200/full data: this is the static-hosting fallback.
  return { ok: true, status: 200, async json() { return manifest; }, async arrayBuffer() { return fixture.slice(start === 0 ? 0 : 0, end + 1).buffer; } };
};
const content = new ContentManifest(manifest, 'http://localhost/', { read: async (_url, offset, size) => {
  const response = await fetcher('', { headers: { Range: `bytes=${offset}-${offset + size - 1}` } });
  const bytes = new Uint8Array(await response.arrayBuffer());
  return bytes.slice(offset, offset + size);
} });
const partial = await content.read('phase1-asset-probe', 4, 12);
assert.equal(partial.length, 12);
assert.deepEqual([...partial], [...fixture.slice(4, 16)]);
const full = await content.readVerified('phase1-asset-probe');
assert.equal(full.length, 76);
console.log(JSON.stringify({ browserContentSmoke: 'passed', manifestAssets: content.entries.size, partialRange: partial.length, verifiedBytes: full.length }));
