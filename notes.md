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
Okay it is inside the full address space but not in the rest.

Stuck in a loop apparently that keeps calling run...
Where does it come from?
Apparently KVM_INTERNAL_ERROR
System call is failing apparently.
Okay so it's setting the wrong cr3.

Or the CR3 is not mapped for some reason. 
Let's see, we have a physical address that is: fa000
Let's look a the user memory regions.

# 22.10.2020

Cannot return from the sandbox apparently. 
So two problems so far

1. switching cr3 is wrong
2. Returning from the sandbox fails with sigaltstack and does an abort.

Okay so apparently with a redpill switch it works.

Maybe here: `[1] id 10824 name python from 0x00007ffff7bb61c0 in runtime.reflectcallmove+80 at /home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/runtime/mbarrier.go:230`

Okay so sigaltstack returns -1 from the handler.
Let's look at what it is supposed to return otherwise.

Okay so it fails inside the VM, don't know why, but it sucks. Maybe try to do it in tryRedpill? unminit apparently is the secret. 
Fook.
If it doesn't work should probably move to MPK instead.

Okay so sigprocmask called from the VM seems to be doing weird shit.
Apparently still get MMIO
So not the issue. but somewhere in fileops.1183

```
if regs.Rax == syscall.SYS_RT_SIGPROCMASK {
	regs.Rax = 0
	regs.Rdx = 0
	return syshandlerValid
}
```

Okay so problem with memset that uses avx2 extensions (vdmovqu). 
Disabled the memset for the moment.

Now the other issue is a second segfault that happens sometime before the second bluepill.

Maybe try to initialize God right before.

Problem comes from something we disable.
Apparently it's sys
Apparently the size is not aligned either fuck.

Oh and there is a fucking segfault in page walk. Something is mapped in godas but not the sandbox which then faults.

Where the bug happens

```
goroutine 17 [running, locked to thread]:
gosb/vtx/platform/ring0/pagetables.(*PageTables).pageWalk(0xc0000d8140, 0x7ffff4017000, 0x7fffeee92000, 0x7fffeeed1fff, 0x3, 0xc000127c08)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/vtx/platform/ring0/pagetables/gosb_walker.go:61 +0x46f
gosb/vtx/platform/ring0/pagetables.(*PageTables).Map(...)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/vtx/platform/ring0/pagetables/gosb_walker.go:40
gosb/vtx/platform/memview.(*MemoryRegion).ApplyRange(0xc000098580, 0x7fffeee92000, 0x40000, 0x36)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/vtx/platform/memview/memview.go:475 +0x12e
gosb/vtx/platform/memview.(*AddressSpace).Extend(0xc000100000, 0x0, 0xc000098580, 0x7fffeee92000, 0x40000, 0xffffffffffffff36)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/vtx/platform/memview/memview.go:283 +0x187
gosb/vtx.DRuntimeGrowth(0x1, 0x0, 0x7fffeee92000, 0x40000)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/vtx/dynamic.go:113 +0xf1
gosb.ExtendSpace(0xc000020001, 0x7fffeee92000, 0x40000)
	/home/aghosn/Documents/Programs/DCSL/sandboxing/code/Go/go/src/gosb/dynamic.go:255 +0xc0
main.SB_RegisterGrowth(...)
	/home/aghosn/Documents/Programs/DCSL/LitterBox2/src/main.go:107
main._cgoexpwrap_1db9a6c5b2e1_SB_RegisterGrowth(0x1, 0x7fffeee92000, 0x40000)
	_cgo_gotypes.go:170 +0x47
```

Solved the above by forcing it to execute in the host.

Now apprently sometimes the memview argument to the vcpu is nil.

Apparenlty triggered when we are in `_PyMem_DebugRawAlloc` and allocate new pool.

Can it be rtsigprocmask?

Okay so it's always the same page that gets hit. Why? What's there?
A pyunicode???

Check if address was used inside the free allocator.
Check the extend otherwise? This might be the issue, it might have triggered a remapping?
Implement another method to go do it outside? Touching the page does not seem to solve the issue.

Okay so the register_growth is never executed before the sandbox gets triggered, and never called before.

```
>>> p 'gosb/vtx.DynTots'
$1 = 132
>>> p 'gosb/vtx.DynSkipped'
$2 = 125
>>> p 'gosb/vtx.DynOut'
$3 = 0
>>> 
```

Not as slow as before, but still pretty slow 4x slower for some reason.
Must be syscalls I guess. Let's check that out. 
Avoid sig exits????
Apparently this works.
