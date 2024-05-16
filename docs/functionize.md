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

TODO 
