from __future__ import annotations

import heapq
import re
from dataclasses import dataclass
from enum import Enum
from typing import Any, Dict, List, Optional

from netqasm.lang.instr import NetQASMInstruction
from netqasm.lang.instr.core import SetInstruction
from netqasm.lang.instr.vanilla import CphaseInstruction
from netqasm.lang.operand import Immediate, Register
from netqasm.lang.subroutine import Subroutine
from qoala.lang.hostlang import AssignCValueOp, IqoalaSingleton
from qoala.lang.program import BasicBlock, BasicBlockType, ProgramMeta, QoalaProgram
from qoala.lang.routine import LocalRoutine, RoutineMetadata

from codegen.lir import LirOp, parse_lir_file


class RegisterManager:
    def __init__(self):
        # Register to Value mapping
        self.reg_to_val: Dict[str, int] = {}

        # Value to Register mapping
        self.val_to_reg: Dict[int, str] = {}

        self._used_ids = set()
        self._available_ids = []

    def id_to_reg(self, i: int) -> str:
        if i < 16:
            return f"R{i}"
        elif i < 32:
            return f"Q{i % 16}"
        elif i < 48:
            return f"C{i % 16}"
        elif i < 64:
            return f"M{i % 16}"

    def reg_to_id(self, reg: str) -> int:
        if reg.startswith("R"):
            return int(reg[1:])
        elif reg.startswith("Q"):
            return int(reg[1:]) + 16
        elif reg.startswith("C"):
            return int(reg[1:]) + 32
        elif reg.startswith("M"):
            return int(reg[1:]) + 48

    def _allocate(self) -> int:
        if not self._available_ids:
            new_id = max(self._used_ids, default=-1) + 1
        else:
            new_id = heapq.heappop(self._available_ids)
        self._used_ids.add(new_id)
        return new_id

    def _free(self, id: int) -> None:
        self._used_ids.remove(id)
        heapq.heappush(self._available_ids, id)

    def allocate_register_with_value(self, value: int) -> str:
        """
        If a register with the value already exists, return that register.
        Otherwise, allocate a new register, set its value, and return the register.
        """
        # Check if value is already loaded in some register
        if value in self.val_to_reg:
            return self.val_to_reg[value]

        # If not, allocate a new register
        new_reg_id = self._allocate()
        new_register = self.id_to_reg(new_reg_id)

        # Update mappings
        self.reg_to_val[new_register] = value
        self.val_to_reg[value] = new_register

        return new_register

    def get_value_of_register(self, register: str) -> int:
        """Return the current value in the given register."""
        return self.reg_to_val.get(register, None)

    def free_register(self, register: str):
        """Free up a register and clear its associated value."""
        value = self.reg_to_val.pop(register, None)
        if value is not None:
            self.val_to_reg.pop(value, None)
        self._free(self.reg_to_id(register))


@dataclass
class LocalRoutineContext:
    routine: LocalRoutine
    regmgr: RegisterManager
    instructions: List[NetQASMInstruction]


class CodegenStatus(Enum):
    pass


class CodeGenerator:
    def __init__(self, path: str) -> None:
        self._path = path
        self._used_ids = set()
        self._available_ids = []
        self._qubits: Dict[str, int] = {}  # name -> virt ID

        self._local_routines: Dict[str, LocalRoutineContext] = {}
        self._current_lr: Optional[str] = None
        self._lr_idx = 0

    def _new_lr(self) -> None:
        name = f"subrt_{self._lr_idx}"
        routine = LocalRoutine(
            name=name,
            subroutine=Subroutine(),
            return_vars=[],
            metadata=RoutineMetadata([], []),
        )
        self._lr_idx += 1
        self._current_lr = name
        self._local_routines[name] = LocalRoutineContext(routine, RegisterManager(), [])

    def _wrap_up_lr(self) -> None:
        lr = self._local_routines[self._current_lr]
        for reg, value in lr.regmgr.reg_to_val.items():
            instr = SetInstruction(reg=Register.from_str(reg), imm=Immediate(value))
            lr.instructions.insert(0, instr)

        lr.routine.subroutine.instructions = lr.instructions
        self._current_lr = None

    def _allocate(self) -> int:
        if not self._available_ids:
            new_id = max(self._used_ids, default=-1) + 1
        else:
            new_id = heapq.heappop(self._available_ids)
        self._used_ids.add(new_id)
        return new_id

    def _free(self, id: int) -> None:
        self._used_ids.remove(id)
        heapq.heappush(self._available_ids, id)

    def _handle_op(self, op: LirOp) -> None:
        if op.op_name == "lir.entangle_keep":
            qubit = op.results[0]
            self._qubits[qubit] = self._allocate()
        elif op.op_name == "lir.ent_to_local":
            in_qubit = op.arguments[0]
            assert in_qubit in self._qubits
            qid = self._qubits.pop(in_qubit)
            out_qubit = op.results[0]
            self._qubits[out_qubit] = qid
        elif op.op_name == "lir.cphase":
            if self._current_lr is None:
                self._new_lr()
            lr = self._local_routines[self._current_lr]
            q0 = op.arguments[0]
            q1 = op.arguments[1]
            assert q0 in self._qubits
            assert q1 in self._qubits
            loc0 = self._qubits[q0]
            loc1 = self._qubits[q1]
            reg0 = lr.regmgr.allocate_register_with_value(loc0)
            reg1 = lr.regmgr.allocate_register_with_value(loc1)

            instr = CphaseInstruction(
                reg0=Register.from_str(reg0), reg1=Register.from_str(reg1)
            )
            lr.instructions.append(instr)
        elif op.op_name == "lir.receive_msg":
            if self._current_lr is not None:
                self._wrap_up_lr()

    def generate(self) -> QoalaProgram:
        ops = parse_lir_file(self._path)
        for op in ops:
            self._handle_op(op)

        print(self._qubits)
        for lr in self._local_routines.values():
            print(lr.routine.subroutine)

        meta = ProgramMeta.empty("program")
        block = BasicBlock(
            "b0",
            typ=BasicBlockType.CL,
            instructions=[AssignCValueOp(IqoalaSingleton("x"), 3)],
        )
        return QoalaProgram(meta, [block])


if __name__ == "__main__":
    CodeGenerator("programs/lir/bqc_server.mlir").generate()
