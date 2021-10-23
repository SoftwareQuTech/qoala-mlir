#ifndef HIR_TYPES
#define HIR_TYPES

include "Dialect/hir/HirDialect.td"

def Quantum_QubitType : Hir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Quantum qubit array type";
  let parameters = (ins "int64_t":$size);
  let printer = [{ $_printer << "qubit"; }];
  let parser = [{ return QubitType::get($_ctxt, 1); }];
  let extraClassDeclaration = [{
    bool hasStaticSize() const;
    bool isSingleQubit() const;
  }];
}


#endif // HIR_TYPES