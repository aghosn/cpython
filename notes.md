# 01.10.2020

```
╰─$ gg c_sb_cache                                                                                                                                                                                                                                     [14:18]
Python/compile.c
173:    PyObject *c_sb_cache;        /* ADDED THIS: Python dict holding package
323:    c->c_sb_cache = PyDict_New();
324:    if (!c->c_sb_cache) {
5023:    PyObject_SetItem(c->c_sb_cache, c->c_current_sb_id, PySet_New(NULL)); 
5042:            assert(PyDict_CheckExact(c->c_sb_cache));
5043:            dep_set = PyDict_GetItemWithError(c->c_sb_cache, c->c_current_sb_id);
6144:    sandboxes = c->c_sb_cache; // TODO some pre-processing ?

tags
50282:c_sb_cache     Python/compile.c        /^    PyObject *c_sb_cache;        \/* ADDED THIS: Python dict holding package$/;"    m       struct:compiler file:
```

Added in this function. 
Can we find out when the shit is called for a module that we define?

```
add_sandbox_dependency
```

Apparently our library is not added by `_PyState_AddModule`
Not called by `PyModule_New`
Not from `PyModule_NewObject`
In import.c `import_find_and_load`


Okay so we need "PyImport_ImportModuleLevelObject"
`PyImport_ImportModuleLevelObject`
Shit here `PyImport_ImportModuleLevelObject` calls this `import_get_module` and final_mode, but it looks like the mylib.hey is doing some weird shit. like first hey and then mylib.
Okay so in `import_find_and_load` we have mylib first and then hey.
And it's not in modules supposedly.

```
[0] from 0x000055555569ca6f in import_find_and_load+245 at Python/import.c:1743
[1] from 0x00005555556a0ea9 in PyImport_ImportModuleLevelObject+641 at Python/import.c:1880
[2] from 0x000055555567066e in import_name+566 at Python/ceval.c:5095
[3] from 0x000055555567c216 in _PyEval_EvalFrameDefault+31700 at Python/ceval.c:2961
[4] from 0x000055555568001e in _PyEval_EvalFrame+25 at ./Include/internal/pycore_ceval.h:43
[5] from 0x000055555568001e in _PyEval_EvalCode+2663 at Python/ceval.c:4246
[6] from 0x0000555555680248 in _PyEval_EvalCodeWithName+75 at Python/ceval.c:4278
[7] from 0x0000555555680287 in PyEval_EvalCodeEx+58 at Python/ceval.c:4294
[8] from 0x00005555556802b5 in PyEval_EvalCode+36 at Python/ceval.c:715
[9] from 0x00005555556ba9c2 in run_eval_code_obj+71 at Python/pythonrun.c:1126
```

So we import `mylib` and then we have `_PyObject_CallMethodIdObjArgs` with the fromlist
Added from `import_find_and_load`.
This is the one that adds it to the module `_PyObject_CallMethodIdObjArgs`
Apparently in `_PyObject_VectorcallTstate`

```
[0] from 0x00005555555be778 in object_vacall+685 at Objects/call.c:801
[1] from 0x00005555555be9c1 in _PyObject_CallMethodIdObjArgs+245 at Objects/call.c:893
[2] from 0x000055555569cb38 in import_find_and_load+304 at Python/import.c:1775
[3] from 0x00005555556a0ed3 in PyImport_ImportModuleLevelObject+641 at Python/import.c:1877
[4] from 0x00005555556706fc in import_name+566 at Python/ceval.c:5095
[5] from 0x000055555567c2a4 in _PyEval_EvalFrameDefault+31700 at Python/ceval.c:2961
[6] from 0x00005555556800ac in _PyEval_EvalFrame+25 at ./Include/internal/pycore_ceval.h:43
[7] from 0x00005555556800ac in _PyEval_EvalCode+2663 at Python/ceval.c:4246
[8] from 0x00005555556802d6 in _PyEval_EvalCodeWithName+75 at Python/ceval.c:4278
[9] from 0x0000555555680315 in PyEval_EvalCodeEx+58 at Python/ceval.c:4294
```

Okay so we know when it is loaded, how can we make sure it is added to the dependencies?
Now that we have the name, we should add it to dependencies and also make sure we find its name in `sm_add_mpool`
Okay so apprently it's not an issue for the moment that we do not call import_name.
I need to check what is going on if I add one.
Okay import dependencies are good for that shit.

Now what do we need to do? 
I need to get the name of the package getting loaded in add_sm_mpool. And that is shit.
Let's see where this comes from.
 Fuck so not all the names are available yet... fuuuuuuuuuuuuuu

The `co_sandboxes` is a dict tracking the dependencies.

Sandbox id is a PyLong object, we can cast it and use that.

Okay so os.path is replaced by posixpath
