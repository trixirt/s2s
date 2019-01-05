S2S

A Source to Source driver

The use of source to source tools is improved providing a compiler-like
driver interface where several standalone tools are integrated
together.  The integration is done through a script interface.

The driver is envoked as

s2s -script=<interface script> -db=<path-to-compile_commands.json>

Examples of interface scripts can be found in the lua/ directory.
