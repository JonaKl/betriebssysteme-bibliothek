#ifndef __SYNC_H_
#define __SYNC_H_
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#define RIGHTS 0600

#ifndef __APPLE__
/* Datenstruktur zum Abfragen/Setzen der Semaphoren */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};
#endif

/* Erzeugt eine neue Semaphorenmenge der Groesse size */
int erzeuge_sem(int size, int key);
/* Initialisiert alle Elemente der Semaphorenmenge zu value */
void init_sem(int semid, int size, int value);
/* Fuehrt ein P (aka. "wait") auf dem Element pos der Semaphorenmenge semid aus */
int p(int semid, int pos);
/* Fuehrt ein V (aka. "signal") auf dem Element pos der Semaphorenmenge semid aus */
int v(int semid, int pos);
/* Semaphore auf einen Wert setzen */
void set_sem(int semid, int pos, int value);
#endif
