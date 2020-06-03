/* Support for sandboxes */

#include "Python.h"
#include "frameobject.h"
#include "pycore_pystate.h"
#include "pycore_pyerrors.h"

int 
sandbox_prolog(PyFrameObject *f, PyObject *mem, PyObject *sys) 
{
    
    printf("%s\n", "call prolog");
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
