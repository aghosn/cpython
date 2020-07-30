



# Python, with support for sandboxes

This has been forked from Python version 3.9.0 alpha 3.

See [here](https://github.com/ElsaWeb/cpython/blob/master/README_PYTHON.rst) for the original content of CPython's README.

## Remarks

Run `./configure --with-pydebug && make -j` to build it under the name `python` (after having built libraries see below). Then only `make` suffices.

The libraries added are `smalloc` (in the `smalloc-lib` folder; used for the system of memory pools) and `sandbox` (in the `sandbox-lib` folder; serves to interface to the backend, for now only prints things). Since `lsandbox` is a dynamic library and has to be linked, the resulting executable can only be used from the current directory, or would need to run `export LD_LIBRARY_PATH=path/to/sandbox-lib:$LD_LIBRARY_PATH`. I don't know if it is possible to avoid this.

One needs to run `make` separately in their folder before building Python.


