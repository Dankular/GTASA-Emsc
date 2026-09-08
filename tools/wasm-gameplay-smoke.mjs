import fs from 'node:fs';
import { fileURLToPath } from 'node:url';

const root = fileURLToPath(new URL('../', import.meta.url));
const factory = (await import(new URL('../emscripten/web/sa_port_probe.js', import.meta.url))).default;
const wasmBinary = fs.readFileSync(new URL('../emscripten/web/sa_port_probe.wasm', import.meta.url));
const module = await factory({ wasmBinary, noInitialRun: true, print: () => {}, printErr: console.error });
if (module._sa_runtime_init() !== 0) throw new Error('runtime init failed');
if (module._sa_runtime_enter_vehicle(411) !== 0) throw new Error('vehicle entry failed');
module._sa_runtime_tick(1, 0, 1, 0);
const x = module._sa_runtime_vehicle_x();
if (!(x > 0)) throw new Error(`vehicle did not move: ${x}`);
if (module._sa_runtime_enter_interior(3) !== 0 || module._sa_runtime_interior_active() !== 1) throw new Error('interior route failed');
console.log(JSON.stringify({ runtime: 'initialized', vehicleEntry: 'passed', movementX: x, interiorEntry: 'passed' }));
