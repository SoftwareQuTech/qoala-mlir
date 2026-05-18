# qoala-mlir

`qoala-mlir` is the MLIR-based middle and back end of the Qoala compiler stack. It consumes a high-level intermediate representation (Qoala HIR) emitted by the [euqalyptus](<EUQALYPTUS_DOCS_URL>) Python frontend, lowers it through three intermediate representations, and emits an `.iqoala` executable that can be run on the Qoala runtime.

The repository builds two command-line tools and a Python bindings package. `qoala-opt` runs analyses and optimizations and lowers between IRs — see [qoala-opt](tools/qoala-opt.md). `qoala-translate` translates the lowest IR to the textual `.iqoala` format — see [qoala-translate](tools/qoala-translate.md). And the `qnet` Python bindings package is what [euqalyptus](<EUQALYPTUS_DOCS_URL>) imports to construct Qoala HIR programmatically — see [Python bindings](bindings.md).

![Big picture: full pipeline](assets/figures/pipeline-overview.svg)

## Where to start

If you are new to the project, the [Overview](overview.md) sketches the pipeline end-to-end, and [Getting started](getting-started.md) walks you through installing a release and running the toolchain. Building from a checkout (rather than installing a release) lives under the [Developer's guide](developer-guide/build-from-source.md). If you already know what you are looking for, jump straight into [Tools / qoala-opt](tools/qoala-opt.md) or the [Passes reference](passes/index.md) for invoking a specific pass or flag, into the [Operations reference](reference/index.md) for looking up a dialect operation, or into the [Developer's guide](developer-guide/working-with-mlir.md) and the [Internals](internals/functionize.md) notes if you are contributing to the dialects themselves.

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
