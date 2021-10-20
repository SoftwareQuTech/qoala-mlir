#ifndef HIR_TYPES
#define HIR_TYPES

def Hir_QubitType : Hir_Type<"Qubit"> {
  let mnemonic = "qubit";  // REQUIRED (otherwise no `generatedType<Parser|Printer>` will be generated)
  let parameters = (ins "int64_t":$size);
}

#endif // HIR_TYPES