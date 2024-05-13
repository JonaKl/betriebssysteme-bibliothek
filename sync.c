#include "sync.h"

/* Erzeugt eine neue Semaphorenmenge der Groesse size */
int erzeuge_sem(int size,int key)
{
	int semid = 0;
	/* pruefen, ob es eine Semaphorenmenge mit dem KEY gibt */
	semid = semget(key, 0, 0);
	if (semid == -1) {
		/* neue Semaphorenmenge erzeugen */
		if ((semid = semget(key, size, IPC_CREAT|IPC_EXCL|RIGHTS)) == -1) {
			perror("semget");
			return -1;
		}
	}
	return semid;
}

/* Initialisiert alle Elemente der Semaphorenmenge zu value */
void init_sem(int semid, int size, int value)
{
	int i = 0;
	union semun cmdval;
	cmdval.val = value;

	for (i = 0; i < size; i++) {
		if (semctl(semid, i, SETVAL, cmdval) == -1) {
			perror("semctl");
		}
	}
}

/* Fuehrt ein P (aka. "wait") auf dem Element pos der Semaphorenmenge semid aus */
int p(int semid,int pos)
{
	struct sembuf operation;
	operation.sem_num = pos;
	operation.sem_flg = 0;
	operation.sem_op = -1;

	if (semop(semid, &operation, 1) == -1) {
		perror("p");
		return -1;
	}
	return 0;
}

/* Fuehrt ein V (aka. "signal") auf dem Element pos der Semaphorenmenge semid aus */
int v(int semid, int pos)
{
	struct sembuf operation;
	operation.sem_num = pos;
	operation.sem_flg = 0;
	operation.sem_op = 1;

	if (semop(semid, &operation, 1) == -1) {
		perror("v");
		return -1;
	}
	return 0;
}

/* Semaphore auf einen Wert setzen */
void set_sem(int semid, int pos, int value)
{
	union semun cmdval;
	cmdval.val = value;
	if (semctl(semid, pos, SETVAL, cmdval) == -1) {
		perror("semctl");
	}
}
