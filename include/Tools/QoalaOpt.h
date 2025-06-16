#ifndef QOALAOPT_H
#define QOALAOPT_H

#include "string"
#include "cstdint"

// Declarations for the extra options of the "qoala-opt" tool
// These extern declarations allow accessing the CLI values declared in qoala-opt.cpp

extern bool qoalaOptUnoptimize;
extern uint32_t qoalaOptSingleGateDuration;
extern uint32_t qoalaOptTwoGateDuration;
extern uint32_t qoalaOptLatency;
extern uint32_t qoalaOptLinkDuration;

#endif //QOALAOPT_H
