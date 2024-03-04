//===- StandaloneTypes.cpp - Standalone dialect types -----------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Dialect/hir/HirTypes.h"

#include "Dialect/hir/HirDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir::hir;

#define GET_TYPEDEF_CLASSES
#include "Dialect/hir/HirTypes.cpp.inc"

void HirDialect::registerTypes() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/hir/HirTypes.cpp.inc"
      >();
}
