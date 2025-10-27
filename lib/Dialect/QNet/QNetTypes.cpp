//===- StandaloneTypes.cpp - Standalone dialect types -----------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Dialect/QNet/QNetTypes.h"

#include "Dialect/QNet/QNetDialect.h"
#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"

using namespace qoala::dialects::qnet;

#define GET_TYPEDEF_CLASSES
#include "Dialect/QNet/QNetTypes.cpp.inc"

void QNetDialect::registerTypes() {
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/QNet/QNetTypes.cpp.inc"
            >();
}
