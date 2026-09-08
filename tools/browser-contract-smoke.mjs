import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execFileSync } from 'node:child_process';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const web = path.join(root, 'emscripten', 'web');
const required = ['index.html', 'browser_services.js', 'manifest.webmanifest', 'content-manifest.json', 'asset-probe.txt', 'sa_port_probe.js', 'sa_port_probe.wasm'];
const checks = [];
const check = (name, passed, detail = '') => checks.push({ name, passed: !!passed, detail });

for (const file of required) check(`file:${file}`, fs.existsSync(path.join(web, file)), 'required browser artifact');

const html = fs.readFileSync(path.join(web, 'index.html'), 'utf8');
const services = fs.readFileSync(path.join(web, 'browser_services.js'), 'utf8');
const js = fs.readFileSync(path.join(web, 'sa_port_probe.js'), 'utf8');
const wasm = fs.readFileSync(path.join(web, 'sa_port_probe.wasm'));
check('wasm:magic', wasm.subarray(0, 4).equals(Buffer.from([0, 0x61, 0x73, 0x6d])), 'WebAssembly binary header');
for (const ref of ['./browser_services.js', './sa_port_probe.js', './content-manifest.json']) {
  check(`html:reference:${ref}`, html.includes(ref) || html.includes(ref.slice(2)), 'index.html references runtime asset');
}
for (const symbol of ['_sa_runtime_tick', '_sa_runtime_vehicle_x', '_sa_runtime_interior_active', '_sa_services_smoke']) {
  check(`wasm:export:${symbol}`, js.includes(symbol), 'generated Emscripten glue exposes runtime ABI');
}
for (const feature of ['indexedDB', 'Range', 'visibilitychange', 'getGamepads', 'createTextureUploadDescriptor', 'renderware-compressed']) {
  check(`browser-service:${feature}`, services.includes(feature), 'browser service contract');
}
check('browser-service:requestFullscreen', html.includes('requestFullscreen'), 'page exposes fullscreen control');
check('legal-boundary:no-native-execution', !html.includes('gta_sa.exe') && !services.includes('gta_sa.exe'), 'browser does not launch the native game');

try {
  execFileSync(process.execPath, ['--check', path.join(web, 'browser_services.js')], { stdio: 'pipe' });
  check('javascript:syntax', true, 'browser services parse cleanly');
} catch (error) {
  check('javascript:syntax', false, error.stderr?.toString() || error.message);
}

const failed = checks.filter(x => !x.passed);
const result = { ok: failed.length === 0, checks };
const report = process.env.BROWSER_CONTRACT_REPORT;
if (report) fs.writeFileSync(path.resolve(report), JSON.stringify(result, null, 2));
console.log(JSON.stringify(result, null, 2));
if (failed.length) process.exitCode = 1;
