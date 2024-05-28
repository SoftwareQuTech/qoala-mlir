#ifndef NETQASMTOIQOALATRANSLATION_H
#define NETQASMTOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerNetQASMToiQoalaTranslations(mlir::DialectRegistry &registry);
}
#endif //NETQASMTOIQOALATRANSLATION_H
