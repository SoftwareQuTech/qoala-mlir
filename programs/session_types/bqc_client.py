import math


class Qubit:
    def measure(self):
        pass

    def rot_Z(self):
        pass

    def H(self):
        pass


class EPRSocket:
    def create_keep(self) -> Qubit:
        pass


class CSocket:
    def send_int(self):
        pass

    def recv_int(self):
        pass


epr_socket = EPRSocket
csocket = CSocket

alpha = 0
beta = 0
theta1 = 0
theta2 = 0


def client():
    epr1: Qubit = epr_socket.create_keep()

    epr1.rot_Z(angle=theta2)
    epr1.H()
    p2 = epr1.measure(store_array=False)

    epr2: Qubit = epr_socket.create_keep()

    epr2.rot_Z(angle=theta1)
    epr2.H()
    p1 = epr2.measure(store_array=False)

    delta1 = alpha - theta1 + p1 * 16
    csocket.send_int(delta1)

    m1 = yield from csocket.recv_int()
    delta2 = math.pow(-1, m1) * beta - theta2 + p2 * 16
    csocket.send_int(delta2)

    m2 = yield from csocket.recv_int()

    return {"p1": p1, "p2": p2, "m2": m2}
