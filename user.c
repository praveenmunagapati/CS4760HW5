#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#define THRESHOLD 10

struct timer
{
	int seconds;
	int ns;
};

int errno;
pid_t pid;
char errmsg[200];
struct timer *shmTime;
int *shmChild;
int *shmTerm;
sem_t * semTime;
sem_t * semTerm;
/* Insert other shmid values here */

void sigIntHandler(int signum)
{
	
	snprintf(errmsg, sizeof(errmsg), "USER %d: Caught SIGINT! Killing all child processes.", pid);
	perror(errmsg);	
	
	errno = shmdt(shmTime);
	if(errno == -1)
	{
		snprintf(errmsg, sizeof(errmsg), "USER %d: shmdt(shmTime)", pid);
		perror(errmsg);	
	}

	errno = shmdt(shmChild);
	if(errno == -1)
	{
		snprintf(errmsg, sizeof(errmsg), "USER %d: shmdt(shmChild)", pid);
		perror(errmsg);	
	}
	
	errno = shmdt(shmTerm);
	if(errno == -1)
	{
		snprintf(errmsg, sizeof(errmsg), "USER %d: shmdt(shmTerm)", pid);
		perror(errmsg);	
	}
	exit(signum);
}

int main (int argc, char *argv[]) {
int o;
int i;
int terminate = 0;
struct timer termTime;
int timeKey = atoi(argv[1]);
int childKey = atoi(argv[2]);
int index = atoi(argv[3]);
int termKey = atoi(argv[4]);
key_t keyTime = 8675;
key_t keyChild = 5309;
key_t keyTerm = 1138;
signal(SIGINT, sigIntHandler);
pid = getpid();

/* Seed random number generator */
srand(pid * time(NULL));

/* snprintf(errmsg, sizeof(errmsg), "USER %d: Slave process started!", pid);
perror(errmsg); */

/********************MEMORY ATTACHMENT********************/
/* Point shmTime to shared memory */
shmTime = shmat(timeKey, NULL, 0);
if ((void *)shmTime == (void *)-1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmat(shmidTime)");
	perror(errmsg);
    exit(1);
}

/* Point shmChild to shared memory */
shmChild = shmat(childKey, NULL, 0);
if ((void *)shmChild == (void *)-1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmat(shmidChild)");
	perror(errmsg);
    exit(1);
}

/* Point shmTerm to shared memory */
shmTerm = shmat(termKey, NULL, 0);
if ((void *)shmTerm == (void *)-1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmat(shmidTerm)");
	perror(errmsg);
    exit(1);
}
/********************END ATTACHMENT********************/

/********************SEMAPHORE CREATION********************/
/* Open Semaphore */
semTime=sem_open("semTime", 1);
if(semTime == SEM_FAILED) {
	snprintf(errmsg, sizeof(errmsg), "USER %d: sem_open(semTime)...", pid);
	perror(errmsg);
    exit(1);
}

semTerm=sem_open("semTerm", 1);
if(semTerm == SEM_FAILED) {
	snprintf(errmsg, sizeof(errmsg), "USER %d: sem_open(semTerm)...", pid);
	perror(errmsg);
    exit(1);
}
/********************END SEMAPHORE CREATION********************/

while(!terminate)
{
	if(rand()%100 <= THRESHOLD)
	{
		snprintf(errmsg, sizeof(errmsg), "USER %d: Slave process terminating!", pid);
		perror(errmsg);
		terminate = 1;
	}
	else
	{
		snprintf(errmsg, sizeof(errmsg), "USER %d: Slave process continuing!", pid);
		perror(errmsg);
	}
	
	/* Calculate Termination Time */
	termTime.ns = shmTime->ns + rand()%250000000;
	termTime.seconds = shmTime->seconds;
	if (termTime.ns > 1000000000)
	{
		termTime.ns -= 1000000000;
		termTime.seconds += 1;
	}

	/* Wait for the system clock to pass the time */
	while(termTime.seconds > shmTime->seconds);
	while(termTime.ns > shmTime->ns);

}

/* signal the release the process from the running processes */
sem_post(semTerm);
*shmTerm = index;

/********************MEMORY DETACHMENT********************/
errno = shmdt(shmTime);
if(errno == -1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmdt(shmTime)");
	perror(errmsg);	
}

errno = shmdt(shmChild);
if(errno == -1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmdt(shmChild)");
	perror(errmsg);	
}

errno = shmdt(shmTerm);
if(errno == -1)
{
	snprintf(errmsg, sizeof(errmsg), "USER: shmdt(shmTerm)");
	perror(errmsg);	
}
/********************END DETACHMENT********************/
exit(0);
return 0;
}
