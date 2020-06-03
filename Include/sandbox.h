#ifndef Py_SANDBOX_H
#define Py_SANDBOX_H
#ifdef __cplusplus
extern "C" {
#endif

/* Support for sandboxes */
int sandbox_prolog(PyFrameObject *, PyObject *, PyObject *); 
int sandbox_epilog(void); // TODO needs arguments ?

#ifdef __cplusplus
}
#endif
#endif /* !Py_SANDBOX_H */
