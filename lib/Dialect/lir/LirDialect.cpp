#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/Lir.h"

/* Dirty fix: Lir.h includes the declaration of mlir::lir::detail::NoisyQubitTypeStorage
 * However, since it is a declaration, it cannot declare any inheritance (like this definition)
 * This leads to a compile-time assert failure, where the NoisyQubitTypeStorage struct cannot be
 * instantiated by the MLIR templates, since it is not a complete definition:
 * /usr/bin/../lib/gcc/x86_64-linux-gnu/11/../../../../include/c++/11/type_traits:1339:23: error: incomplete type 'mlir::lir::detail::NoisyQubitTypeStorage' used in type trait expression
 * 1339 |                     __bool_constant<__has_trivial_destructor(_Tp)>>
 *      |                                     ^
 * /usr/bin/../lib/gcc/x86_64-linux-gnu/11/../../../../include/c++/11/type_traits:3215:5: note: in instantiation of template class 'std::is_trivially_destructible<mlir::lir::detail::NoisyQubitTypeStorage>' requested here
 * 3215 |     is_trivially_destructible<_Tp>::value;
 *      |     ^
 * /usr/local/include/mlir/Support/StorageUniquer.h:152:24: note: in instantiation of variable template specialization 'std::is_trivially_destructible_v<mlir::lir::detail::NoisyQubitTypeStorage>' requested here
 *  152 |     if constexpr (std::is_trivially_destructible_v<Storage>)
 *      |                        ^
 * /usr/local/include/mlir/IR/TypeSupport.h:267:27: note: in instantiation of function template specialization 'mlir::StorageUniquer::registerParametricStorageType<mlir::lir::detail::NoisyQubitTypeStorage>' requested here
 *  267 |     ctx->getTypeUniquer().registerParametricStorageType<typename T::ImplType>(
 *      |                           ^
 * /usr/local/include/mlir/IR/TypeSupport.h:257:5: note: in instantiation of function template specialization 'mlir::detail::TypeUniquer::registerType<mlir::lir::NoisyQubitType>' requested here
 *  257 |     registerType<T>(ctx, T::getTypeID());
 *      |     ^
 * /usr/local/include/mlir/IR/Dialect.h:306:26: note: in instantiation of function template specialization 'mlir::detail::TypeUniquer::registerType<mlir::lir::NoisyQubitType>' requested here
 *  306 |     detail::TypeUniquer::registerType<T>(context);
 *      |                          ^
 * /usr/local/include/mlir/IR/Dialect.h:264:6: note: in instantiation of function template specialization 'mlir::Dialect::addType<mlir::lir::NoisyQubitType>' requested here
 *  264 |     (addType<Args>(), ...);
 *      |      ^
 * /home/diego/code/qoala-mlir-b/lib/Dialect/lir/LirDialect.cpp:27:5: note: in instantiation of function template specialization 'mlir::Dialect::addTypes<mlir::lir::CommQubitType, mlir::lir::EntQubitType, mlir::lir::NoisyQubitType, mlir::lir::QubitType>' requested here
 *   27 |     addTypes<
 *      |     ^
 * /home/diego/code/qoala-mlir-b/build/include/Dialect/lir/LirTypes.h.inc:40:8: note: forward declaration of 'mlir::lir::detail::NoisyQubitTypeStorage'
 *   40 | struct NoisyQubitTypeStorage;
 * The correct declaration (and definition) of the struct is available in the "Dialect/lir/Dialect.h.inc" file
 * (provided that we use the #define GET_TYPEDEF_CLASSES, just like in "Lir.h"). This makes this file compile, however
 * *it makes fail the linking of `lir-opt`*, since the class defintion is found twice in the linked libraries (once here,
 * and once in the libMLIRLir.a, since Lir.cpp also uses the"Dialect/lir/Dialect.h.inc with the #define GET_TYPEDEF_CLASSES macro).
 * BARCKGROUND: Since C and C++ share the linker, a struct/class definition (struct/class name that contains methods) need to be
 * translated by the compiler into something that the linker can understand. To this end, the compiler will simply "extract" the
 * methods and translated their named into the mangled ones, and export those sumbols in the binary. After this happens (at compile
 * time), a C++ class or struct is not different from a C struct or union. The mangled names only become meaningful at runtime,
 * since the dispatcher needs to know which method to invoke; for this, it uses the mangled name of the method.
 * SOLUTION: Knowing this we can simply insert a "shell" *declaration* of the struct here (declaring the inheritance), so the
 * instantiation of the templates work correctly. When linking (and knowing that the C/C++ linker will look for the mangled names),
 * the "expanded" definition for this class (which contains methods and stuff) will link with no problems, since the linker will
 * simply find the right mangled method name within the linked libraries/objects.
 */

namespace mlir::lir::detail {
struct NoisyQubitTypeStorage : public ::mlir::TypeStorage{};
}

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/lir/LirDialect.cpp.inc"

using namespace mlir;
using namespace mlir::lir;

void LirDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/lir/Lir.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/lir/LirTypes.cpp.inc"
        >();
}
