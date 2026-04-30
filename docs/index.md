# qoala-mlir

`qoala-mlir` is the MLIR-based middle and back end of the Qoala compiler stack. It consumes a high-level intermediate representation (Qoala HIR) emitted by the [euqalyptus](<EUQALYPTUS_DOCS_URL>) Python frontend, lowers it through three intermediate representations, and emits an `.iqoala` executable that can be run on the Qoala runtime.

The repository builds two command-line tools and a Python bindings package:

- **`qoala-opt`** — runs analyses and optimizations, and lowers between IRs. See [qoala-opt](tools/qoala-opt.md).
- **`qoala-translate`** — translates the lowest IR to the textual `.iqoala` format. See [qoala-translate](tools/qoala-translate.md).
- **`qnet` Python bindings** — used by the [euqalyptus](<EUQALYPTUS_DOCS_URL>) frontend to construct Qoala HIR programmatically. See [Python bindings](bindings.md).

![Big picture: full pipeline](assets/figures/pipeline-overview.svg)

## Where to start

- New to the project? Read the [Overview](overview.md), then follow [Getting started](getting-started.md) to install a release and run the toolchain. Building from a checkout lives under the [Developer's guide](developer-guide/build-from-source.md).
- Looking to invoke a specific pass or flag? Jump to [Tools / qoala-opt](tools/qoala-opt.md) or [Passes reference](passes/index.md).
- Looking up an op? Pick the dialect in [Operations reference](reference/index.md).
- Contributing or working on the dialects themselves? Start with the [Developer's guide](developer-guide/working-with-mlir.md) and the [Internals](internals/functionize.md) notes.

## What lives where

| You want to… | Go to |
| --- | --- |
| Understand the pipeline at a glance | [Overview](overview.md), [Architecture](architecture/index.md) |
| Install a release / run the binaries | [Getting started](getting-started.md) |
| Build the project from source | [Developer's guide / Building from source](developer-guide/build-from-source.md) |
| Look up an op in HIR / MIR / LIR | [Operations reference](reference/index.md) |
| Look up a pass and its options | [Passes reference](passes/index.md) |
| See every `qoala-opt` / `qoala-translate` flag | [Tools](tools/index.md) |
| Write a Python script that produces Qoala HIR | [Python bindings](bindings.md) and the [euqalyptus docs](<EUQALYPTUS_DOCS_URL>) |
| Add a new dialect, op or pass | [Developer's guide](developer-guide/working-with-mlir.md), [Contributing](contributing.md) |
