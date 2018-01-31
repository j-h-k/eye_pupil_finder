import sysv_ipc
import os

memory = sysv_ipc.SharedMemory(123456)

memory_value = memory.read()

print (memory_value)
