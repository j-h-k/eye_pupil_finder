#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include <iostream>

int main()
{

	int shmid;
	key_t key = 123456;
	char *shared_memory;
	unsigned int shared_size = 100;

	// Setup shared memory
	//if ((shmid = shmget(key, shared_size, IPC_EXCL | IPC_CREAT | 0666)) < 0)
	if ((shmid = shmget(key, shared_size, IPC_EXCL | IPC_CREAT | 0666)) < 0)
	{
		printf("Error getting shared memory id");
		exit(1);
	}
	else { // GOLDEN!!!
		std::cout << "shmid: " << shmid << std::endl;
	}

	// Attached shared memory
	if ((shared_memory = (char *)shmat(shmid, NULL, 0)) == (char *) -1)
	{
		printf("Error attaching shared memory id");
		exit(1);
	}
	else { // GOLDEN!!!
		std::cout << "shared_memory: " << (unsigned long)shared_memory << std::endl;
	}

	// Copy over data
	char str[] = "one";
	memcpy(shared_memory, str, strlen(str));

	// sleep so there is enough time to run the reader!
	//sleep(10);

	// Detach and remove shared memory
	shmdt(shared_memory);
	shmctl(shmid, IPC_RMID, NULL);



	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// SysV Semaphore now...

}
