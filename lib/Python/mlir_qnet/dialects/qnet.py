from ._qnet_ops_gen import *
from ._qnet_ops_gen import _Dialect

from ..ir import *
from ._ods_common import (
    _cext as _ods_cext,
)
# Import the types registered in the C++ python extension
from .._mlir_libs._qnetTypes.qnet_types import *
from .._mlir_libs._qnetTypes import __version__ as _cext_version

from typing import Any, List, Optional, Sequence, Union

__version__ = _cext_version


@_ods_cext.register_operation(_Dialect, replace=True)
class FuncOp(FuncOp):
    """Convenience wrapper around the ``qnet.func`` operation.

    Ported from the upstream ``FuncOp`` class in MLIR's ``func``
    dialect. The wrapper preserves the same API (so you can use it as
    a drop-in replacement when constructing HIR programmatically) and
    re-registers it against the ``qnet`` dialect so that
    ``qnet.func``-typed ops parse and print correctly through the
    Python bindings.
    """

    def __init__(
            self, name, type, *, visibility=None, body_builder=None, loc=None, ip=None
    ):
        """Build a ``qnet.func`` operation.

        Args:
            name: The function name, used as the symbol attribute.
                Coerced to ``str``.
            type: The function type. Either a :class:`FunctionType` or
                a tuple ``(inputs, results)`` of lists describing the
                input and result types — in the latter case a
                :class:`FunctionType` is built on the fly.
            visibility: One of ``"public"``, ``"private"``, or
                ``"nested"``. Defaults to ``None``, which implies
                private visibility.
            body_builder: Optional callback. When provided, a new
                entry block is created on the function and the
                callback is invoked with the new op as argument
                inside an :class:`InsertionPoint` context already
                set for the block. The callback is expected to
                insert a terminator in the block.
            loc: Optional :class:`Location` for the op.
            ip: Optional :class:`InsertionPoint` for the op.
        """
        sym_name = StringAttr.get(str(name))

        # If the type is passed as a tuple, build a FunctionType on the fly.
        if isinstance(type, tuple):
            type = FunctionType.get(inputs=type[0], results=type[1])

        type = TypeAttr.get(type)
        sym_visibility = (
            StringAttr.get(str(visibility)) if visibility is not None else None
        )
        super().__init__(sym_name, type, sym_visibility=sym_visibility, loc=loc, ip=ip)
        if body_builder:
            entry_block = self.add_entry_block()
            with InsertionPoint(entry_block):
                body_builder(self)

    @property
    def is_external(self):
        """``True`` if the function has no body (i.e. it is a declaration only)."""
        return len(self.regions[0].blocks) == 0

    @property
    def body(self):
        """The :class:`Region` holding the function's body blocks."""
        return self.regions[0]

    @property
    def type(self):
        """The function's :class:`FunctionType` (inputs and results)."""
        return FunctionType(TypeAttr(self.attributes["function_type"]).value)

    @property
    def visibility(self):
        """The function's visibility attribute (``StringAttr``)."""
        return self.attributes["sym_visibility"]

    @property
    def name(self) -> StringAttr:
        """The function name as a :class:`StringAttr`."""
        return StringAttr(self.attributes["sym_name"])

    @property
    def entry_block(self):
        """The function's entry block.

        Returns:
            The first :class:`Block` of the function's body region.

        Raises:
            IndexError: If the function has no body (i.e. it is an
                external declaration). Use :attr:`is_external` to
                check before accessing.
        """
        if self.is_external:
            raise IndexError("External function does not have a body")
        return self.regions[0].blocks[0]

    def add_entry_block(self, arg_locs: Optional[Sequence[Location]] = None):
        """Add an entry block to the function body.

        Uses the function signature to infer the block arguments
        (their types come from the function's input types).

        Args:
            arg_locs: Optional sequence of :class:`Location` values,
                one per block argument, used as the locations of the
                inferred block arguments.

        Returns:
            The newly created :class:`Block`.

        Raises:
            IndexError: If the function already has an entry block.
        """
        if not self.is_external:
            raise IndexError("The function already has an entry block!")
        self.body.blocks.append(*self.type.inputs, arg_locs=arg_locs)
        return self.body.blocks[0]

    @property
    def arg_attrs(self):
        """The function's argument attributes as an :class:`ArrayAttr`."""
        return ArrayAttr(self.attributes[ARGUMENT_ATTRIBUTE_NAME])

    @arg_attrs.setter
    def arg_attrs(self, attribute: Union[ArrayAttr, list]):
        if isinstance(attribute, ArrayAttr):
            self.attributes[ARGUMENT_ATTRIBUTE_NAME] = attribute
        else:
            self.attributes[ARGUMENT_ATTRIBUTE_NAME] = ArrayAttr.get(
                attribute, context=self.context
            )

    @property
    def arguments(self):
        """The list of block arguments of the function's entry block."""
        return self.entry_block.arguments

    @property
    def result_attrs(self):
        """The function's result attributes (as the raw attribute payload)."""
        return self.attributes[RESULT_ATTRIBUTE_NAME]

    @result_attrs.setter
    def result_attrs(self, attribute: ArrayAttr):
        self.attributes[RESULT_ATTRIBUTE_NAME] = attribute
