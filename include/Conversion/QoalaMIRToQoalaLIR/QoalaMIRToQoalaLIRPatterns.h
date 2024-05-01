#ifndef QOALAMIR_TO_QOALALIR_PATTERNS
#define QOALAMIR_TO_QOALALIR_PATTERNS
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

    /* Lowering for operations that belong in the module */
    class FuncOpLowering : public SimpleOneToToOneLoweringTemplate<qmem::FuncOp, qoalahost::MainFuncOp> {
    public:
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qoalahost::MainFuncOp createNewOp(qmem::FuncOp op, qmem::FuncOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const override;
    };
    class ReturnOpLowering : public SimpleOneToToOneLoweringTemplate<qmem::ReturnOp, qoalahost::ReturnOp> {
    public:
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qoalahost::ReturnOp createNewOp(qmem::ReturnOp op, qmem::ReturnOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RemoteOpLowering: public SimpleOneToToOneLoweringTemplate<qmem::RemoteOp, qoalahost::RemoteOp> {
    public:
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qoalahost::RemoteOp createNewOp(qmem::RemoteOp op, qmem::RemoteOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* Lowering for operations that can only belong to netqasm.local_routine or netqasm.request_routine */
    class QAllocLowering : public SimpleOneToToOneLoweringTemplate<qmem::QAllocOp, netqasm::QAllocOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::QAllocOp createNewOp(qmem::QAllocOp op, qmem::QAllocOp::Adaptor adaptor,
                                      ConversionPatternRewriter &rewriter) const override;
    };
    class QInitLowering : public SimpleOneToToOneLoweringTemplate<qmem::InitOp, netqasm::QInitOp> {
    public:
        // Constructor simply matches the super class
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;

        netqasm::QInitOp createNewOp(qmem::InitOp op, qmem::InitOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
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

#endif // QOALAMIR_TO_QOALALIR_PATTERNS
