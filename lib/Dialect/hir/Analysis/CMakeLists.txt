add_mlir_dialect_library(MLIRHirAnalysis
  CheckLinear.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/hir

  DEPENDS
  MLIRHirPassIncGen

  LINK_LIBS PUBLIC
  MLIRAffineDialect
  MLIRAffineTransforms
  MLIRAffineUtils
  MLIRAnalysis
  MLIRArithDialect
  MLIRArithTransforms
  MLIRArithUtils
  MLIRBufferizationDialect
  MLIRBufferizationTransforms
  MLIRComplexDialect
  MLIRDestinationStyleOpInterface
  MLIRDialectUtils
  MLIRFuncDialect
  MLIRFuncToLLVM
  MLIRFuncTransforms
  MLIRInferTypeOpInterface
  MLIRIR
  MLIRMemRefDialect
  MLIRMemRefTransforms
  MLIRLinalgDialect
  MLIRLinalgUtils
  MLIRSCFDialect
  MLIRSCFTransforms
  MLIRSCFUtils
  MLIRPass
  MLIRSparseTensorDialect
  MLIRTensorDialect
  MLIRTensorTilingInterfaceImpl
  MLIRTensorTransforms
  MLIRTransforms
  MLIRTransformUtils
  MLIRValueBoundsOpInterface
  MLIRVectorDialect
  MLIRVectorTransforms
  MLIRVectorUtils
  MLIRX86VectorTransforms
  MLIRVectorToSCF
)
