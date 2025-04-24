# Beginner's Guide for developing with MLIR dialects

## Developing in MLIR

### Defining passes

MLIR passes can be defined using tablegen in a `.td` file. To do so, the bare minimum code
that define the pass is:
```mlir
include "mlir/Pass/PassBase.td"

# First arg for Pass is the string for the CLI tool to invoke this pass
# Second arg is the operation type on this this pass can run on.
def MyPass : Pass<"lower-qoala-hir-to-mir", "mlir::ModuleOp"> {
  let summary = "Shor description of my pass";
  let description = [{
    Some more extended description of the pass that you want to implement.
    This description can span across
    multiple lines.
  }];
  // We list the fully qualified names of the dependent dialects.
  // List here all the dialect that this pass uses, i.e. either process operations
  // in these dialects *or* creates operations on these dialects.
  // The "BuiltinDialect" is listed "for free; no need to list it here again
  let dependentDialects = ["::mlir::func::FuncDialect", "::mlir::arith::ArithDialect"];
}
```
This tablegen code needs to be processed to generate all the base classes. To do so, we use a
CMake file that invokes tablegen to generate the code:
```cmake
set(LLVM_TARGET_DEFINITIONS MyPass.td)
mlir_tablegen(MyPass.h.inc -gen-pass-decls -name MyPass)
add_public_tablegen_target(MLIRMyPassIncGen)
```

This creates a bunch of code, in particular the `impl::MyPassBase` class. This is the class
to extend when implementing. The generated header need to be included on a new header:
```c++
// We also need to include the dialect header of the dependent dialects
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

#include "mlir/Pass/Pass.h"

// To declare a function named createMyPass() to create the pass instance
#define GEN_PASS_DECL
#include "Folder/MyPass.h.inc"

// Generate the code for registering passes in the `mlir-opt` tool
#define GEN_PASS_REGISTRATION
#include "Folder/MyPass.h.inc"
```

### Implementing passes

To implement the declared pass, we use the generated header to attach the definition of
the base pass class `impl::MyPassBase`. We then create a class inheriting from the base
class:
```c++
#include "Folder/MyPass.h"
#include "mlir/IR/BuiltinOps.h"

// Include the code that defines impl::MyPassBase class, and the code that
// implements (defines) the `createMyPass` function.
#define GEN_PASS_DEF_MYPASS
#include "Dialect/QMem/Passes.h.inc"

class MyPassClass : public impl::MyPassBase<MyPassClass> {
public:
    using MyPassBase::MyPassBase;
    // We are *required* to override the `runOpOperation` function
    void runOnOperation() override;
};

// Provide an implementation for the `runOnOperation` function
void QMemSimpleFunctionizePass::runOnOperation() {
    // It is safe to perform this dyn_cast, since we declared that
    // the pass runs on operations of type ModuleOp
    ModuleOp module = dyn_cast<mlir::ModuleOp>(getOperation());
    // Do stuff
}
```

The file can then be included in another Cmake target (mlir library, conversion library, etc).
Please don't forget that this target will now depend on the `MLIRMyPassIncGen` target, since we
need to generate the pass code form tablegen before compiling the implementation.


### Conversion passes

Converting one dialect into another (lowering) is implemented using MLIR passes. This being said,
you can follow the same section above to implement your conversion pass.


## CMake targets available for MLIR projects

### `add_mlir_dialect`

`add_mlir_dialect(Toy toy)` adds the target `MLIRToyIncGen`
This target does the following:
- `mlir-tblgen -gen-op-decls`, producing `Toy.h.inc` (contains the Ops classes declarations)
- `mlir-tblgen -gen-op-defs`, producing `Toy.cpp.inc` (contains the Ops classes definitions)
- `mlir-tblgen -gen-typedef-decls`, producing `ToyTypes.h.inc` (contains the dialect's *types* classes declarations)
- `mlir-tblgen -gen-typedef-defs`, producing `ToyTypes.cpp.inc` (contans the dialect's *types* classes definitions)
- `mlir-tblgen -gen-dialect-decls`, producing `ToyDialect.h.inc` (contains the declaration of the dialect class)
- `mlir-tblgen -gen-dialect-defs`, producing `ToyDialect.cpp.inc` (contains the definition of the dialect class)

All `.inc` files appear in `build/include/Dialect/toy` (assuming the `.td` files
are in `include/Dialect/toy`).

Every `mlir-tblgen` run takes `Toy.td` as input. This `Toy.td` file should therefore
`include` all other relevant `.td` files (e.g. `ToyDialect.td`).

Despite `mlir-tblgen` generates declarations for the operations, this code (the `.h.inc` file)
*must be included* (#include) in a header file which usually resides next to the `.td` file.


### `add_mlir_dialect_library`

`add_mlir_dialect_library(MLIRToy)` adds the target `MLIRToy`.
It produces the library `libMLIRToy.a` inside `build/lib`.
It automagically depends on the target 'MLIRToyIncGen` (see the section above).

For targets in `lib/`, each `add_mlir_dialect_library` should have its own
`CMakeLists.txt`, and therefore in its own directory.
So, a separate Transforms library would require a separate
`Transforms` directory inside `lib/Dialect/toy`, with its own `CMakeLists.txt`.

In general, all the code generated by `mlir-tblgen` from the ops definition (the `.cpp.inc`
file) *must be included* (#include) in a source file that is part of a target defined with
`add_mlir_dialect_library`.

### `add_mlir_conversion_library`

`add_mlir_conversion_library(MLIRToy)` adds the target `MLIRToy`.
It produces the library `libMLIRToy.a` inside `build/lib`.

A conversion library is implemented as an MLIR pass. This means that there should also
be a tablegen file (`.td`) that defines the pass.


### `add_mlir_translation_library`

TODO


### `add_mlir_library`

TODO


## Operation Definition Specification - ODS
Operations are defined in _Operation Definition Specification_ (ODS) language, in a `.td` file).
Running the `mlir-tblgen` commands (using the `add_mlir_dialect` function) generate the following
C++ class names:
- `MyNameOp` -> `class MyNameOp`
- `Idk_MyNameOp` -> `class MyNameOp` (!!)
- `Idk_Wtf_MyNameOp` -> `class Wtf_MyNameOp`
In other words: if the name starts with `XXX_`, this part is omitted!

## Traits

Traits are special "labels" that can give operations certain properties or constraints.
When defining an operation, we can use traits to attach the related properties to the
defined operation.

Some of the most common traits are (this list is not exhaustive). 
- `NoTerminator` (from `GraphRegionNoTerminator` list): region does not have a terminator
- `SingleBlock` (from `GraphRegionNoTerminator` list): region must have exactly 1 block
-
- `FunctionOpInterface`: means
  - single region
  - must have ODS `function_type` TypeAttr (used by `getFunctionType()`)
  - must have ODS `sym_name` StringAttr (used by `getSymbolAttrName()`)
  - must have ODS `sym_visibility` StringAttr


## Writing programs in MLIR

### General Syntax
An operation can be written in the generic as:
```mlir
"dialect.opname"() ([operands]) : ([operand_types]) : ([results_types])
```
If the operation receives a single operand (or yields a single result), the parenthesis
`()` on the types can be omitted.

Some operations can have a nested region, which makes its writing a bit more complex:
```mlir
"dialect.opname"() ( { <blocks> } ) : <function-type>
```
where the `()` indicate a list of regions, and each region is indicated by `{}`.

A region `{}` consists of blocks `^b:`.
A region `{}` may start directly with operations: they are in an implicit entry block.

A block `^b:` can have arguments, written as e.g. `^b(%a: i32):`

Operation with a successor block (e.g. `cf.br`):
```mlir
"dialect.opname"() [^b1] : <function-type>
```


### The "root" operation: Module

- Dialect: `Builtin_Dialect`, name: `builtin`
- Full Op name: `builtin.module`
- A Module should be the top-most operation, acting as a container for all other operations
  such as functions (operation `func.func`) or others.


### Function

- Dialect: `Func_Dialect`, name: `func`
- Full Op name: `func.func`


## Comparing values
- Dialect: `Arithmetic_Dialect`, name: `arith`
- Full Op name: `arith.cmpi` or `arith.cmpf`
- Compares 2 operands and returns a test result.
- The "i" version compares 2 integers, whereas the "f" version compares floats.
- It also uses an attribute, which is the type of test to perform (how to compare), e.g.:
  - `eq`: compare if equals
  - `slt`: compare *signed integers* if less than
  - `uge`: compare *unsigned integers* is greater or equal than
- A full list of these comparison attributes can be found at [Arith Dialect Documentation](https://mlir.llvm.org/docs/Dialects/ArithOps/#arithcmpi-arithcmpiop)


### Branch

- Dialect: `ControlFlow_Dialect`, name: `cf`
- Full Op name: `cf.br`
- This operation represents an unconditional jump to a given block.


## Conditional Branch

- Dialect: `ControlFlow_Dialect`, name: `cf`
- Full Op name: `cf.cond_br`
- This operation uses 3 operands. The semantics for this operation is a bit limited; it will
  branch to the true destination if the condition (first operand), otherwise, it will branch to
  the false destination.
- First, we specify the branch condition. This is an operand that will be interpreted as false
  (operand evaluates to 0) of true (operand evaluates to anything else).
- The second and third operands are the branching destinations; second operand is the destination
  if condition evaluates to true, third operand is the destination if condition evaluates to false.
- To emulate the behavior of x86 conditional branching (eg bne, bgt) we need to use this operation
  in coordination with the arithmetic comparison.
- A fully functional example to emulate a "branch on greater than" should look like this:
```mlir
// %arg0 and %arg1 are i32 values.
%test = arith.cmpi gt %arg0, %arg1 : i32
cf.cond_br %test, ^bb1, ^bb2
```


## Tools

`mlir-tblgen` is like a compiler that takes input source files (`.td` files) and also requires you
to set include directories with `-I`. It's in `{llvm_build}/bin/` (built) or `/usr/local/bin` (installed).


The default `mlir-cpu-runner` in `llvm/build/bin` does not register any dialects.
`tools/interpreter` contains a copy of the source code of `mlir-cpu-runner` but does register all dialects.
It can interpret operations in the LLVM Dialect (`llvm.mlir`) directly.
Other ops may or may not be valid, depending on whether there is a translation interface implemented for it.


## Using the headers generated by MLIR

Be careful when `#include`ing `.h.inc` files. You may or may not want to
`#define` a certain variable just before it. For example, you might need to use

```
#define GET_OP_CLASSES
#include "Dialect/toy/Toy.h.inc"
```

instead of just `#include "Dialect/toy/Toy.h.inc"`.


## Passes

Using `PassManager`s or `PassRegistration` doesn't seem to do anything.
The following does work: use 

```
mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass>
                       { return createXXXPass(); });
```

Creating a `std::unique_ptr` inside the lambda doesn't seem to work; it's better to
declare a `createXXXPass` function in `MyTransforms.h` which returns a `std::unique_ptr<Pass>`.


## Tranformations
Inside a pass, one may call `applyPatternsAndFoldGreedily()` with a set of `Pattern`s.


### Type vs TypeDef
`Type` defined in `CommonTypeConstraints.td`, `TypeDef` defined in `AttrTypeBase.td`.
Apparently `TypeDef` should be used to create custom types.

### Useful llvm source files:
- `llvm/mlir/include/mlir/IR/OpBase.td`
- `llvm/mlir/include/mlir/IR/BuiltinOps.td`
- `llvm/mlir/include/mlir/IR/AttrTypeBase.td`
- `llvm/mlir/include/mlir/IR/Types.h`



### Misc
`op-result-list` cannot be surrounded by parentheses!
So you cannot write `(%0, %1) = <op> ...` but it must be `%0, %1 = <op> ...`

To print the generic (non-custom) format of MLIR operations, pass `--mlir-print-op-generic` to `mlir-opt`.