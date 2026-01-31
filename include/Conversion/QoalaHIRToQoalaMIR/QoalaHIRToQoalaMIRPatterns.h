#ifndef QNET_TO_QMEM_PATTERNS
#define QNET_TO_QMEM_PATTERNS
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QNet/QNet.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Transforms/DialectConversion.h"

namespace qoala::conversion::hir {
    class QoalaHIRToQoalaMIRTypeConverter : public mlir::TypeConverter {
    public:
        explicit QoalaHIRToQoalaMIRTypeConverter(mlir::MLIRContext *ctx);
    };

    class FuncOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::FuncOp, dialects::qmem::FuncOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::FuncOp op, dialects::qnet::FuncOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class ReturnOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::ReturnOp, dialects::qmem::ReturnOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::ReturnOp op, dialects::qnet::ReturnOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    /* Operations used to create entanglement */
    class EprsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::EprsOp, dialects::qmem::EprsOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::EprsOp op, dialects::qnet::EprsOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class EprsMeasureOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::EprsMeasureOp, dialects::qmem::EprsMeasureOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::EprsMeasureOp op, dialects::qnet::EprsMeasureOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    /* Remote operations can be mapped with a simple one-to-one operation */
    class RemoteOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::RemoteOp, dialects::qmem::RemoteOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RemoteOp op, dialects::qnet::RemoteOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class SendIntsOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::SendIntsOp, dialects::qmem::SendIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendIntsOp op, dialects::qnet::SendIntsOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RecvIntsOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RecvIntsOp, dialects::qmem::RecvIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvIntsOp op, dialects::qnet::RecvIntsOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class SendFloatsOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::SendFloatsOp, dialects::qmem::SendFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendFloatsOp op, dialects::qnet::SendFloatsOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RecvFloatsOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RecvFloatsOp, dialects::qmem::RecvFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvFloatsOp op, dialects::qnet::RecvFloatsOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class SendIntOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::SendIntOp, dialects::qmem::SendIntOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendIntOp op, dialects::qnet::SendIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RecvIntOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::RecvIntOp, dialects::qmem::RecvIntOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvIntOp op, dialects::qnet::RecvIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class SendFloatOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::SendFloatOp, dialects::qmem::SendFloatOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendFloatOp op, dialects::qnet::SendFloatOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RecvFloatOpLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RecvFloatOp, dialects::qmem::RecvFloatOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvFloatOp op, dialects::qnet::RecvFloatOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class NewQubitLowering : public helpers::OpLoweringTemplate<dialects::qnet::NewQubitOp, dialects::qmem::QAllocOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::NewQubitOp op, dialects::qnet::NewQubitOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateXLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotXOp, dialects::qmem::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotXOp op, dialects::qnet::RotXOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateYLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotYOp, dialects::qmem::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotYOp op, dialects::qnet::RotYOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateZLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotZOp, dialects::qmem::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotZOp op, dialects::qnet::RotZOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class MeasureLowering : public helpers::OpLoweringTemplate<dialects::qnet::MeasureOp, dialects::qmem::MeasureOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::MeasureOp op, dialects::qnet::MeasureOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class HadamardLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::HadamardOp, dialects::qmem::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::HadamardOp op, dialects::qnet::HadamardOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class XLowering : public helpers::OpLoweringTemplate<dialects::qnet::XOp, dialects::qmem::XOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::XOp op, dialects::qnet::XOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class YLowering : public helpers::OpLoweringTemplate<dialects::qnet::YOp, dialects::qmem::YOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::YOp op, dialects::qnet::YOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class ZLowering : public helpers::OpLoweringTemplate<dialects::qnet::ZOp, dialects::qmem::ZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::ZOp op, dialects::qnet::ZOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class CNotLowering : public helpers::OpLoweringTemplate<dialects::qnet::CnotOp, dialects::qmem::CnotOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CnotOp op, dialects::qnet::CnotOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class CzLowering : public helpers::OpLoweringTemplate<dialects::qnet::CzOp, dialects::qmem::CzOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CzOp op, dialects::qnet::CzOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class CRotXLowering : public helpers::OpLoweringTemplate<dialects::qnet::CrotXOp, dialects::qmem::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CrotXOp op, dialects::qnet::CrotXOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateXIntLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RotXIntOp, dialects::qmem::RotateXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotXIntOp op, dialects::qnet::RotXIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateYIntLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RotYIntOp, dialects::qmem::RotateYIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotYIntOp op, dialects::qnet::RotYIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class RotateZIntLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::RotZIntOp, dialects::qmem::RotateZIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotZIntOp op, dialects::qnet::RotZIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class CRotXIntLowering
        : public helpers::OpLoweringTemplate<dialects::qnet::CrotXIntOp, dialects::qmem::CrotXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CrotXIntOp op, dialects::qnet::CrotXIntOp::Adaptor adaptor,
                             mlir::ConversionPatternRewriter &rewriter) const override;
    };

    class ScfIfLowering : public mlir::OpRewritePattern<mlir::scf::IfOp> {
    public:
        explicit ScfIfLowering(mlir::MLIRContext *context): OpRewritePattern(context) {
            this->setHasBoundedRewriteRecursion();
        }

        mlir::LogicalResult matchAndRewrite(mlir::scf::IfOp op, mlir::PatternRewriter &rewriter) const override;
    };

    namespace helpers {
        mlir::LogicalResult replaceUnrealizedCastOps(mlir::Operation *user, const mlir::Value &replaceWith,
                                                     mlir::PatternRewriter &rewriter);

        mlir::LogicalResult replaceYieldOps(mlir::scf::YieldOp &thenYield, mlir::scf::YieldOp &elseYield,
                                            const mlir::DenseMap<uint32_t, mlir::Value> &qubitYieldIndexes,
                                            mlir::PatternRewriter &rewriter);

        mlir::LogicalResult analyzeYieldOps(mlir::scf::YieldOp &thenYield, mlir::scf::YieldOp &elseYield,
                                            mlir::DenseMap<uint32_t, mlir::Value> &matchedIdx);
        void fixEmptySCFBranchIfNeeded(mlir::scf::IfOp ifOp, mlir::PatternRewriter &rewriter);
    } // namespace helpers
} // namespace qoala::conversion::hir

#endif // QNET_TO_QMEM_PATTERNS
