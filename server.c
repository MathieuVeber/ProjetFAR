#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include "struct.h"

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
struct message msg;

/*int end(char* message){
  int fin=strcmp("fin", message);
  if (fin==0){
    return 1;
  } else {
    return 0;
  }

}*/

void *C1ToC2() {

  int end_tchat=0;
  struct message the_message; 
  while(1){

    if(!end_tchat){
      //on receptionne le mssg du client 2
      int r = recv(cl2,&the_message,sizeof(the_message)+1,0); 
      if (r < 0){
        printf("Erreur reception client 2");
      }

      //on va le renvoyer au client 1
      struct message msg_send;
      msg_send.etiquette = the_message.etiquette;
      if (strcmp(the_message.contenu, "file\n")==0){
        msg_send.etiquette=1;
      }
    
      r = send(cl1,&msg_send,sizeof(msg_send)+1,0);
      if (r < 0){
        printf("Erreur envoi client 1");
      }

      if (strcmp(the_message.contenu,"fin\n")==0){
        end_tchat=1;
        printf("client 1 a fermé le tchat");
      }
    }
    
    //si fin du tchat
    else{
      break;
    }
  }

}



void *C2ToC1() {
  int end_tchat=0;
  struct message the_message; 
  while(1){

    if(!end_tchat){
      //on receptionne le mssg du client 1

      printf("%d", cl1);
      int r = recv(cl1,&the_message,sizeof(the_message)+1,0); 
      if (r < 0){
        printf("Erreur reception client 1");
      }

      //on va le renvoyer au client 2
      struct message msg_send;
      msg_send.etiquette = the_message.etiquette;
      if (strcmp(the_message.contenu, "file\n")==0){
        msg_send.etiquette=1;
      }
    
      r = send(cl2,&msg_send,sizeof(msg_send)+1,0);
      if (r < 0){
        printf("Erreur envoi client 2");
      }

      if (strcmp(the_message.contenu,"fin\n")==0){
        end_tchat=1;
        printf("client 2 a fermé le tchat");
      }
    }
    
    //si fin du tchat
    else{
      break;
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
		for (int i = 0; i < NbSalon; i++){
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

      pthread_t threadClient1;
			struct thread_args argsClient1;
			argsClient1.numSalon = salonChoisit;
			argsClient1.socketServer = dS;
			argsClient1.socketClient1 = salons[salonChoisit].socketClient1;
			argsClient1.socketClient2 = salons[salonChoisit].socketClient2;
			pthread_create(&threadClient1, 0, C1ToC2, &argsClient1);

			pthread_t threadClient2;
			struct thread_args argsClient2;
			argsClient2.numSalon = salonChoisit;
			argsClient2.socketServer = dS;
			argsClient2.socketClient1 = salons[salonChoisit].socketClient2;
			argsClient2.socketClient2 = salons[salonChoisit].socketClient1;
      pthread_create(&threadClient2, 0, C2ToC1, &argsClient2);

      pthread_join(threadClient1, 0);
      pthread_join(threadClient2, 0);

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
