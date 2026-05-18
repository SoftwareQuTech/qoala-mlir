# qoala-mlir

`qoala-mlir` is the MLIR-based middle and back end of the Qoala compiler stack. It consumes a Qoala HIR module — emitted by the [euqalyptus](<EUQALYPTUS_DOCS_URL>) Python frontend — lowers it through three intermediate representations (`qnet` → `qmem` → `qoalahost` + `netqasm` + `qremote`), and emits a textual `.iqoala` executable that can be run on the Qoala runtime. The build produces two command-line tools, `qoala-opt` (analyses, optimizations, and lowerings) and `qoala-translate` (LIR-to-`.iqoala` translation), together with a `qnet` Python bindings package that the frontend imports to construct HIR programmatically.

## Documentation

The full documentation is published at [`<QOALA_MLIR_DOCS_URL>`](<QOALA_MLIR_DOCS_URL>). It covers installation, the three intermediate representations, the pass pipeline, the per-op and per-pass reference, the Python bindings, and contributor-facing material.

## Design and paper

For a deeper account of the compiler's design — the design considerations, the optimization passes (peephole rules, quantum dead-code elimination, MILP-based block reordering), the deadline-estimation MILP, and the static analyses — please refer to the accompanying paper: [`<PAPER_URL>`](<PAPER_URL>).

## Running the documentation locally

You can serve the documentation site locally with the official `squidfunk/mkdocs-material` Docker image, without installing MkDocs into your environment. Because the site uses the `mkdocstrings[python]` plugin to render API documentation from the `qnet` Python bindings' docstrings, the command below installs the docs-build dependencies (listed in [`requirements-docs.txt`](requirements-docs.txt)) into the container before serving:

```sh
docker run --rm -it -p 8000:8000 -v "$(pwd)":/docs \
  --entrypoint sh squidfunk/mkdocs-material:latest \
  -c 'pip install --quiet -r requirements-docs.txt && mkdocs serve --dev-addr=0.0.0.0:8000'
```

Run the command from the repository root. The site is then available at <http://localhost:8000>, with live reload on every change to `docs/`, `mkdocs.yml`, or the docstrings under `lib/Python/mlir_qnet/`.

## Citation

If you use `qoala-mlir` in academic work, please cite the accompanying paper. A BibTeX entry will be available alongside the paper at the URL above; the placeholder below will be replaced once the paper is published:

```bibtex
<BIBTEX_PLACEHOLDER>
```

## License

`qoala-mlir` is released under the MIT License (Copyright © 2025 QuTech). See the [`LICENSE`](LICENSE) file for the full text.
