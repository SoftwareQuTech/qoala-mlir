# Functionize algorithm


## Introduction

As part of the lowering from Qoala Middle to Lower Intermedia Representation (MIR to HIR), the program operations
need to overcome in a major reorganization. In particular, all quantum instructions that live inside the "main"
function need to be according to certain rules and places inside their own functions. These function wrappers will
be called from the main body of the program to maintain the logic of the program.
This reorganization of the quantum operations is called _functionization_, and it is necessary to prepare the
translation unit before generating the final assembly in the iQoala format.

## Hands-on description of the algorithm

The `qoala-compiler-specs` repository provides a "hands-on" description about how the functionization algorithm
must select the operations that belongs to a group, and place them in a separate function.
This description does not provide a full specification about the algorithm, but it helps to understand how the
operations are grouped together and how classical communications acts as _barriers_ that define the end of a
partilly-filled quantum operations group.

Despite this, the compiler specification limits the description only to how the operations are classified into
an operations group. In this description, once certain  conditions are met, the group is considered "complete"
and the operations are "placed into a separate function, replacing the original locations of the functionized
operations with the respective `call` operation".

From this description, we can identify the first 2 big steps of the functionzation:

1. Classify the operations with the objective of forming the groups.
2. For each of the groups, place all the instructions in its own function definition (within the same translation
   unit).

Before we proceed with the detailed explanation of each of the aforementioned steps, we need to introduce a few
concepts.

## Definitions

### Qubit lifecycle

In the QMem dialect, we start seeing more "low level" quantum instructions, meaning that we can also observe what
is the qubit lifecycle.

In general, we could observe that any quantum program using qubits would use them in a very similar fashion:

1. Allocate a qubit using the `qalloc` operation.
2. Initialize the qubit (using the `init` operation), create entanglement (using the `eprs` operation) _or_
   immediately measure an entanglement measure (using the `eprs_measure` operation).
3. Perform a set of gates on the qubit.
4. Measure the state of teh qubit using either the `measure` or the `eprs_measure` operations.

After measuring the qubit, it is no longer useful to apply more operations on the same qubit without going through 
the lifecycle once again.

By observing the set of steps we can identify three main stages: 
1. Qubit allocation and initialization: Steps 1 and 2; these two steps _always_ appear in that order when using a
   qubit, which means that "there is no `qalloc` without its respective init",
2. Operations on the qubit: Step 3; these are just a set of quantum operations performed on the qubit, and 
3. Measure the result: Step 4; putting an end t the life of the qubit value, and an end to the lifecycle.

We will use the observations of the three main stages in the later sections ot explain certain assumptions when
implementing the grouping algorithm.


### MLIR operations "closure"

The funcitonization process shown in the `qoala-compiler-specs` describes how to form a group of operations that
will be placed in an isolated function within the same translation unit. When moving these operations to it own
function declaration, we treat this group of operations as a _"operations closure"_ due to its similarities with
the concept of [function closure](https://en.wikipedia.org/wiki/Closure_(computer_programming)).

In this document, we understand an _operations closure_ (or simply a _closure_) as a group of operations that:

- May use a set of values (arguments) from _outside_ the group of operation; we call these _external arguments_.
- May produce a set of values (results) that are used _outside_ the same group of operations; we call these
  _external results_.
- May use a set of values _produced by operations within the same group_; we call these _internal arguments_.
- May produce a set of values _used by operations within the same group_; we call these _internal results_.

We will use these concepts when describing the implementation of the process that creates the function definition
for a given set of operations.


## High level design of the functionization algorithm

The method `qoala::analysis::functionize::functionizeModule` located in the file `lib/Analysis/QMem/Functionize.cpp`
is the main entry point of the funcitonization algorithm. This method receives:

1. The MLIR _module_ object that contains the main function to process its operations.
2. A `classifyOperations` function, whose responsibility is process all the operations inside the main function
   (a `qmem.func` operation) and produce a set of operation groups. The correct signature of this function
   (`ClassifierFnTy`) and the type of the operations group (`QuantumOpsGroupTy`) is declared in the file
   `include/Analysis/QMem/Conversion.h`.

In addition to this, the `functionizeModule` function also outlines the high-level steps of the functionization
algorithm:

1. Create the operations groups by using the `classificationFunction` reference.
2. For each one of the function groups:
   1. Declare a `FunctionizeData` structure that will contain the value-mapping data.
   2. Create a new function with the operations of the group. This procedure will populate the functionize data struct.
   3. Search for the symbol (name) of the newly-created function.
   4. Insert a `call` operation to the newly-created function. This uses the data form the functionization to get the
      list of right arguments to use.
   5. Replace the values generated by the operations in the group, with the values generated by the `call` operation.
      We use the functionization data to aid this process, since it contains a map between the original _external_
      values of the operations group (closure) and the index of the results produced by the `call` operation.

The following sections will go deeper in the step 2.2., which carries most of the complexity when functionizing an
operation group


## Creation of a new function in the module

TODO


## Known issues

### Deletion of original operations in wrong order

TODO - Fixed in f5326817d7cd15bb3c221e724e1e4c6ff869150c


### Insertion of new (cloned) operations in the wrong order

TODO - Fixed in dbdceddf176ab702b2439cb9090c424c38eda9f7


### Incorrect handling of the function arguments when using multiple rotations

TODO
