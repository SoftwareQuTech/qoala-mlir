//===- StandaloneTypes.cpp - Standalone dialect types -----------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Dialect/Qnet/QnetTypes.h"

#include "Dialect/Qnet/QnetDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace qoala::dialects::qnet;

#define GET_TYPEDEF_CLASSES
#include "Dialect/Qnet/QnetTypes.cpp.inc"

void QnetDialect::registerTypes() {
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/Qnet/QnetTypes.cpp.inc"
        >();
}
