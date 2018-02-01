#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <iostream>

int main()
{

	int shmid;
	key_t key = 123456;
	char *shared_memory;
	size_t shared_size = sizeof(long);

	// ***
	// Setup shared memory
	if ((shmid = shmget(key, shared_size, IPC_EXCL | IPC_CREAT | 0666)) < 0)
	{
		printf("Error getting shared memory id");
		exit(1);
	}
	// Attached shared memory
	if ((shared_memory = (char *)shmat(shmid, NULL, 0)) == (char *) -1)
	{
		printf("Error attaching shared memory id");
		shmctl(shmid, IPC_RMID, NULL);
		exit(1);
	}
	std::cout << "shmid: " << shmid << std::endl;
	std::cout << "shared_memory: " << (unsigned long)shared_memory << std::endl;



	// ***
	// Named Semaphore
  sem_t * sem = sem_open("/capstone", O_CREAT|O_EXCL, 0666, 1);
  if (sem == SEM_FAILED) {
    std::cout<<"*** *** sem_open failed!!!"<<std::endl;
		sem_unlink("/capstone");
    sem_close(sem);
    exit(1);
  }







	// Copy over data
	//char str[] = "one";
	//memcpy(shared_memory, str, strlen(str));
	long num = 1;
	for (int i = 0; i < 1000000; ++i) {
		sem_wait(sem);
		//memcpy(shared_memory+sizeof(long)*i, &num, sizeof(long));
		memcpy(shared_memory, &num, sizeof(long));
		num++;
		// sleep(1);
		sem_post(sem);
		// sleep(1);
	}


	sleep(2);


	// Clean up Semaphore
	sem_unlink("/capstone");
	sem_close(sem);

	// Detach and remove shared memory
	shmdt(shared_memory);
	shmctl(shmid, IPC_RMID, NULL);

}
