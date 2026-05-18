# Getting started

This page covers the quickest path to a working qoala-mlir install: download a pre-built release from the GitHub releases page and start running `qoala-opt` and `qoala-translate`.

If you need to modify the toolchain itself (add a pass, change a dialect, hack on the Python bindings), follow [Building from source](developer-guide/build-from-source.md) under the Developer's guide instead.

## Releases live on GitHub

Every release publishes its artifacts to the GitHub releases page:

> [`<QOALA_MLIR_RELEASES_URL>`](<QOALA_MLIR_RELEASES_URL>)

Each release attaches two kinds of artifacts:

- A **binary tarball** (e.g. `qoala-mlir-<version>-linux-x86_64.tar.gz`) containing the standalone `qoala-opt` and `qoala-translate` executables. Use this if you only need to run the tools on `.mlir` files.
- A **Python wheel** (e.g. `qoala_mlir-<version>-cp311-cp311-linux_x86_64.whl`) containing the same executables plus the `qnet` Python bindings package. Use this if you (or [euqalyptus](<EUQALYPTUS_DOCS_URL>)) need to construct HIR from Python.

Pick whichever matches your platform (Linux x86-64 is the primary target).

## Path A — install just the binaries

Download the `qoala-mlir-<version>-<platform>.tar.gz` for your release, then:

```sh
mkdir -p ~/.local/qoala-mlir
tar -xzf qoala-mlir-<version>-linux-x86_64.tar.gz -C ~/.local/qoala-mlir
export PATH="$HOME/.local/qoala-mlir/bin:$PATH"
```

(Add the `export PATH=` line to your shell rc so the binaries stay on `PATH` across sessions.)

Smoke-test:

```sh
qoala-opt --help | head
qoala-translate --help | head
```

You should see the `qoala-opt` cost-model flags listed (see [Tools / qoala-opt](tools/qoala-opt.md)) and a `--mlir-to-iqoala` translation under `qoala-translate --help`.

This is enough if you're feeding hand-written `.mlir` files (or `.mlir` files emitted by a separate tool) through the pipeline.

## Path B — install the wheel (binaries + Python bindings)

The wheel bundles the same binaries *plus* the `qnet` Python bindings, all in one install.

Requirements:

- **Python 3.10, 3.11, or 3.12.** Python 3.13 is **not supported** — the bundled MLIR Python bindings depend on `numpy ≤ 1.26`, which itself does not support 3.13.
- Each release attaches a **separate wheel per supported Python version** (e.g. `cp310`, `cp311`, `cp312` tags in the filename). Pick the one that matches the interpreter you intend to run; wheels are not interchangeable across Python minor versions because the bundled MLIR Python bindings link against a specific CPython ABI.

In a fresh virtual environment, install the wheel directly from the release URL:

```sh
python3.11 -m venv .venv
source .venv/bin/activate
pip install https://github.com/<org>/qoala-mlir/releases/download/<version>/qoala_mlir-<version>-cp311-cp311-linux_x86_64.whl
```

(Replace the `<…>` with the actual release URL for your version. The exact filename varies by Python and platform.)

The wheel installs:

- `qoala-opt` and `qoala-translate` under your venv's `bin/`.
- The `qnet` Python bindings, importable as `qnet.dialects.qnet` and `qnet.ir`.
- The MLIR runtime libraries needed by both.

Smoke-test:

```sh
qoala-opt --help | head
python -c "from qnet.dialects import qnet; from qnet.ir import Module, Context, Location; print('ok')"
```

## Run a sample program

The simplest end-to-end exercise: take a Qoala HIR program produced by [euqalyptus](<EUQALYPTUS_DOCS_URL>), lower it, and translate it.

```sh
qoala-opt program.hir.mlir \
  --qnet-peephole-optimizations \
  --qnet-dead-code-elimination \
  --lower-qoala-hir-to-mir \
  --lower-qoala-mir-to-lir \
| qoala-translate --mlir-to-iqoala > program.iqoala
```

See [Architecture / Pass pipeline](architecture/pipeline.md) for what each pass does and which combinations are recommended.

If you don't yet have an HIR module, the [euqalyptus docs](<EUQALYPTUS_DOCS_URL>) walk through writing one in Python.

## Where to go next

- Run a specific pass or look up a flag: [Tools / qoala-opt](tools/qoala-opt.md), [Passes reference](passes/index.md).
- Look up an op: [Operations reference](reference/index.md).
- Use the Python bindings to construct HIR: [Python bindings](bindings.md).
- Modify the toolchain itself: [Developer's guide / Building from source](developer-guide/build-from-source.md).
