#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define taille 256 /*taille choisit abritrairement*/
#define port 10400

int dS;
int res;


void *receivMessage() {
	char* receiv = (char*) malloc(taille * sizeof(char));
	receiv[0]='\0';
  
  res = recv(dS,receiv,taille,0);
  if (res<0) {
    printf("error receiving from server \n");
  }
  printf("> %s\n",receiv);

  while(1){
    bzero(receiv, taille);

    res = recv(dS,receiv,taille,0);
    if (res<0) {
      printf("error receiving from server \n");
    }
    printf("reçu > %s\n",receiv);
    int fin=strcmp("fin\0", receiv);
    if (fin==0){
      free(receiv);
      kill(getpid(),SIGUSR1);
      break;
    }
  }
}

void *sendMessage() {

	char* sender = (char*) malloc(taille * sizeof(char));
	sender[0]='\0';

  while(1){
    fgets(sender,256,stdin);
    char *pos1 = strchr(sender, '\n');
    *pos1 = '\0';
    
    send(dS,sender,strlen(sender),0);

    int fin=strcmp("fin\0", sender);
    if (fin==0){
      free(sender);
      kill(getpid(),SIGUSR1);
      break;
    }
  }
}

void handler() {
  int c = close(dS);
  if (c != 0){
    perror("Erreur fermeture socket");
  }
  else {
    printf("//////////Fin du chat//////////\n");
    exit(0);
  }
}

int main(int argc, char const *argv[]){

  signal(SIGUSR1,handler);

  if (argc < 2) {
    printf("Pas le bon nombre d'argument \n");
    return 0;
  }

  dS = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
  if (dS<0){
      perror("Erreur creation de socket");
    }
  struct sockaddr_in adServ;
  adServ.sin_family = AF_INET;
  adServ.sin_port = htons(port);  /* fait reference a la variable globale port*/

  res = inet_pton(AF_INET, argv[1], &(adServ.sin_addr)); //creation client et verification
  if (res == -1) {
    printf("/////// error creating client /////// \n");
  } else {
    printf("/////// success creating client ///////\n");
  }
  
  socklen_t lgA = sizeof(struct sockaddr_in);
  res = connect(dS, (struct sockaddr *) &adServ, lgA); /*demande de connexion au serveur*/
  if (res!=0) {
    printf("error connecting to server \n");
    return 0;
  }


  //creation de 2 threads differents pour gerer l'envoi et la reception de message simultanement
  pthread_t tRecevoir;
  pthread_t tEnvoyer;

  pthread_create(&tRecevoir,0,receivMessage,0);
  pthread_create(&tEnvoyer,0,sendMessage,0);

  pthread_join(tRecevoir,0);
  pthread_join(tEnvoyer,0);

  return res;
}