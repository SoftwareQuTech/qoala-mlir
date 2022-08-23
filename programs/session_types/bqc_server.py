import math


class Qubit:
    def measure(self):
        pass

    def rot_Z(self):
        pass

    def cphase(self, q):
        pass

    def H(self):
        pass


class EPRSocket:
    def recv_keep(self) -> Qubit:
        pass


class CSocket:
    def send_int(self):
        pass

    def recv_int(self):
        pass

    def recv_int(self):
        pass

    def send_int(self):
        pass


epr_socket = EPRSocket
csocket = CSocket


def server():
    epr1: Qubit = epr_socket.recv_keep()
    epr2: Qubit = epr_socket.recv_keep()
    epr2.cphase(epr1)

    delta1 = yield from csocket.recv_int()

    epr2.rot_Z(angle=delta1)
    epr2.H()
    m1 = epr2.measure(store_array=False)

    csocket.send_int(m1)

    delta2 = yield from csocket.recv_int()

    epr1.rot_Z(angle=delta2)
    epr1.H()
    m2 = epr1.measure(store_array=False)

    return {"m1": m1, "m2": m2}
