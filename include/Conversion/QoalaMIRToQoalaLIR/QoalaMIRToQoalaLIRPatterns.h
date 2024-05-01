#ifndef QMEM_TO_QOALAHOST_PATTERNS
#define QMEM_TO_QOALAHOST_PATTERNS
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"

using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion::mir {

    func::CallOp insertCallAngleTransform(Operation *operation,
                                          ConversionPatternRewriter &rewriter,
                                          Value angle);

    /* Lowering for operations that can only belong to netqasm.local_routine or netqasm.request_routine */
    class RotateXLowering : public SimpleOneToToOneLoweringTemplate<qmem::RotateXOp, netqasm::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::RotateXOp createNewOp(qmem::RotateXOp op, qmem::RotateXOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public SimpleOneToToOneLoweringTemplate<qmem::RotateYOp, netqasm::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::RotateYOp createNewOp(qmem::RotateYOp op, qmem::RotateYOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public SimpleOneToToOneLoweringTemplate<qmem::RotateZOp, netqasm::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::RotateZOp createNewOp(qmem::RotateZOp op, qmem::RotateZOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class MeasureLowering : public SimpleOneToToOneLoweringTemplate<qmem::MeasureOp, netqasm::MeasureOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::MeasureOp createNewOp(qmem::MeasureOp op, qmem::MeasureOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public SimpleOneToToOneLoweringTemplate<qmem::HadamardOp, netqasm::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::HadamardOp createNewOp(qmem::HadamardOp op, qmem::HadamardOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public SimpleOneToToOneLoweringTemplate<qmem::CnotOp, netqasm::CnotOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::CnotOp createNewOp(qmem::CnotOp op, qmem::CnotOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public SimpleOneToToOneLoweringTemplate<qmem::CzOp, netqasm::CzOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::CzOp createNewOp(qmem::CzOp op, qmem::CzOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public SimpleOneToToOneLoweringTemplate<qmem::CrotXOp, netqasm::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::CrotXOp createNewOp(qmem::CrotXOp op, qmem::CrotXOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
} // namespace qoala::conversion::mir

#endif // QMEM_TO_QOALAHOST_PATTERNS
