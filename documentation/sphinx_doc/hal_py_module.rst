Logging
==========================

The python counterparts of HAL's C++ ``log_*`` macros.
Like the macros, every severity but ``info`` prefixes the message with the source location of the caller.

.. automodule:: hal_py
   :members: log_trace, log_debug, log_info, log_warning, log_error, log_critical
