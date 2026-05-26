# Load the native extension that registers the C dialect with MLIR.
from .dialects.qnet import register_dialect  # noqa: F401

#from ._mlir_libs._qnetTypes import __version__ as _cversion  # noqa: F401
from ._version import __version__

__all__ = ["__version__"]
