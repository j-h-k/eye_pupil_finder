import posix_ipc
import os
import time

# Connect to existing
sem = posix_ipc.Semaphore("/capstone")


for x in range(10):
    sem.acquire()
    print("Critical! posix 2")
    time.sleep(2)
    print("Leaving! posix 2")
    sem.release()
    time.sleep(1)

#sem.unlink()
sem.close()
