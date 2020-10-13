from mylib import hey

def MyFunction():
    sandbox("",""):
        hey.foo()
        print("Hello")

def MySecondFunction():
    sandbox("", ""):
        hey.foo()
        print("Hello again")

MyFunction()
MySecondFunction()
