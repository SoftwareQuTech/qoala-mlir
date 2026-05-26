# Functionize algorithm

## Introduction

As part of the lowering from Qoala Middle to Lower Intermediate Representation (MIR to LIR), the program undergoes a major reorganization: all quantum instructions that live inside the `main` function need to be grouped according to a set of rules and moved into their own functions. Those function wrappers are then called back from the main body to preserve the program's logic. This reorganization is called *functionization*, and it is necessary to prepare the translation unit before generating the final assembly in the iQoala format.

## Hands-on description of the algorithm

At a "hands-on" level, functionization selects which operations belong to a group and then places each group into a separate function. Two rules drive the grouping:

- **Contiguous quantum-operation groups.** Sequences of QMem operations that act on the same logical qubit (or set of qubits) are accumulated into the same group. Once certain conditions are met (resource boundary, group-size limit, or barrier — see below), the group is considered complete and its operations are placed into a separate function, with their original locations in the main body replaced by a single `call` to that function.

- **Classical communication as barriers.** Operations that perform classical communication or receive data from the network (e.g.\ `qmem.recv_*`) act as *barriers*: they terminate any currently-open group and stay in the main body. This keeps classical-communication semantics observable in the host program, where the Qoala runtime expects them, instead of burying them inside a quantum routine.

One invariant is enforced unconditionally on top of these rules: a logical-qubit allocation and its corresponding initialization (either a local `init` or an entanglement-generating operation) are always placed in the same group and are never separated by the classifier. The size of the resulting groups is otherwise configurable through a tunable upper bound on the number of quantum operations per group, which the [compiler paper](<PAPER_URL>) refers to as the selfish-vs-cooperative compilation knob.

From this description we can already identify the two big steps of functionization: first, classify the operations to form the groups; second, for each group, place all of its instructions into its own function definition within the same translation unit. Before going deeper into either step we need to introduce a few concepts.

## Definitions

### Qubit lifecycle

In the QMem dialect we start seeing more low-level quantum instructions, meaning that the qubit's lifecycle becomes observable. Most quantum programs use qubits in a stereotyped way: allocate the qubit (with `qalloc`); then initialize it (`init`), create entanglement (`eprs`), or immediately measure an entanglement measure (`eprs_measure`); then perform some gates on the qubit; then measure it with `measure` or `eprs_measure`. Once the qubit has been measured, no more operations can usefully be applied to it without going through the lifecycle again from the start.

These four observable steps decompose into three main stages: qubit allocation and initialization (the first two steps, which *always* appear together — "there is no `qalloc` without its respective `init`"), operations on the qubit (the third step, a set of quantum operations performed on the qubit), and measurement (the fourth step, which ends the qubit's life). We refer to these three stages later to justify certain assumptions made by the grouping algorithm.

### MLIR operations "closure"

The functionization process described above produces groups of operations that are placed in isolated functions within the same translation unit. We refer to each such group as an *operations closure*, by analogy with the concept of [function closure](https://en.wikipedia.org/wiki/Closure_(computer_programming)).

In this document, we define an operations closure (or simply a *closure*) as a group of operations that *isolate the operations applied to certain (quantum) values*. This definition does not restrict the isolated operations to use values from or create values that are used outside the enclosed set.

Being this said, it is immediate that an operations closure can use a set of values produced *outside* the group (its *external arguments*) and may produce a set of values that are used *outside* the same group (its *external results*). Similarly, it may also use values produced *by operations within the group* (its *internal arguments*), and may produce values *used by other operations within the group* (its *internal results*). These concepts come back in the explanation of how the function definition is built for a given set of operations.

## High level design of the functionization algorithm

The method `qoala::analysis::functionize::functionizeModule` in `lib/Analysis/QMem/Functionize.cpp` is the main entry point of the algorithm. It takes the MLIR module that contains the main function to process, and a `classifyOperations` function whose responsibility is to walk all the operations inside the main function (a `qmem.func` operation) and produce a set of operation groups. The correct signature of this classifier function (`ClassifierFnTy`) and the type of the operations group (`QuantumOpsGroupTy`) are declared in `include/Analysis/QMem/Conversion.h`.

`functionizeModule` itself sketches the high-level steps of functionization. It first creates the operation groups by calling the classifier. Then, for each operation group, it (1) declares a `FunctionizeData` structure that will hold the value-mapping data; (2) creates a new function with the operations of the group, populating the `FunctionizeData` struct in the process; (3) looks up the symbol (name) of the newly-created function; (4) inserts a `call` operation to that function, using the data from `FunctionizeData` to get the correct argument list; (5) replaces the values generated by the operations in the group with the values generated by the `call` op, using the functionize data to map original external values to the index of the corresponding result of the `call`. To avoid breaking data dependencies — for example, operations in the group that use data from operations "in the middle" of the group — the `call` operation is placed *right after the last operation of the group*, which effectively moves all the data dependencies up to just before the call. The final step is then (6) deleting the original operations from the original function body.

The next sections go deeper into step 2, which carries most of the complexity when functionizing an operation group.

## Creation of a new function in the module

The implementation of how to create a new function from a given group of operations lives in `qoala::analysis::functionize::createNewFunctionWithOperations` in `lib/Analysis/QMem/Functionize.cpp`. Among its arguments it takes the `FunctionizeData` instance that records the mapping data needed by `functionizeModule`, an `OpBuilder` used to create new operations and clone the ones in the group, and an ordered set containing the operations of the current group. The order of that set establishes the order in which the operations are inserted into the new body.

It is worth remarking that, despite the appearance of "simply moving" operations from the main body to the body of a new function, moving an operation outside its scope is not easy: doing so effectively breaks the data dependencies within the original function — the values used by the moved operation, and the operations that use values from the moved operation, would all need to move with it. `createNewFunctionWithOperations` solves this by *partially cloning* the original operations.

To this end, the QMem operations implement the `qoala::helpers::SimpleCloneInterface` declared in `include/Analysis/Helpers/SimpleCloneInterface.td`. Any operation implementing this interface — that is, every QMem operation, see the `QMem_Op` class definition in `include/Dialect/QMem/QMemOps.td` — is forced to implement the `simpleClone` method, which creates a new operation with the same operands and attributes as the original, but unlinked from any function block and not used (yet) by any other function. After simple-cloning an operation, `createNewFunctionWithOperations` updates the operands to use values that come from operations *within the new body*.

To help build the cloned operations in the new function body, `createNewFunctionWithOperations` relies on the concept of operations closure. The method keeps two maps between original operation results and cloned operation results: one for the *internal* results and one for the *external* results of the new function. A map of *external* results (values) is additionally used to populate the `FunctionizeData` field that correlates each original external result with the index of the corresponding value in the new function's return statement. Naturally, these indexes match one-to-one with the indexes of the return values on the `call` operation placed in the main body by `functionizeModule`.

`createNewFunctionWithOperations` achieves all of this in six steps. First, it computes the argument and return types of the given operation set by calling `computeArgTypesAndReturns`, whose results are placed in the `FunctionizeData` struct. Second, it creates a new function declaration (`func.func` operation) using the argument and result types collected in step one. Third, it creates a new `OpBuilder` whose insertion point is the start of the body block of the newly-created function. Fourth, for each operation in the given set, it simple-clones the operation, computes the new operands of the clone (via `mapOriginalOperandsInClonedOp`, which uses the external-values info collected earlier together with a map from original internal results to new internal results — and because the operations are iterated in the order they appeared in the original body, the corresponding mapped value always exists by the time it is needed), processes the clone's results (`mapOriginalResultsInClonedOp`, which updates both the internal and external result maps), and updates the operands of the new operation. Fifth, it uses the external-operations map to identify the values that need to be operands of the `return` statement and to correlate the original values with their index in that return statement. Sixth, it creates the `return` and sets its operands accordingly.

The next section goes a bit deeper on step one — the `computeArgTypesAndReturns` method, which discovers the *external* arguments and results (return values) and their types of a given operations closure.

## Function type discovery

The final level of nesting in the functionization algorithm is the discovery of the external results and values (and their types) of a given operations closure. This is implemented by `computeArgTypesAndReturns` in `lib/Analysis/QMem/Functionize.cpp`, and is where the *closure* concept becomes most relevant.

`computeArgTypesAndReturns` takes a `FunctionizeData` structure that will be populated with the external arguments and results together with their types, and an ordered set of the operations to analyze. The discovery algorithm iterates over each operation and, for each one, processes operands first and then results. For each operand, it checks whether the operation that defines the operand is *outside* the operations set and whether the value was not already discovered as coming from outside the group; if both conditions hold, the operand (MLIR value) is an external argument, and the algorithm records how it will be passed to the new function (mapping the original value to its position in the argument list) and adds the value and type to the external-values and external-types sets. For each result, the algorithm checks whether the result has any uses: if it has none, it is still marked as a return value with its type; if it has uses, the algorithm iterates over the user operations and, for any user that is not in the operations closure, records the result as an external result (taking care, via the result's index in the current operation, not to double-count a value that is used by multiple outside operations). Finally, it populates the `FunctionizeData` structure with the discovered external arguments and result values together with their types.

## Known issues (Fixed)

A few known issues remain in the functionization implementation, each tracked to the commit that fixed it.

### Deletion of original operations in wrong order

During testing we discovered that deleting the original operations in their textual order led to a broken data dependency in the middle of the deletion process. For example, given:

```mlir
%value = dialect.operation_A %value_one, %value_two ;; operation1
dialect.operation_B %value ;; operation2
```

if `dialect.operation_A` is deleted first, MLIR complains immediately after the deletion because `dialect.operation_B` is still on the body and uses a value from an operation that no longer exists. The fix, in commit `f5326817d7cd15bb3c221e724e1e4c6ff869150c`, is to delete the operations in reverse order.

### Insertion of new (cloned) operations in the wrong order

When creating the clones, they were inserted in reverse order and the return statement was placed in second-to-last position. This led to MLIR failing the validation of the return operation:

```mlir
func.func @__qoala_wrapper() -> i32 {
   "qoala.init"(%0) : (i32) -> ()
   "func.return"(%0%) : () -> i32
   %0 = "qoala.qalloc"() : () -> i32
}
...
error: 'netqasm.return' op must be the last operation in the parent block
```

The fix, in commit `dbdceddf176ab702b2439cb9090c424c38eda9f7`, is to set the `OpBuilder` insertion point only once, before the loop that iterates over the operations to be cloned.

### Incorrect handling of the function arguments when using multiple rotations

Consider the following Qoala MIR:

```mlir
module {
  qmem.func @test_local_quantum_program() {
    %0 = qmem.qalloc : i32
    qmem.init %0
    %1 = qmem.qalloc : i32
    qmem.init %1
    %cst = arith.constant 2.120000e+01 : f32
    %cst_0 = arith.constant 0.0306796152 : f32
    %cst_1 = arith.constant 1.050000e+01 : f32
    %cst_2 = arith.constant 2.710000e+00 : f32
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1
    qmem.return
  }
}
```

When lowering this MIR to LIR, `opt` complains in a very weird way:

```
file.mlir: error: operand #3 does not dominate this use
    qmem.rot_x %0, %cst_0
    ^
file.mlir: note: see current operation: %6 = "qoalahost.call"(%0, %5#0, %5#1, %7#0, %7#1) <{callee = @__qoala_wrapper2}> : (i32, i32, i32, i32, i32) -> i1
file.mlir: note: operand defined here (op in the same block)
    qmem.rot_y %0, %cst_1
    ^
```

Turning on the debug printing of the functionize internals (`--debug-only=mir-to-lir,functionize-internal,op-classifier`) gives some hints. The classifier function recognizes the operation groups correctly:

```
%%%%%%%%%%%%%%%%%%%%%%%%
%      CLASSIFIER      %
%%%%%%%%%%%%%%%%%%%%%%%%
 - New Group #0 :
   - op: %0 = qmem.qalloc : i32
   - op: qmem.init %0
 - New Group #1 :
   - op: %1 = qmem.qalloc : i32
   - op: qmem.init %1
 - New Group #2 :
   - op: qmem.rot_x_int %0, %2#0, %2#1
   - op: qmem.rot_y_int %0, %3#0, %3#1
   - op: %4 = qmem.measure %0 : i1
```

But the processing of group #2 is not correct:

```
------------------------
Process group #2
------------------------
Discovered func type: (i32, i32, i32, i32, i32, i32, i32) -> i1
------------------------
Cloning ops:
 - original op: qmem.rot_x_int %0, %1#0, %1#1
 * cloned op: "qmem.rot_x_int"(%arg0, %arg1, %arg2) : (i32, i32, i32) -> ()
 - original op: qmem.rot_y_int %0, %2#0, %2#1
 * cloned op: "qmem.rot_y_int"(%arg0, %arg1, %arg2) : (i32, i32, i32) -> ()
 - original op: %3 = qmem.measure %0 : i1
 * cloned op: %0 = "qmem.measure"(%arg0) : (i32) -> i1
------------------------
New Function: ****** CREATED FUNCTION BODY ******
func.func @__qoala_wrapper2(%arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32, %arg4: i32, %arg5: i32, %arg6: i32) -> i1 {
  qmem.rot_x_int %arg0, %arg1, %arg2
  qmem.rot_y_int %arg0, %arg1, %arg2
  %0 = qmem.measure %arg0 : i1
  return %0 : i1
}
Has entanglement: No
Return op:
 - func.return %0 : i1
------------------------
Replacing operations in group #2:
 -> "qmem.rot_x_int"(%0, %5#0, %5#1) : (i32, i32, i32) -> ()
 -> "qmem.rot_y_int"(%0, %7#0, %7#1) : (i32, i32, i32) -> ()
 -> %8 = "qmem.measure"(%0) : (i32) -> i1
With:
 * %6 = "func.call"(%0, %5#0, %5#1, %7#0, %7#1) <{callee = @__qoala_wrapper2}> : (i32, i32, i32, i32, i32) -> i1
```

Two observations come out of this trace. First, the discovery method does not correctly deduct the number of external arguments of the group: for group #2 we expected five discovered arguments, but it discovers seven — the discovery method does not recognize that `%0` is the same argument used in the three functionized operations, so it adds it three times (once per usage in the closure). Second, the created function body shows all the arguments but the operations only use the first three, hinting that the matching between arguments and values is also incorrect.

Investigation surfaced three issues. First, when discovering arguments and values, the algorithm did not check whether the argument was already discovered, so an argument could be recorded more than once whenever an operation used the same qubit several times. Commit `3b62033921a32981542786bd4f2baecd7aa41d72` fixes this by keeping track of the discovered argument values. Second, once the arguments were discovered, the algorithm updated the operands of the cloned operation by taking the operand index on the original operation and replacing the value with the function argument *at the same index* — an assumption that does not hold when the group contains more than one operation. Commit `c8361dcc71db03e00c00286c483ad3d4c94a0c12` fixes this by tracking the original values and their mapping to the argument index in the new function. Third, the `call` operation that replaced the functionized operations was being inserted right after the *first* operation of the group; consider this fragment:

```mlir
qmem.rot_x %qubit, %cst_0, %cst_1
%cst_2 = arith.constant 2 : i32
qmem.rot_y %qubit, %cst_0, %cst_2
```

If both rotations go in the same group, the `call` operation gets placed right after the first one, yielding:

```mlir
func.func @__qoala_wrapper(%arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32) {
  qmem.rot_x %arg0, %arg1, %arg2
  qmem.rot_y %arg0, %arg1, %arg3
}

func.call @__qoala_wrapper(%qubit, %cst_0, %cst_1, %cst_2) ;; forward reference!
%cst_2 = arith.constant 2 : i32
```

The generated `call` contains a forward reference to `%cst_2`, which breaks the data dependency graph. Commit `618f521416021370316ee3473cb1c4bf58e40a06` fixes this by placing the `call` operation right after the *last* operation of the group:

```mlir
func.func @__qoala_wrapper(%arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32) {
  qmem.rot_x %arg0, %arg1, %arg2
  qmem.rot_y %arg0, %arg1, %arg3
}

%cst_2 = arith.constant 2 : i32
func.call @__qoala_wrapper(%qubit, %cst_0, %cst_1, %cst_2) ;; %cst_2 reference is no forward anymore.
```
