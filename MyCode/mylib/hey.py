import os

MyVariable = 3

def foo():
    nested = 545 + MyVariable
    print("In foo")
    print(os.getcwd())
    print(hex(id(os.getcwd)))
