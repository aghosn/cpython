import datetime
import os 


LOOP=100000

def benchmark():
    sandbox("", ""):
        for i in range(0, LOOP):
           os.getpid() 

start = datetime.datetime.now()
benchmark()
end = datetime.datetime.now()

diff = end - start
print(diff)

