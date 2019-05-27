#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>

#define taille 256 /*taille choisie abritrairement ici*/
#define NbSalon 4
#define port 10400
#define id1 0
#define id2 1

struct thread_args{
	int numSalon;
	int socketServer;
	int socketClient1;
	int socketClient2;
};

struct salon{
	int numSalon;
	int nbClientConnecte;
	int socketClient1;
	int socketClient2;
};

struct salon salons[NbSalon];

int cl1;
int cl2;
int s;
char message[taille];

int end(char* message){
  int fin=strcmp("fin", message);
  if (fin==0){
    return 1;
  } else {
    return 0;
  }

}

void TransfertFichier(int client1, int client2){
      int res;		
      int tailleNom;
			int tailleContenu;
			char nomFichier[taille];
			char contenuFichier[taille];
			printf("Prêt à recevoir un ficher...\n");

			res = recv(client1, &tailleNom, sizeof(int),0); /* Réception de la taille du nom du fichier */

			if(res < 0){
				perror("Problème lors de la réception de la taille du nom du fichier");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			res = recv(client1, &nomFichier, tailleNom,0); /* Réception du nom du fichier */

			if(res < 0){
				perror("Problème lors de la réception du nom du fichier");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			nomFichier[tailleNom] = '\0';

			printf("Nom du fichier : %s\n", nomFichier); /* Affichage du nom du fichier */

			res = send(client2, &tailleNom, sizeof(int),0);

			if(res < 0){
				perror("Problème lors de l'envoie de la taille du nom du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			res = send(client2, &nomFichier, tailleNom,0);

			if(res < 0){
				perror("Problème lors de l'envoie du nom du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			res = recv(client1, &tailleContenu, sizeof(int),0); /* Réception de la taille du contenu du fichier */

			if(res < 0){
				perror("Problème lors de la réception de la taille du contenu du fichier");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			res = recv(client1, &contenuFichier, tailleContenu,0); /* Réception du contenu du fichier */

			if(res < 0){
				perror("Problème lors de la réception du contenu du fichier");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			contenuFichier[tailleContenu] = '\0';

			printf("Contenu du fichier : %s\n", contenuFichier); /* Affichage du nom du fichier */

			res = send(client2, &tailleContenu, sizeof(int),0);

			if(res < 0){
				perror("Problème lors de l'envoie de la taille du contenu du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}

			res = send(client2, &contenuFichier, tailleContenu,0);

			if(res < 0){
				perror("Problème lors de l'envoie du contenu du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("Socket fermé");
				exit(1);
			}
}

void *C2ToC1() {

  int end_tchat=0;
  while(!end_tchat){

    bzero(message, taille);

    int s = recv(cl2,message,taille,0);
    if (s < 0){
      perror("Erreur reception client 2");
    }
    s = send(cl1,message,strlen(message),0);
    if (s < 0){
      perror("Erreur envoi client 1");
    }

    //si fin du tchat

    if(end(message)){
      end_tchat=1;
    }
    if(strcmp(message, "file")== 0){
       TransfertFichier(cl2, cl1);
    }
  }
}



void *C1ToC2() {

  int end_tchat=0;
  while(!end_tchat){

    bzero(message, taille);

    int r = recv(cl1,message,taille,0);
    if (r < 0){
      perror("Erreur reception client 1");
    }
    r = send(cl2,message,taille,0);
    if (r < 0){
      perror("Erreur envoi client 2");
    }

    //si fin du tchat

    if(end(message)){
      end_tchat=1;
    }
    if(strcmp(message, "file")== 0){
       TransfertFichier(cl1, cl2);
    }
  }

}


int main(){

  /*creation socket*/
	int dS = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
  if (dS<0){
    printf("Erreur creation de socket");
  }
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(port); /*port*/

  int b = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
  if (b!=0){
    perror("Erreur de nommage socket \n");
  }
  int l = listen(dS,7);
  if (l != 0){
      perror("Erreur de listening \n");
    }

  struct sockaddr_in aC1 ;
  socklen_t lg1 = sizeof(struct sockaddr_in) ;

  struct sockaddr_in aC2 ;
  socklen_t lg2 = sizeof(struct sockaddr_in) ;


  for (int i = 0; i < NbSalon; i++){
		salons[i].numSalon = i;
		salons[i].nbClientConnecte = 0;
}

  while(1){

    char salonChoisitChar[2] = "";
		char salonDispo[taille] = "";
		for (int i = 1; i < NbSalon+1; i++){
			char message1[] = "Salon ";
			char message2[] = "";
			char message3[] = ", place disponible : ";
			char message4[] = "";
			sprintf(message2, "%d", i);
			int placeDispo = 2-salons[i].nbClientConnecte;
			sprintf(message4, "%d", placeDispo);

			strcat(salonDispo,message1);
			strcat(salonDispo,message2);
			strcat(salonDispo,message3);
			strcat(salonDispo,message4);
			strcat(salonDispo,"\n");
    }


    int tailleSalonDispo = strlen(salonDispo);
		
		int dSocketClient = accept(dS, (struct sockaddr*)&aC1, &lg1);

		if(dSocketClient < 0){
			printf("erreur : impossible de créer le socket client1");
			return 1;
    }

    int res = send(dSocketClient, &tailleSalonDispo, sizeof(int),0);
    if(res <= 0) {
    printf("erreur : impossible d'envoyer le nombre de salon disponible !");
    return 0;
    }

    res = send(dSocketClient, &salonDispo, tailleSalonDispo,0);
    if(res <= 0) {
    printf("erreur : impossible d'envoyer les salons disponible !");
    return 0;
    }

    res = recv(dSocketClient, &salonChoisitChar, taille,0);
    if(res <= 0) {
    printf("erreur : reception impossible du salon choisit par le client !");
    }

    

    int salonChoisit = atoi(salonChoisitChar);

    if(salons[salonChoisit].nbClientConnecte == 0){
			int numClient = 1;
			salons[salonChoisit].nbClientConnecte += 1;
			salons[salonChoisit].socketClient1 = dSocketClient;      

			res = send(dSocketClient, &numClient, sizeof(int),0);
      if(res <= 0) {
      printf("erreur : envoie du numero client !");
      }

      printf("%d", salons[salonChoisit].nbClientConnecte);

			} else if(salons[salonChoisit].nbClientConnecte == 1) {

      int numClient = 2;
      salons[salonChoisit].nbClientConnecte += 1;
      salons[salonChoisit].socketClient2 = dSocketClient;

      res = send(dSocketClient, &numClient, sizeof(int),0);
      if(res <= 0) {
      printf("erreur : envoi du numero client2");
      return 0;
      }
      
      char confirm[taille]="client 2 connecte, debut du chat !";
      res=send(salons[salonChoisit].socketClient1,&confirm, taille, 0);
      if (s<=0){
        printf("Erreur de transmission \n");
      }
      pthread_t tC1ToC2;
      pthread_t tC2ToC1;
      
			struct thread_args argsClient1;
			argsClient1.numSalon = salonChoisit;
			argsClient1.socketServer = dS;
			argsClient1.socketClient1 = salons[salonChoisit].socketClient1;
			argsClient1.socketClient2 = salons[salonChoisit].socketClient2;
			pthread_create(&tC1ToC2, 0, C1ToC2, &argsClient1);

			
			struct thread_args argsClient2;
			argsClient2.numSalon = salonChoisit;
			argsClient2.socketServer = dS;
			argsClient2.socketClient1 = salons[salonChoisit].socketClient2;
			argsClient2.socketClient2 = salons[salonChoisit].socketClient1;
      pthread_create(&tC2ToC1, 0, C2ToC1, &argsClient2);

      pthread_join(tC1ToC2, 0);
      pthread_join(tC2ToC1, 0);

      }

      // Debut du tchat avec les threads
      /*pthread_t tC1ToC2;
      pthread_t tC2ToC1;

      pthread_create(&tC1ToC2,0,C1ToC2,0);
      pthread_create(&tC2ToC1,0,C2ToC1,0);

      pthread_join(tC1ToC2,0);
      pthread_join(tC2ToC1,0);

      //fermeture socket client
      printf("/////// Fin du tchat /////// \n");
      close(cl1);
      close(cl2);

      // Tchat termine, recommence une boucle pour trouver des clients

      printf("En attente de connexion \n");*/

    }

    int c = close (dS);
    if (c != 0){
      perror("Erreur fermeture socket");
    }

	return 0;

}
