



# Python, with support for sandboxes

This has been forked from Python version 3.9.0 alpha 3.

See [here](https://github.com/ElsaWeb/cpython/blob/master/README_PYTHON.rst) for the original content of CPython's README.

## Build

Run `./configure --with-pydebug && make -j` to build it under the name `python`.

## Sandboxes' support additions
 - **Parser**: added a `sandbox_stmt` to the grammar (in `Grammar/Grammar`). It has the simple form 
    
       sandbox(<string mem>, <string sys>):
		  <indented block of code>
   where the 2 strings are there for later use, and to have to same signature as the [go version](https://github.com/aghosn/go). 
 
   The parser creates the AST using ASDL (thanks to `Parser/Python.asdl`). I added a `Sandbox` node there, as well as a way to handle `Sandbox_kind` nodes in `Python/ast.c`. The function is called `ast_for_sandbox` and is responsible to create the `Sandbox` nodes with all necessary attributes.
 - **Compilation**: added a `compiler_sandbox` function in `Python/compile.c`. Basically, 
 
       sandbox(MEM, SYS):
	       BLOCK
        
    is implemented as:
        
        LOAD_CONST (MEM, SYS)
        SETUP_SANDBOX 1
        <code for BLOCK>
        SETUP_SANDBOX 0 
    
   this requires a new opcode, defined in `Lib/opcode.py`. It is called `SETUP_SANDBOX` and takes one argument, that we use as a boolean value to know if it is the beginning or the end of the sandbox.

 - **Evaluation**: added a case `TARGET(SETUP_SANDBOX)` in `Python/ceval.c` to handle the new opcode. If the argument is 1, then it retrieves the 'mem' and 'sys' strings on the stack and calls `sandbox_prolog`. Otherwise it calls `sandbox_epilog` to return from the sandbox. These two functions are declared in `Include/sandbox.h` and implemented in `Python/sandbox.c`, which are new files that I created. What is left to do is to complete these files and to adapt other things to make it compatible with the backend.

## Explanation of CPython's internals

### Compilation
 The compilation of the AST into Python bytecode is done in `Python/compile.c`. The bytecode works using a stack of execution. The Python `dis` module can be used to read bytecode of files, and a complete list of opcodes can be found on [its reference page](https://docs.python.org/3/library/dis.html).
 Here is a very simple example:
 
     import dep
     print(dep.foo())
 is compiled into the following bytecode:
     
		  0 LOAD_CONST               0 (0)
		  2 LOAD_CONST               1 (None) 
		  4 IMPORT_NAME              0 (dep) 
		  6 STORE_NAME               0 (dep)
		  8 LOAD_NAME                1 (print)
		 10 LOAD_NAME                0 (dep)
		 12 LOAD_METHOD              2 (foo)
		 14 CALL_METHOD              0
		 16 CALL_FUNCTION            1
		 18 POP_TOP
		 20 LOAD_CONST               1 (None)
		 22 RETURN_VALUE

Most arguments are indexes to consts and names lists explained in more details below, and their value is represented between parentheses. The arguments to `CALL_METHOD` and `CALL_FUNCTION` represent the number of arguments that the function/method takes and that have been loaded on the stack. We also notice that any function in the form `m.a()` where `m` is a module or package name is considered a method, even though no class has been defined.

### Evaluation
 - The result after compilation is a code object (defined in `Include/code.h`). The most important fields comprise `co_code`, which contains the instruction opcodes, and the lists `co_consts` and `co_names` containing respectively the constants and names used by the code: they are loaded using the `LOAD_CONST` and `LOAD_NAME` opcodes. The functions are compiled in individual code objects. Code objects can be manipulated from Python code using the builtin `compile` function.
 - The evaluation is done in `Python/ceval.c`, in the default evaluation frame function (`_PyEval_EvalFrameDefault` l.743, the main evaluation switch begins at line 1324)
 - The 'backbone' of the evaluation is the thread state (`PyThreadState *tstate`). The struct contains several fields, but the most important is the interpreter state (`PyInterpreterState *interp`). It also has a pointer to the frame currently being evaluated (`PyFrameObject *frame`).
 - The interpreter state (`tstate->interp`) is defined in `Include/pystate.h` and its internal struct is defined in `Include/cpython/pystate.h`. It contains most of the important fields during evaluation. For instance, `modules` is a `PyDict` that contains all imported modules (see below for an explanation of how it is used). It also has the `importlib` module directly as a field, which is the default module for imports.
 - A frame contains the locals (`f->f_locals`), the globals (`f->f_globals`) and the builtin-functions (`f->f_builtins`), all dictionary structures associating their names to their values (they are in fact, symbol tables). For the first frame (corresponding to the main program), the locals are the same as the globals, and are equal to the `__dict__` of the current module (see below the explanation of what it is). Then, only the globals are passed to the next frame, and it extracts from them the value corresponding to `__builtins__` to initialize `f->f_builtins`. When resolving a name (during the evaluation of `LOAD_NAME`), first the name is retrieved from the `names` list, then a look up is done, first in the locals, then the globals, and finally the builtins. If it doesn't appear in any of them, a name error is rised. 
 - The evaluation is done for a specific frame (as the name of the evaluation function indicates), which are represented as `PyFrameObject`s. During a function call, a new frame is created, to which are passed: the thread state, the code object of the function, and the globals. It then extracts the consts and names from the code object, and proceeds to evaluate the bytecode. When the function returns, the frame is marked as 'not evaluating' and `tstate->frame` is set back to the previous frame on the stack.
 - When evaluating a method call, first the whole module (or package) it is declared in is loaded on the stack using `LOAD_NAME`. Then the required method (or function, but it is considered as a method) is loaded from it, using this module's `__dict__`. Finally, the arguments to the method are pushed on the stack and the method is called.
   
### Imports
 - *Fun fact*: 'import' statements can be situated anywhere in Python code, not necessarily at the beginning of the file
 - The compile phase only generates the opcode `IMPORT_NAME` with the appropriate loading of constants before. Everything is then resolved only during evaluation.
 - The import machinery is started by `PyImport_ImportModuleLevelObject` in `Python/import.c`. It first checks if the module-to-be-imported is already present in `tstate->interp->modules`, which corresponds to the python interface `sys.modules`. This actually contains ALL modules needed to run the program, with around 40 of them being internals and only used to run python itself ('posix', '_thread', 'encodings.utf_8', etc). If is not there already it tries to find & load it, using the frozen module importlib (`tstate->interp->modules`; the source files are at `Lib/importlib/`, the specific function being called is `_find_and_load` in `_boostrap.py`). A large part the import mechanism is therefore implemented in Python, rather than C (and this probably makes it quite slow). The process is explained in the Python documentation [here](https://docs.python.org/3/reference/import.html), and it uses structures known as Finders and Loaders. I did not give it much more thought, but it most certainly is where it resolves a module's dependencies, and might then be of interest.
 - As a result, it adds the new modules to `tstate->interp->modules`.
 - Module objects are defined in `Objects/moduleobject.c`. Their most important field is `md_dict` which is a dictionary containing all the globals of the module (known as `__dict__` from the Python interface). For a module, the notion of 'globals' includes: the modules it imports, the code objects of the functions defined, as well as the global variables (among which are the implicit, Python-specific ones, such as `__name__`, `__doc__`, etc).

### Memory Management
 - Python uses automatic reference counting rather than garbage collection for memory management. Actually it still uses a gc to get rid of the cycles (which are a big problem for ARC). It has some macros, `INCREF` and `DECREF` to update the reference counter of the `PyObject` during evaluation. Sometimes, the reason why they are used is quite obscure, so it is not impossible that I forgot to use them at some places.
 - Every value or data structure is 'wrapped' in a `PyObject` and is stored on the heap. The definition is in `Include/object.h` and a good explanatory comment about what they are can be found there. The basic form of a `PyObject` has only 2 attributes, the reference count and its type. It can then be cast into a bigger struct using the appropriate macros to access its value (variable size objects contain a pointer to another place on the heap, so that the size of a `PyObject` is fixed).

#### Extra note: What I learned on how to use gdb with Python
- The simplest way is to use `gdb <link to python binary>`, then set breakpoints inside cpython, and then type `run <script python>`.
- Sometimes we want to directly see the evaluation of our Python script, and putting a breakpoint at evaluation does not suffice since it first evaluates quite a lot of internal Python files. A trick that can be used is to add the following line, in place of a breakpoint in the script: ```import os, signal; os.kill(os.getpid(), signal.SIGTRAP)```. The sigtrap will stop gdb, but then it is possible to resume execution.
- Since I wanted to inspect memory from gdb, I looked for a concise way to do it, and found that you can define your own commands. It is very handy to do:
 
      (gdb) define xxd
       > dump binary memory dump.bin $arg0 $arg0+$arg1
       > shell xxd dump.bin
       > end 
  Which then allows to type `xxd <addr> <n>` to see the 'n' first bytes at address 'addr'. However, since almost everything is a `PyObject *`, looking at the memory is not very instructive.

