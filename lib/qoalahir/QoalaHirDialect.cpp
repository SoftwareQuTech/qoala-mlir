//===- StandaloneDialect.cpp - Standalone dialect ---------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Dialect/hir/HirDialect.h"
#include "Dialect/hir/Hir.h"

using namespace mlir;
using namespace mlir::hir;

#include "Dialect/hir/HirDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// QoalaHir dialect.
//===----------------------------------------------------------------------===//

void HirDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Dialect/hir/Hir.cpp.inc"
      >();
  registerTypes();
}
