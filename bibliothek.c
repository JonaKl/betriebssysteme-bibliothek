#include "sync.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

#define ANZAHL_PRIESTER 2
#define AMENOPHIS 0
#define BEMENOPHIS 1

#define PRIESTERNAME(x) ((x) == AMENOPHIS ? "Amenophis" : "Bemenophis")

/* Funktionsdeklarationen */
void programmabbruch(int);
int erzeugePriester(int);
void kind(int);
void vater(void);
void heimweg(int priester);
void deadlockobserver(void);

int semid_schriftrollen = 0, semid_priester = 0, vaterpid = 0;
int priester_pids[ANZAHL_PRIESTER];
enum STATUS {LESEN, EINTRETEN, WARTEN};
int color = 0;

int main(void)
{
	int i = 0;

	/* Signalhandler registrieren */
	struct sigaction aktion;
	aktion.sa_flags = 0;
	aktion.sa_handler = &programmabbruch;
	sigemptyset(&aktion.sa_mask);

	if (sigaction(SIGINT, &aktion, NULL) == -1) {
		perror("set actionhandler");
		exit(EXIT_FAILURE);
	}

	/* Die Kindprozesse erben den Signalhandler, da wir diesen bereits vor
	 * dem fork registrieren.  Damit der Vater (Bibliothek) weiss, dass er
	 * der Vater ist, speichern wir an dieser Stelle seine PID.
	 */
	vaterpid = getpid();

/* HIER MUSS EUER CODE EINGEFUEGT WERDEN */
	//Erzeugen der Semaphoren:
	semid_schriftrollen = erzeuge_sem(1, 0xcaffee);
	semid_priester = erzeuge_sem(2, 0xcall);
	init_sem(semid_schriftrollen, 1, 2);
	init_sem(semid_priester, 2, 0);

	for (i = 0; i < ANZAHL_PRIESTER; i++) {
		priester_pids[i] = erzeugePriester(i);
		sleep(1);
	}

	vater();
	return 0;
}

void vater(void)
{
	while(1){
		printf("\033[0;32mTemenophis (%d): Ich bin Temenophis, Herrscher ueber das "
			"Aegyptische Reich, Verwalter der Bibliothek von Hermopolis!\033[0m\n",
			getpid());
		deadlockobserver();
	/* HIER MUSS EUER CODE EINGEFUEGT WERDEN */
		sleep(5);
	}
}

void kind(int priester)
{
	/* Bemenophis trifft spaeter ein. */
	if (priester == BEMENOPHIS) {
		sleep(4);
	}
	color = 33;
	if(priester==AMENOPHIS){
		color = 34;
	}
	while(1){
		
		printf("\033[0;%dm%s (%d): Ich bin %s, auf geht's zur Bibliothek nach Hermopolis.\033[0m\n", 
			color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		/* Wartezeiten:
		* Lesen der ersten Schriftrolle: sleep(3)
		* Lesen der zweiten Schriftrolle: sleep(5)
		* 
		* D.h., ein Priester liest in der ersten genommenen Schriftrolle
		* kuerzer als in der zweiten.
		*/
		printf("\033[0;%dm%s (%d): Ich bin %s, ich brauche jetzt die erste Schriftrolle!\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		v(semid_priester, priester);
		p(semid_schriftrollen, 0);
		printf("\033[0;%dm%s (%d): Ich bin %s, ich lese jetzt für 3 Tage diese Schriftrolle.\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		sleep(3);
		printf("\033[0;%dm%s (%d): Ich bin %s, nun brauche ich noch die zweite Schriftrolle!\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		v(semid_priester, priester);
		p(semid_schriftrollen, 0);
		printf("\033[0;%dm%s (%d): Ich bin %s, ich nehme mir nun die zweite Schriftrolle.\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		printf("\033[0;%dm%s (%d): Ich bin %s, ich lese jetzt für 5 Tage diese Schriftrolle!\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		sleep(5);
		printf("\033[0;%dm%s (%d): Ich bin %s, ich habe diese Schriftrolle in 5 Tagen vollständig gelesen.\033[0m\n",
				color, PRIESTERNAME(priester), getpid(), PRIESTERNAME(priester));
		set_sem(semid_schriftrollen, 0, 2);
		/* Hier wird die Zeit des Heimwegs der Priester berechnet. Veraendert
		* den Inhalt der Funktion besser nicht, sonst bekommt ihr keine
		* Deadlocks mehr, oder muesst sehr lange darauf warten.
		*/
		set_sem(semid_priester, priester, 0);
		heimweg(priester);
	}
}

void heimweg(int priester)
{
	static int j = 0;

	/* Amenophis kommt nach 8 Zeiteinheiten wieder, Bemenophis in immer
	 * schnelleren Abstaenden.  Dadurch wird der Deadlock provoziert.
	 */
	if (priester == AMENOPHIS) {
		printf("\033[0;%dm%s (%d): Das reicht fuers erste. In 8 Tagen komme ich wieder!\033[0m\n",
		       color, PRIESTERNAME(priester), getpid());
		sleep(8);
	} else {
		if ((8 - j) < 0) {
			j = 0;
		}
		printf("\033[0;%dm%s (%d): Das reicht fuer's erste. In %d Tagen komme ich wieder\033[0m\n",
		       color, PRIESTERNAME(priester), getpid(), 8 - j);
		sleep(8 - j);
		j += 6;
	}
}

void programmabbruch(int sig)
{
	/* Pruefen, ob wir im Vaterprozess sind */
	if (getpid() == vaterpid) {
		int i;
		for(i = 0; i<ANZAHL_PRIESTER; i++){
			kill(priester_pids[i], SIGTERM);
			pid_t res;
			res = waitpid(priester_pids[i], NULL, WNOHANG);
			if(res==-1){
				if(errno!= ECHILD){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
			}
			
		}
		/* Freigabe der Semaphormengen für Priester und Schriftrollen */
		semctl(semid_priester, 0, IPC_RMID);
		semctl(semid_schriftrollen, 0, IPC_RMID);
		/* Beenden des Vaterprocesses */
		exit(0);
	}
}

int erzeugePriester(int priester)
{
	int pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		kind(priester);
		return 0;
	} else {
		return pid;
	}
}

void deadlockobserver(void)
{
	if(semctl(semid_priester, AMENOPHIS, GETVAL)==2&&semctl(semid_priester, BEMENOPHIS, GETVAL)==2){
		printf("\033[0;31mAmenophis, du gibst die Schriftrolle zurück und wirst getötet!\033[0m\n");
		kill(priester_pids[0], SIGTERM);
		pid_t res;
		res = waitpid(priester_pids[0], NULL, WNOHANG);
		if(res==-1){
			if(errno!= ECHILD){
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
		}
		sleep(1); //otherwise process is not fully killed and still increases semaphore!
		set_sem(semid_priester, AMENOPHIS, 0);
		v(semid_schriftrollen, 0);
		//printf("value semid_priester 0: %d, semid_schriftrollen: %d\n", 
			//semctl(semid_priester, AMENOPHIS, GETVAL), semctl(semid_schriftrollen, 0, GETVAL));
		priester_pids[0] = erzeugePriester(AMENOPHIS);
	}
}