import sysv_ipc
import posix_ipc
import time
import os
import ctypes


class EyeFinderReader:
    num_longs = 2 * 30 * ctypes.sizeof(ctypes.c_long)
    sizeoflong = ctypes.sizeof(ctypes.c_long)

    def __enter__(self):
        # Connect to existing shared memory
        self.memory = sysv_ipc.SharedMemory(123456)
        # Connect to existing semaphore
        self.sem = posix_ipc.Semaphore("/capstone")
        return self

    def bytesToLongs(self, bstr):
        tmp = [int.from_bytes(bstr[i * EyeFinderReader.sizeoflong:(i + 1) * EyeFinderReader.sizeoflong], byteorder='little')
               for i in range(int(len(bstr) / EyeFinderReader.sizeoflong))]
        return [[tmp[i],tmp[i+1]] for i in range(0, len(tmp), 2)]

    def read(self):
        self.sem.acquire()
        memory_value = self.memory.read()
        #print (int.from_bytes(memory_value, byteorder='little'))
        self.sem.release()
        facial_features_list = self.bytesToLongs(memory_value)
        return facial_features_list

    def __exit__(self, exc_type, exc_value, traceback):
        # Clean up Semaphore
        self.sem.close()
        # Clean up shared memory
        try:
            self.memory.remove()
        except:
            pass


with EyeFinderReader() as efr:
    print(efr.read())
