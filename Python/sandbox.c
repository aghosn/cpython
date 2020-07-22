/* Support for sandboxes */

#include "Python.h"
#include "frameobject.h"
#include "pycore_pystate.h"
#include "pycore_pyerrors.h"

int 
sandbox_prolog(PyThreadState *tstate, PyObject *mem, PyObject *sys) 
{
  /* NOTE: f has the following interesting fields (defined in Include/cpython/frameobject.h):
      - PyObject *f_locals   : local symbol table
      - PyObject *f_globals  : global symbol table
      - PyObject *f_builtins : builtin symbol table
     and all of them are needed for the execution 
    */

    printf("%s\n", "call prolog");
    printf("dependencies are: ");
    PyObject_Print(tstate->interp->dependencies, stdout, 0);
    putchar('\n');
    fflush(stdout);
    return 1;
}

int 
sandbox_epilog() 
{
    printf("%s\n", "call epilog");
    fflush(stdout);
    return 1;
}
