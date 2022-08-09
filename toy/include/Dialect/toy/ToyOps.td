#ifndef TOY_OPS
#define TOY_OPS

include "mlir/IR/OpBase.td"
include "Dialect/toy/ToyDialect.td"

// Needed for `[NoSideEffect]`
// Note that Toy.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

class Toy_Op<string mnemonic, list<Trait> traits = []> :
    Op<Toy_Dialect, mnemonic, traits>;

def ConstantOp : Toy_Op<"constant"> {
    let arguments = (ins F64ElementsAttr:$idk);

    let results = (outs F64Tensor);

    let hasVerifier = 1;

    let builders = [
        OpBuilder<(ins "DenseElementsAttr":$idk), [{
            build(odsBuilder, odsState, idk.getType(), idk);
        }]>,

        OpBuilder<(ins "double":$value)>
    ];
}

def TransposeOp : Toy_Op<"transpose", [NoSideEffect]> {
  let summary = "transpose operation";

  let arguments = (ins F64Tensor:$input);
  let results = (outs F64Tensor);

  let assemblyFormat = [{
    `(` $input `:` type($input) `)` attr-dict `to` type(results)
  }];

  let builders = [
    OpBuilder<(ins "Value":$input)>
  ];

  let hasCanonicalizer = 1;
}

#endif // TOY_OPS