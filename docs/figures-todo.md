# Figures to (re)draw

After triage, the docs site needs **two** figures — both can be exported directly from the accompanying paper's LaTeX source rather than redrawn from scratch. Drop the exported PDF/SVG into `docs/assets/figures/` under the filename listed below; the markdown references already point at those paths.

## `pipeline-overview.svg`

- **Referenced from:** `docs/index.md`, `docs/overview.md`.
- **Source:** Export from the paper's `fig:qoala-compiler-architecture` (the big TikZ overview in `03-architecture/architecture.tex`). The content matches what the docs need: user Python → euqalyptus frontend → HIR/MIR/LIR stack → `.iqoala`.

## `backend-overview.svg`

- **Referenced from:** `docs/architecture/index.md`, `docs/overview.md`.
- **Source:** Export from the paper's `fig:backend` (the qoala-translate detail TikZ in `03-architecture/architecture.tex`).

## Figures intentionally dropped

The following stubs were removed because the prose + code blocks on each page already carry the meaning; reintroduce them only if a future doc revision explicitly needs the visual:

- `pass-pipeline-default.svg` — `passes/pipeline.md` already lists the passes in a sequenced shell-command block.
- `dialect-graph.svg` — `architecture/irs.md` and `reference/index.md` already enumerate the dialects per IR in prose.
- `qnet-op-anatomy.svg` — HIR section of `architecture/irs.md` already explains "value-to-value" with a code example.
- `qmem-memory-model.svg` — MIR section of `architecture/irs.md` already explains "side-effecting on `i32` pointers" with a code example.
- `qoalahost-block-structure.svg` — LIR section of `architecture/irs.md` already shows a complete `qoalahost.main_func` example.
- `netqasm-routine.svg` — covered by the LIR example and the `netqasm` reference page.
- `python-bindings-arch.svg` — `bindings.md` walks through the CMake wiring in prose.
