#ifndef HIR_TYPES
#define HIR_TYPES

def Hir_QubitType : Hir_Type<"Qubit"> {
  let mnemonic = "qubit";  // REQUIRED (otherwise no `generatedType<Parser|Printer>` will be generated)
  let printer = [{ ::print(*this, $_printer); }];
  let parser = [{ return ::parseQubit($_parser, $_ctxt); }];
}

#endif // HIR_TYPES