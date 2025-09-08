#ifndef QOALAOPT_H
#define QOALAOPT_H

#include <cstdint>

// Declarations for the extra options of the "qoala-opt" tool
// These extern declarations allow accessing the CLI values declared in qoala-opt.cpp

namespace qoala::options {
    extern bool qoalaOptUnoptimize;
    extern uint32_t qoalaOptSingleGateDuration;
    extern uint32_t qoalaOptTwoGateDuration;
    extern uint32_t qoalaOptLatency;
    extern uint32_t qoalaOptLinkDuration;
    extern uint32_t qoalaOptHostInstrTime;
    extern uint32_t qoalaOptHostPeerLatency;
    extern uint32_t qoalaOptQNosInstrTime;
    extern uint32_t qoalaOptQubitLifetime;
    extern bool qoalaOptGroupEntReqs;
    extern uint32_t qoalaOptProgramHorizon;
} // namespace qoala::options

#endif // QOALAOPT_H
