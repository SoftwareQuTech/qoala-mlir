#ifndef QOALA_HIR_TYPES_TD
#define QOALA_HIR_TYPES_TD

def QoalaHir_IntStructType : DialectType<
    QoalaHir_Dialect, CPred<"$_self.isa<IntStructType>()">, "HIR IntStruct type">;

def QoalaHir_Type : AnyTypeOf<[QoalaHir_IntStructType]>;

#endif // QOALA_HIR_TYPES_TD