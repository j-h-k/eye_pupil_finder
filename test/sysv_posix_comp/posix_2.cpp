#include <iostream>

// Named Semaphore headers
// linke with -lpthread
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

// exit()
#include <stdlib.h>
// sleep()
#include <unistd.h>

int main()
{
  // Named Semaphore
  sem_t * sem = sem_open("/capstone", O_CREAT, 0666, 1);
  if (sem == SEM_FAILED) {
    std::cout<<"*** *** sem_open failed!!!"<<std::endl;
    sem_unlink("/capstone");
    exit(1);
  }
  else {
    for (int i = 0; i < 10; ++i) {
      sem_wait(sem);
      std::cout<<"Critical! posix 2"<<std::endl;
      sleep(2);
      std::cout<<"Leaving!  posix 2"<<std::endl;
      sem_post(sem);
      sleep(1);
    }

    sem_close(sem);
  }
}
