from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Any, Dict, List

from qoala.lang.hostlang import AssignCValueOp, IqoalaSingleton
from qoala.lang.program import BasicBlock, BasicBlockType, ProgramMeta, QoalaProgram


def to_list(s: str) -> List[str]:
    return [item.strip() for item in s.split(",")] if s else []


def to_dict(s: str) -> dict:
    if not s:
        return {}
    attr_list = s.split(",")
    attr_dict = {}
    for attr in attr_list:
        key, raw_value = [item.strip() for item in attr.split("=")]
        # Check if value is a string
        if raw_value.startswith('"') and raw_value.endswith('"'):
            value = raw_value[1:-1]  # Remove quotes
        # Check if value is a symbol
        else:
            try:
                # Convert to integer or float
                value = int(raw_value)
            except ValueError:
                try:
                    value = float(raw_value)
                except ValueError:
                    value = raw_value  # Keep it as a string
        attr_dict[key] = value
    return attr_dict


@dataclass
class LirOp:
    op_name: str
    results: List[str]
    arguments: List[str]
    attributes: Dict[str, Any]

    def __str__(self) -> str:
        return f"{self.results} {self.op_name} {self.arguments} {self.attributes}"


LIR_INSTR_PATTERN = r"""
    (?:(%[a-zA-Z0-9_]+(?:,\s*%[a-zA-Z0-9_]+)*)\s*=\s*)?  # results
    \"                                                   # starting quote
    ([a-zA-Z_][a-zA-Z0-9_]*\.[a-zA-Z_][a-zA-Z0-9_]*)     # op_name with a dot
    \"                                                   # ending quote
    \(
        ([^)]*)                                          # arguments
    \)
    (?:\s*\{([^}]*)\})?                                  # optional attributes
    \s*:\s*
    ([^\-]*->.*)                                         # type
"""


def parse_lir_file(filename: str) -> List[LirOp]:
    lir_ops = []

    with open(filename, "r") as f:
        for line in f:
            line = line.strip()
            # Ignore empty lines and module delimiters
            if not line or line.startswith("module") or line == "}":
                continue
            match = re.match(LIR_INSTR_PATTERN, line, re.VERBOSE)
            if match:
                (
                    results_str,
                    op_name,
                    arguments_str,
                    attributes_str,
                    type_str,
                ) = match.groups()
                results = to_list(results_str)
                arguments = to_list(arguments_str)
                attributes = to_dict(attributes_str)
                lir_op = LirOp(
                    op_name=op_name,
                    results=results,
                    arguments=arguments,
                    attributes=attributes,
                )
                lir_ops.append(lir_op)

    return lir_ops
