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
#define port 10400
#define id1 0
#define id2 1

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
        perror("Erreur reception client 1");
      }

      //on va le renvoyer au client 1
      struct message msg_send;
      msg_send.etiquette = the_message.etiquette;
      if (strcmp(the_message.contenu, "file\n")==0){
        msg_send.etiquette=1;
      }
    
      r = send(cl1,&msg_send,sizeof(msg_send)+1,0);
      if (r < 0){
        perror("Erreur envoi client 2");
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
      int r = recv(cl1,&the_message,sizeof(the_message)+1,0); 
      if (r < 0){
        perror("Erreur reception client 1");
      }

      //on va le renvoyer au client 2
      struct message msg_send;
      msg_send.etiquette = the_message.etiquette;
      if (strcmp(the_message.contenu, "file\n")==0){
        msg_send.etiquette=1;
      }
    
      r = send(cl2,&msg_send,sizeof(msg_send)+1,0);
      if (r < 0){
        perror("Erreur envoi client 2");
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
    perror("Erreur creation de socket");
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

  struct sockaddr_in aC ;
  socklen_t lg = sizeof(struct sockaddr_in) ;

  while(1){

    /*connexion client 1*/
    cl1 = accept(dS, (struct sockaddr*) &aC,&lg);
    if (cl1 < 0){
      perror("Erreur, client 1 non accepte");
    }
    printf("Client 1 connecte \n");
    sprintf(msg.contenu, "Bienvenue client 1 ! \n");
    s = send(cl1,&msg, sizeof(msg)+1, 0);
    if (s<0){
      perror("Erreur de transmission \n");
    }

    /*conexion client 2*/

    cl2 = accept(dS, (struct sockaddr*) &aC,&lg);
    if (cl2 < 0){
      perror("Erreur, client 1 non accepte");
    }
    
    printf("Client 2 connecté \n");
    
    sprintf(msg.contenu, "Bienvenue client 2 ! \n");
    s=send(cl2,&msg, sizeof(msg)+1, 0);
    if (s<0){
      perror("Erreur de transmission \n");
    }
    
    printf("ok");
    
    sprintf(msg.contenu,"> Client 2 connecte, debut du tchat \n");
    s=send(cl1,&msg, sizeof(msg)+1, 0);
    if (s<0){
      printf("Erreur de transmission \n");
    }

    // Debut du tchat avec les threads
    pthread_t tC1ToC2;
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

    printf("En attente de connexion \n");

  }

  int c = close (dS);
  if (c != 0){
    perror("Erreur fermeture socket");
  }

	return 0;
}
