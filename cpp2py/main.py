import sysv_ipc
import posix_ipc
import time
import os

class EyeFinderReader:
    def __enter__(self):
        # Connect to existing shared memory
        self.memory = sysv_ipc.SharedMemory(123456)
        # Connect to existing semaphore
        self.sem = posix_ipc.Semaphore("/capstone")
        return self

    def read(self):
        for x in range(5):
            self.sem.acquire()
            memory_value = self.memory.read()
            print (int.from_bytes(memory_value, byteorder='little'))
            self.sem.release()
            time.sleep(0.05)
            #time.sleep(1)

    def __exit__(self, exc_type, exc_value, traceback):
        # Clean up Semaphore
        self.sem.close()
        # Clean up shared memory
        try:
            self.memory.remove()
        except:
            pass

with EyeFinderReader() as efr:
    efr.read()
