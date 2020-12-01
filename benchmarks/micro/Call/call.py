import datetime

LOOP=100000
def mysandbox():
    sandbox("",""):
        a = 1+1#print("Hello") 

def benchmark():
    for i in range(0, LOOP):
        mysandbox()

# Cheat by taking out the first call
mysandbox()

start = datetime.datetime.now()
benchmark()
end = datetime.datetime.now()

diff = end - start
print(diff)

