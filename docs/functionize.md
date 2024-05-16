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
partially-filled quantum operations group.

Despite this, the compiler specification limits the description only to how the operations are classified into
an operations group. In this description, once certain  conditions are met, the group is considered "complete"
and the operations are "placed into a separate function, replacing the original locations of the functionized
operations with the respective `call` operation".

From this description, we can identify the first 2 big steps of the functionization:

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
function declaration, we treat this group of operations as an _"operations closure"_ due to its similarities with
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
is the main entry point of the functionization algorithm. This method receives:

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

The `qoala::analysis::functionize::createNewFunctionWithOperations` located in the file `lib/Analysis/QMem/Functionize.cpp`
contains the implementation of how to create a new function with a given group of operations. This method receives
(among other not-so-relevant arguments):

1. The `FunctionizeData` instance used to leave the mapping data needed in the `functioneModule` method.
2. An `OpBuilder` object, which will be used to create new operations and _clone_ the ones in the group.
3. An _ordered set_ containing the operations of the current group. The order of this set establishes the
   order on which these operations need to be inserted in the new body. Additionally, we will start treating
   set as an _operations closure_.

It is important to remark that despite the fact that we need to "simply move" operations from the main body
to the body of a function that will be created, _moving an operation outside its scope is not an easy thing
to do_. This is due to the fact the in order to move teh operation to a new body _we effectively break the
data dependencies of the function_, i.e., the values used by that function and operations using values from
the operation _would need to be moved with the operation under move_. The `createNewFunctionWithOperations`
method solves this issue by _partially cloning_ the original operations.

To this end, the QMem operations implement the `qoala::helpers::SimpleCloneInterface` declared in the file
`include/Analysis/Helpers/SimpleCloneInterface.td`. Any operation implementing this interface (i.e., all
operations from the QMem dialect, see the `QMem_Op` class definition in teh  `include/Dialect/QMem/QMemOps.td`
file) _are forced_ to implement the `simpleClone` method, which provides a simple way to create new operation
_with the same operands and attributes as the original_, but not linked in any way to the original function block,
and, in particular, not used (yet) by any other function. After "simple cloning" an operation the
`createNewFunctionWithOperations` function will have to update the operands, to use the ones that come from
operations _within the new body_.

To help the creation of the cloned operations in the new function body, the `createNewFunctionWithOperations` method
keeps two maps between original operations results and the cloned operations results. One map contains translations
for the _internal results_ and the second for the _external results_ of the new function.

Additionally, the  map of the _external values_ is used to populate the `FunctionizeData` map that correlate the
original external result value with the index of the respective value that is placed in the return statement.
Naturally, these indexes will match 1-to-1 with the index of the returned values on the `call` operation placed in
the main body by `functionizeModule`.

Finally, the `createNewFunctionWithOperations` method achieve all these goals in the following steps:

1. Compute the arguments and values and types of the given operations set by calling the method
   `computeArgTypesAndReturns`. These results are placed in the `FunctionizeData` struct.
2. Create a new Function declaration (`func.func` operation), using the functionize data collected before for function
   arguments and results types. 
3. Create a new `OpBuilder` object, setting the insertion point at the beginning of the body block of the newly-created
   function definition.
4. For each one of the operations in the given set:
   1. Create a simple clone of the operation.
   2. Compute the new operands of the cloned operation (call to `mapOriginalOperandsInClonedOp`). This is achieved by
      using the information of the external values discovered before, and a map between the original internal results,
      and the new internal results. It is possible to assume that the new internal result value exists, since the
      operations set is iterated _in the order they appear in the original body_, so all the original values used by
      an operation are _already_ mapped to the new value at this point of the execution.
   3. Process the results of the cloned operation (call to `mapOriginalResultsInClonedOp`). This is necessary to update
      both the internal and external results maps. This is done so subsequent operations will know how to map the old
      operation result with the new one returned by the cloned operation, and help creating the external results map
      between the original values and their respective index in the return statement of the new function.
   4. Update the operands of the new operations using the recent computation.
5. Use the map of the external operations to: (1) identify the values that need to be used as operands of the `return`
   statement, and (2) correlate the original values with their index on the `return` statement.
6. Create the return statement and set its operands with the ones computed in the previous step.

The following section will go a bit deeper on the step 1. The `computeArgTypesAndReturns` method is in charge of
discovering the _external_ arguments and return values (and their types) of a given operations closure.


## Function type discovery

The final level of nesting in the functionzation algorithm is the discovery of the _external_ results and values
(and their respective types) of a given operations closure. This process is implemented in the `computeArgTypesAndReturns`
method, lcoated in the file `lib/Analysis/QMem/Functionize.cpp`. This is where the _closure_ concept becomes more relevant
to understand the logic behind this computation.

The `computeArgTypesAndReturns` receives the following arguments:

1. Declare a `FunctionizeData` structure that will contain the external results and values, together with the types.
2. An _ordered_ set containing the operations to analyze.

The implementation of the discovery algorithm will simply iterate over each of the given operations, and compute:

1. Get the operands (which are values) of the operation. For each one of them:
   1. Check if the operation that defines the operand value is _outside_ the set of operations. If so, the operand is
      an _external_ argument, so we add that value (and the type) to the set of external values (and external types).
2. Get the results of the operation. For each one of them:
   1. Check if the results has any uses (another operation _somewhere_ uses the current result of the current operation):
      1. If there are no uses, we still mark the value as a return value, and its type.
      2. If there are uses, then we iterate over the value users (operations):
         1. If the operation that uses the value is not in the operations closure set, then we discovered an _external_
            result of the operations closure, so we add that value (and the type) to the set of external values (and
            external types). In this step we also check that the value was not identified before as an external result.
            This can happen since a value can be used by multiple operations outside the operations closure. To achieve
            this extra check, we use the index of the result in the current operation.
3. Populate the `FunctionizeData` structure with the discovered external arguments and result values, and their
   respective types.


## Known issues

### Deletion of original operations in wrong order

TODO - Fixed in f5326817d7cd15bb3c221e724e1e4c6ff869150c


### Insertion of new (cloned) operations in the wrong order

TODO - Fixed in dbdceddf176ab702b2439cb9090c424c38eda9f7


### Incorrect handling of the function arguments when using multiple rotations

TODO
