/* Minimal main program -- everything is loaded from the library */

#include "Python.h"
#include "pycore_pylifecycle.h"
#include "smalloc.h"

#ifdef MS_WINDOWS
int
wmain(int argc, wchar_t **argv)
{
    return Py_Main(argc, argv);
}
#else
int
main(int argc, char **argv)
{
    /* (elsa) ADDED THIS */
    int ret;
    if (!sm_pools_init(100)) // TODO have a default value somewhere ?
        return 1;

    ret = Py_BytesMain(argc, argv);

    sm_release_pools();
    return ret;
}
#endif
