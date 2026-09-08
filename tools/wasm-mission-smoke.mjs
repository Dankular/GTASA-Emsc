import fs from 'node:fs';

const factory = (await import(new URL('../emscripten/web/sa_port_probe.js', import.meta.url))).default;
const wasmBinary = fs.readFileSync(new URL('../emscripten/web/sa_port_probe.wasm', import.meta.url));
const module = await factory({ wasmBinary, noInitialRun: true, print: () => {}, printErr: console.error });
if (module._sa_runtime_init() !== 0) throw new Error('runtime init failed');
if (module._sa_runtime_run_mission() !== 0) throw new Error('mission execution failed');
const count = module._sa_runtime_ped_count();
const task = module._sa_runtime_ped_task(0);
const health = module._sa_runtime_ped_health(0);
if (count !== 1 || task !== 1 || health !== 80) throw new Error(`ped state mismatch: ${count}/${task}/${health}`);
if (module._sa_runtime_interior_active() !== 1) throw new Error('mission did not enter interior');
const slot = module.stringToNewUTF8('mission-smoke');
try {
  if (module._sa_runtime_save(slot) !== 0) throw new Error('runtime save failed');
  if (module._sa_runtime_enter_interior(0) !== 0 || module._sa_runtime_interior_active() !== 1) throw new Error('pre-load state transition failed');
  if (module._sa_runtime_load(slot) !== 0 || module._sa_runtime_interior_active() !== 1) throw new Error('runtime load failed');
} finally { module._free(slot); }
console.log(JSON.stringify({ mission: 'passed', ped: { count, task, health }, interior: 'passed', saveLoad: 'passed' }));
