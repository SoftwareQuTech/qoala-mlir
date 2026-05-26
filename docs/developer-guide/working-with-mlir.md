# Working with MLIR

This page collects the practical knowledge needed to extend the qoala-mlir codebase: declaring a new pass, defining a new op, wiring TableGen output into the build, and the few MLIR conventions the existing dialects rely on. It is a *developer's* reference, not a tutorial. For an introduction to MLIR itself, see the [official MLIR docs](https://mlir.llvm.org/docs/).

## Defining a pass with TableGen

A pass is declared in a `.td` file under the dialect that owns it. The minimum looks like this (compare with `include/Dialect/QNet/Passes.td`):

```tablegen
include "mlir/Pass/PassBase.td"

def MyPass : Pass<"my-pass", "mlir::ModuleOp"> {
    let summary = "One-line description shown in --help.";
    let description = [{
        Longer description, can span multiple lines.
    }];
    // List dialects this pass *uses*: it inspects ops in them or creates them.
    // BuiltinDialect is implicit.
    // This list depends on the dialects that you use in your pass. In this example, we assume these two.
    // Declaring a dependency on mlir::BuiltinDialect is not necessary, since it is implicit.
    let dependentDialects = [
        "mlir::func::FuncDialect",
        "qoala::dialects::qnet::QNetDialect"
    ];
}
```

The first argument to `Pass<…>` is the CLI mnemonic (`--my-pass`), and the second is the op type the pass anchors on (typically `mlir::ModuleOp` or a function-like op such as `qmem::FuncOp`). Declaring an anchor type here immediately introduces a dependency on the dialect the pass anchors to.

### Wire the TableGen output into CMake

Using the pass declaration in the `.td` file, TableGen (via CMake) can generate a `MyPass.h.inc` header which is the base for the real pass header file:

```cmake
set(LLVM_TARGET_DEFINITIONS MyPass.td)
mlir_tablegen(MyPass.h.inc -gen-pass-decls -name MyPass)
add_public_tablegen_target(MLIRMyPassIncGen)
```

After this, you need to create a `Passes.h` header that uses the `.h.inc` file created by TableGen. This new header has the following standard skeleton (compare with `include/Dialect/QNet/Passes.h`):

```cpp
#include "mlir/Pass/Pass.h"

namespace qoala::analysis {
// The following two lines inserts declarations of the base classes of the pass
#define GEN_PASS_DECL
#include "Dialect/MyDialect/Passes.h.inc"

// The following two lines creates declarations of the functions used to register the pass in qoala-opt
#define GEN_PASS_REGISTRATION
#include "Dialect/MyDialect/Passes.h.inc"
} // namespace qoala::analysis
```

### Implement the pass

In a `.cpp` file under `lib/Analysis/<MyDialect>/`:

```cpp
// We insert the delcarations of the classes and functions from the main pass header
#include "Dialect/MyDialect/Passes.h"
#include "mlir/IR/BuiltinOps.h"


namespace qoala::analysis {
// Using the TableGen generated file, we insert the definitions of the pass' base classes.
// These definitions match the declarations imported from TableGen placed in the main pass header class
#define GEN_PASS_DEF_MYPASS
#include "Dialect/MyDialect/Passes.h.inc"

struct MyPass : public impl::MyPassBase<MyPass> {
    using MyPassBase::MyPassBase;
    void runOnOperation() override {
        auto module = cast<mlir::ModuleOp>(getOperation());
        // ...
    }
};
} // namespace qoala::analysis
```

The pass is registered automatically when `tools/qoala-opt/qoala-opt.cpp` calls the generated `register*Passes()` for the owning dialect. If you add a new dialect, also add the corresponding `qoala::analysis::register*Passes()` call there:

```cpp
    // And also the passes from QMem, QNet and QoalaHost
    qoala::analysis::registerQNetPasses();
    qoala::analysis::registerMIRToLIRHelpersPasses();
    qoala::analysis::registerQoalaHostPasses();
    // Register all the passes from MyDialect
    qoala::analysis::registerMyDialectPasses();
```

### Conversion passes

Conversions follow the same shape (a TableGen `def`, a generated header, a C++ implementation), but live under `include/Conversion/<From>To<To>/` and `lib/Conversion/<From>To<To>/`. See `include/Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.td` for an example. The implementation typically uses `mlir::ConversionTarget`, `RewritePatternSet`, and `applyPartialConversion` / `applyFullConversion`.

## Defining a dialect, ops, and types with TableGen

The qoala-mlir dialects use the conventional MLIR ODS layout. For a new dialect `Foo`:

```
include/Dialect/Foo/
├── Foo.td            # entry point: includes the others
├── FooDialect.td     # dialect declaration
├── FooOps.td         # op definitions
├── FooTypes.td       # type definitions (if you have custom types)
├── Foo.h
├── FooDialect.h
└── CMakeLists.txt
```

`Foo.td` looks like:

```tablegen
include "mlir/IR/OpBase.td"
include "Dialect/Foo/FooDialect.td"
include "Dialect/Foo/FooTypes.td"
include "Dialect/Foo/FooOps.td"
```

The dialect is declared in `FooDialect.td`:

```tablegen
def Foo_Dialect : Dialect {
    let name = "foo";
    let cppNamespace = "qoala::dialects::foo";
}
```

Ops follow the standard ODS form:

```tablegen
class Foo_Op<string mnemonic, list<Trait> traits = []> :
    Op<Foo_Dialect, mnemonic, traits>;

def MyOp : Foo_Op<"my_op", [Pure]> {
    let summary = "Short summary.";
    let arguments = (ins I32:$x, F32:$angle);
    let results = (outs I32:$result);
    let assemblyFormat = "operands attr-dict `:` type($result)";
}
```

The CMake `add_mlir_dialect(Foo foo)` invocation wires up TableGen for ops, types, and the dialect class. After it runs, the generated `*.h.inc` and `*.cpp.inc` files appear in `build/include/Dialect/Foo/`. Source files generated under `lib/Dialect/Foo/` need to be `#include`d with the standard `GEN_OP_CLASSES` / `GEN_TYPEDEF_CLASSES` macros:

```cpp
#define GET_OP_CLASSES
#include "Dialect/Foo/Foo.h.inc"
```

### Op naming convention

`mlir-tblgen` strips a leading `Prefix_` from the TableGen `def` name when forming the C++ class name. `def MyOp` becomes `class MyOp`. `def Foo_MyOp` becomes `class MyOp` — the `Foo_` prefix is dropped. `def Foo_Bar_MyOp` becomes `class Bar_MyOp` — only the *first* prefix is dropped. The qoala dialects all use the simple `Foo_Op<"…">` mixin form, which avoids the second-prefix case.

### Custom types

The TableGen-generated Python op bindings emit *operations only* — types must be registered manually from C++. See `lib/Python/QNetExtension.cpp` for the pattern used to expose `!qnet.qubit` to the Python bindings.

## Useful CMake helpers

| Helper | What it does |
| --- | --- |
| `add_mlir_dialect(Foo foo)` | Generates op decls/defs, type decls/defs, and dialect decls/defs from the corresponding `.td` files. Outputs into `build/include/Dialect/Foo/`. Adds target `MLIRFooIncGen`. |
| `add_mlir_dialect_library(MLIRFoo …)` | Builds `libMLIRFoo.a` under `build/lib/`. Implicitly depends on `MLIRFooIncGen`. |
| `add_mlir_conversion_library(MLIRFooToBar …)` | Same shape as `add_mlir_dialect_library` but for a conversion pass. |
| `mlir_tablegen(Out.h.inc -gen-pass-decls -name Foo)` | Generates pass-declaration boilerplate from a `Passes.td`. |

Each `add_mlir_dialect_library` should live in its own subdirectory with its own `CMakeLists.txt` — the qoala-mlir codebase already follows this layout throughout.

## ODS quick reference

| ODS construct | Purpose |
| --- | --- |
| `Op<Dialect, mnemonic, [traits]>` | Op definition. |
| `let arguments = (ins …)` | Op operands and attributes. Use `Variadic<Type>` for variadic, `Optional<Type>` for optional. |
| `let results = (outs …)` | Op results. Same syntax. |
| `let assemblyFormat = "…"` | Custom textual assembly. Use `operands`, `attr-dict`, `type($x)`, `functional-type($args, results)`. |
| `let hasCustomAssemblyFormat = 1` | Marks this operation as having a custom assembly format. Doing so requires implementing `parse`/`print` method in C++. |
| `let hasVerifier = 1` | Marks this operation as having a verifier. Doing so requires implementing `LogicalResult verify()` method in C++. |
| `let hasRegionVerifier = 1` | Only used in operations that have a nested region. Marks this operation as having a verifier. Doing so requires implementing `LogicalResult verifyRegions()` method in C++. |
| `Arg<Type, "name", [MemRead/MemWrite]>` | Operand with a memory-effect annotation (used widely in QMem to say "this op writes the qubit slot"). |
| `DeclareOpInterfaceMethods<Iface, ["m1", "m2"]>` | Pull in interface methods that you implement in C++. |
| `let dependentDialects = [...]` (on a Pass) | Dialects this pass produces ops in or otherwise needs registered. |

For the canonical list, check the TableGen files `llvm/mlir/include/mlir/IR/OpBase.td` and `BuiltinOps.td` located inside the LLVM submodule tree.

## Common traits used in qoala-mlir

| Trait | Effect |
| --- | --- |
| `NoTerminator` | Region needs no terminator. |
| `SingleBlock` | Region must contain exactly one block. |
| `IsolatedFromAbove` | Op cannot reference SSA values from outside the region. |
| `Symbol` | Op declares a name visible in a symbol table. |
| `FunctionOpInterface` | Op behaves like a function: needs `function_type`, `sym_name`, `sym_visibility?`. |
| `Pure`, `MemRead`, `MemWrite` | Memory-effect declarations. |
| `Terminator` | Op terminates a block. Used by `qoalahost.call`, `qoalahost.recv_*`, `qoalahost.nop_term` to enforce one-call/recv-per-block. |
| `HasParent<"FuncOp">` | Op is only valid inside a specific parent op. |
| `ParentOneOf<["A", "B"]>` | Op is only valid inside one of several parent op kinds. |
| `HermitianOpIface` (qoala) | The op is its own inverse; the peephole optimizer cancels `op op` pairs. |
| `RotationOpIface` (qoala) | The op is a rotation with a numeric angle, so `qnet-peephole-optimizations` can fold consecutive same-axis rotations. |

## Useful upstream files

When in doubt about a TableGen primitive, the upstream LLVM/MLIR sources are the cleanest reference. `llvm/mlir/include/mlir/IR/OpBase.td` defines the `Op`, `Trait`, and `MemoryEffects` building blocks. `llvm/mlir/include/mlir/IR/BuiltinOps.td` defines `module` and `unrealized_conversion_cast`. `llvm/mlir/include/mlir/IR/AttrTypeBase.td` defines `TypeDef` and `AttrDef`. And `llvm/mlir/include/mlir/Pass/PassBase.td` defines `Pass` and `Option`.

## Patterns and rewrites

Most rewrites in qoala-mlir use the standard greedy pattern driver. Inside a pass:

```cpp
RewritePatternSet patterns(&getContext());
patterns.add<MyPattern>(&getContext());
if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
    signalPassFailure();
```

The method `applyPatternsGreedily` was backported from newer LLVM versions because the one available in the pinned LLVM commit (`applyPatternsAndFoldGreedily`) also applies constant folding with no option to disable that last optimization. In some places in this codebase we need to apply rewrite patterns without folding constants, which is what motivated the backport. The full implementation lives in `lib/Analysis/Helpers/PatternRewriteDriver.cpp`.

Patterns inherit from `OpRewritePattern<MyOp>` and override `matchAndRewrite`. For lowerings, the standard combo is `OpConversionPattern<MyOp>` plus a `ConversionTarget`.

## Debugging your pass

The cheapest way to inspect pass behavior is `LLVM_DEBUG(llvm::dbgs() << ...)` statements; toggle them on with `qoala-opt --debug`, or scope them with `--debug-only=<DEBUG_TYPE>` after defining `#define DEBUG_TYPE "my-tag"` at the top of the source file. `--print-ir-before=<my-pass>` and `--print-ir-after=<my-pass>` dump the IR around the pass. If your custom verifier recurses through a custom printer, `--mlir-print-op-generic` and `--mlir-print-assume-verified` break the loop. And for deterministic debug output across multi-threaded pipelines, `--mlir-disable-threading` serializes pass execution. The full list of these knobs is in [tools/qoala-opt](../tools/qoala-opt.md#standard-mlir-knobs).
