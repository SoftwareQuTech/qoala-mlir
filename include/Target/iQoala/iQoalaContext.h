#ifndef IQOLACONTEXT_H
#define IQOLACONTEXT_H

namespace qoala::iqoala {
    /**
     * This class represents the "context" of the iQoala programs
     * It holds any static state, and mappings (such as virtual-physical
     * qubit maps).
     */
    class iQoalaContext {
    public:
        iQoalaContext() = default;
        ~iQoalaContext() = default;
    };
}
#endif //IQOLACONTEXT_H
