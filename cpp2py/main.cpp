#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include <iostream>

int main() {

  int shmid;
  key_t key = 123456;
  char *shared_memory;
  size_t shared_size = 2 * 30 * sizeof(long);

  // ***
  // Setup shared memory
  if ((shmid = shmget(key, shared_size, IPC_EXCL | IPC_CREAT | 0666)) < 0) {
    printf("Error getting shared memory id\n");
    shmctl(shmget(key, shared_size, IPC_CREAT | 0666), IPC_RMID, NULL);
    exit(1);
  }
  // Attached shared memory
  if ((shared_memory = (char *)shmat(shmid, NULL, 0)) == (char *)-1) {
    printf("Error attaching shared memory id\n");
    shmctl(shmid, IPC_RMID, NULL);
    exit(1);
  }
  // std::cout << "shmid: " << shmid << std::endl;
  // std::cout << "shared_memory: " << (unsigned long)shared_memory << std::endl;

  // ***
  // Named Semaphore
  sem_t *sem = sem_open("/capstone", O_CREAT | O_EXCL, 0666, 1);
  if (sem == SEM_FAILED) {
    std::cout << "*** *** sem_open failed!!!" << std::endl;
    sem_unlink("/capstone");
    sem_close(sem);
    exit(1);
  }

  // Copy over data
  // char str[] = "one";
  // memcpy(shared_memory, str, strlen(str));
  long num = 1;
  for (int i = 0; i < 1000000; ++i, ++num) {
    sem_wait(sem);
    // memcpy(shared_memory+sizeof(long)*i, &num, sizeof(long));
    memcpy(shared_memory, &num, sizeof(long));
    num++;
    memcpy(shared_memory + sizeof(long) * (1), &num, sizeof(long));
    num++;
    memcpy(shared_memory + sizeof(long) * (2), &num, sizeof(long));
    num -= 2;
    sem_post(sem);
  }

  sleep(3);

  // Clean up Semaphore
  sem_unlink("/capstone");
  sem_close(sem);

  // Detach and remove shared memory
  shmdt(shared_memory);
  shmctl(shmid, IPC_RMID, NULL);
}
