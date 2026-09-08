# Binary/Compass mapping contract

The SA port adopts `Dankular/Binary` (Compass) as the verification tool for the
`gta-reversed` subsystem map. It is pinned as a git submodule under
`tools/binary`, deliberately separate from the runtime and source tree.

`tools/map-gta-subsystems.ps1` performs the first reproducible pass by walking
all C/C++ files and `docs/hooks.csv`, then recording portability-risk clusters
and representative evidence files. A second pass with Compass can verify
function-level behavior against a user-owned legal executable or fixture.

Track two independent statuses: **source mapped** (inventory evidence exists)
and **portable verified** (Binary confirms behavior and the detached browser
implementation has a regression test). A pattern match is evidence to inspect,
not a claim that a subsystem is complete.
