#ifndef IQOALA_OPS_TD
#define IQOALA_OPS_TD

include "mlir/IR/OpBase.td"
include "Dialect/iqoala/iQoalaDialect.td"

// Needed for `[NoSideEffect]`
// Note that iQoala.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

class iQoala_Op<string mnemonic, list<Trait> traits = []> :
    Op<iQoala_Dialect, mnemonic, traits>;

def ConstantOp : iQoala_Op<"constant"> {
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

def TransposeOp : iQoala_Op<"transpose", [NoSideEffect]> {
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

def ReshapeOp : iQoala_Op<"reshape", [NoSideEffect]> {
  let arguments = (ins F64Tensor:$input);

  let results = (outs StaticShapeTensorOf<[F64]>);

  let assemblyFormat = [{
    `(` $input `:` type($input) `)` attr-dict `to` type(results)
  }];

  let hasCanonicalizer = 1;
}

#endif // IQOALA_OPS_TD