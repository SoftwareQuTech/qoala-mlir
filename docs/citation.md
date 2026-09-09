# Paper and citation

The design behind `qoala-mlir` — the design considerations, the optimization
passes (peephole rules, quantum dead-code elimination, MILP-based block
reordering), the deadline-estimation MILP, and the static analyses — is
described in:

> Sacha Bernheim, Bart van der Vecht, Davide Ferrari, Diego Rivera and
> Stephanie Wehner. **A Multi-Level Compiler Pipeline for Qoala Quantum
> Internet Programs**. In *2026 IEEE International Conference on Quantum
> Software (QSW)*, pages 12–24, Sydney, Australia, July 2026. IEEE.
> [doi:10.1109/QSW72780.2026.00012](https://doi.org/10.1109/QSW72780.2026.00012)

The paper is available at
[ieeexplore.ieee.org/document/11662182](https://ieeexplore.ieee.org/document/11662182).

## BibTeX

If you use `qoala-mlir` in academic work, please cite the paper:

```bibtex
@inproceedings{bernheim2026qoalacompiler,
  author    = {Bernheim, Sacha and van der Vecht, Bart and Ferrari, Davide and
               Rivera, Diego and Wehner, Stephanie},
  title     = {A Multi-Level Compiler Pipeline for Qoala Quantum Internet Programs},
  booktitle = {2026 IEEE International Conference on Quantum Software (QSW)},
  year      = {2026},
  month     = jul,
  address   = {Sydney, Australia},
  publisher = {IEEE},
  pages     = {12--24},
  doi       = {10.1109/QSW72780.2026.00012},
  url       = {https://ieeexplore.ieee.org/document/11662182},
}
```

## Where the paper maps onto the code

| Paper topic | Documentation |
| --- | --- |
| The three intermediate representations | [The three IRs](architecture/irs.md), [Operations reference](reference/index.md) |
| The pass pipeline as a whole | [Pass pipeline](architecture/pipeline.md) |
| Peephole rules and dead-code elimination | [LIR passes](passes/lir.md), [HIR passes](passes/hir.md) |
| MILP-based block reordering, deadline estimation | [LIR passes](passes/lir.md) |
| The selfish-vs-cooperative compilation knob | [Functionize algorithm](internals/functionize.md) |

## Related repositories

The compiler stack has three leaves, each with its own documentation:

- [euqalyptus](https://softwarequtech.github.io/euqalyptus/) — the Python
  frontend that emits Qoala HIR.
- `qoala-mlir` — this project, the MLIR-based middle and back end.
- [qoala-bench](https://github.com/SoftwareQuTech/qoala-bench) — the
  benchmarking harness that produced the paper's empirical results. The
  measurements themselves are published as a dataset:
  [doi:10.4121/bcac962c-fa21-40c5-9908-44ea5ccaaa5a.v1](https://doi.org/10.4121/bcac962c-fa21-40c5-9908-44ea5ccaaa5a.v1).
