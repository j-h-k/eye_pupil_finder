import sysv_ipc
import posix_ipc
import time
import os

# Connect to existing shared memory
memory = sysv_ipc.SharedMemory(123456)

# Connect to existing semaphore
sem = posix_ipc.Semaphore("/capstone")

for x in range(5):
    sem.acquire()
    memory_value = memory.read()
    print (memory_value)
    time.sleep(1)
    sem.release()
    time.sleep(1)

#sem.unlink()
sem.close()
