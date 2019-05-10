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
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include "struct.h"

#define port 10400

int dS;
int cl;
int check;
struct message receiv;
struct message sender;
struct message the_file; //servira de reception pour fichier
pthread_mutex_t envoiMutex; 

FILE* new_tty() {
  pthread_mutex_t the_mutex;
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal"); sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  int i;
  for(i = strlen(tty_name)-1; i >= 0; i--) {
    if(tty_name[i] == '/') break;
  }
  tty_name[i+1] = '\0';
  strcat(tty_name,str);
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}

int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("Impossible d'exécuter la commande\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }
  pclose(fp);
  return i;
}

/*void *sendFile(){
  FILE *fp1=new_tty();
  fprintf(fp1, "%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

  //demander à l'utilisateur de choisir le fichier
  DIR *dp;
  struct dirent *ep;     
  dp = opendir ("./");
  if (dp != NULL) {
    fprintf(fp1,"Voilà la liste de fichiers :\n");
    while (ep = readdir (dp)) {
      if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) 
  fprintf(fp1,"%s\n",ep->d_name);
    }    
    (void) closedir (dp);
  }
  else {
    perror ("Ne peux pas ouvrir le répertoire");
  }
  printf("Indiquer le nom du fichier : ");
  char fileName[1023];
  fgets(fileName,sizeof(fileName),stdin);

  pthread_mutex_unlock(&envoiMutex);

  fileName[strlen(fileName)-1]='\0';
  FILE *fps = fopen(fileName, "r");
  if (fps == NULL){
    printf("Ne peux pas ouvrir le fichier suivant : %s",fileName);
  }
  else {  
  sender.etiquette=1;
  strcpy(sender.contenu,fileName);
  check=send(dS,&sender,sizeof(sender),0); //envoie le nom du fichier
  if (check<0){
    perror("Erreur d'envoi");
  }

  char sendByBlock[2000];
  while(fgets(sendByBlock,2000,fps)!=NULL){
    sender.etiquette=1;
    strcpy(sender.contenu,sendByBlock);
    check=send(dS,&sender,sizeof(sender)+1,0);
    if (check<0){
    perror("Erreur d'envoi");
    }
  }
  sender.etiquette=1;
  check=send(dS,&sender,sizeof(sender)+1,0);
  if (check<0){
    perror("Erreur d'envoi");
  }
  printf("fichier envoyé");
  }
  fclose(fps);    
}*/

/*void *receivFile(){
  FILE *fp2;
  while(1){
    check=recv(dS,&the_file,sizeof(the_file)+1,0);
    if (check<0) {
      printf("erreur reception \n");
      break;
    }
  }
}*/

void *sendMessage() {

  while(1){
    pthread_mutex_lock(&envoiMutex);
    fgets(sender.contenu,200,stdin);
    pthread_mutex_unlock(&envoiMutex);

    char *pos1 = strchr(sender.contenu, '\n');
    *pos1 = '\0';

    // Si envoie du fichier
    if(strcmp("file\0", sender.contenu)==0){
      pthread_t tSendFile;
      check=send(dS,sender.contenu,sizeof(sender)+1,0);
      if (check<0) {
      printf("erreur d'envoi\n");
      }
      //pthread_create(&tSendFile,0,sendFile,0);
      pthread_mutex_lock(&envoiMutex);
    }
    else {
      sender.etiquette=0; //message classique
      check=send(dS,sender.contenu,sizeof(sender)+1,0);
      if (check<0) {
      printf("erreur d'envoi\n");
      }
      // Si fin de la conversation
      if(strcmp("fin\n", sender.contenu)==0){
      kill(getpid(),SIGUSR1);
      break;
      }
    }
    
  }
}

void *receivMessage() {

  check = recv(dS,&receiv,sizeof(receiv)+1,0);
  if (check<0) {
    printf("erreur de reception \n");
  }
  printf("> %s\n",receiv.contenu);

  while(1){

    check = recv(dS,&receiv,sizeof(receiv)+1,MSG_PEEK);
    if (check<0) {
      printf("erreur de reception \n");
    }

    if (receiv.etiquette=0){
      check = recv(dS,&receiv,sizeof(receiv)+1,0);
      if (check<0) {
      printf("erreur de reception \n");
      }
      printf("reçu : %s\n",receiv.contenu);
      // Si fin de la conversation
      if(strcmp("fin\0", receiv.contenu)==0){
        kill(getpid(),SIGUSR1);
        break;
      }
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

  check = inet_pton(AF_INET, argv[1], &(adServ.sin_addr)); //creation client et verification
  if (check == -1) {
    printf("/////// error creating client /////// \n");
  } else {
    printf("/////// success creating client ///////\n");
  }

  socklen_t lgA = sizeof(struct sockaddr_in);
  check = connect(dS, (struct sockaddr *) &adServ, lgA); /*demande de connexion au serveur*/
  if (check!=0) {
    printf("error connecting to server \n");
    return 0;
  }


  //creation de 2 threads differents pour gerer l'envoi et la reception de message simultanement
  pthread_t tRecevoir;
  pthread_t tEnvoyer;
  pthread_t tRecevoirFichier;

  pthread_create(&tRecevoir,0,receivMessage,0);
  pthread_create(&tEnvoyer,0,sendMessage,0);
  /*pthread_create(&tRecevoirFichier,0,receivFile,0);*/

  pthread_join(tRecevoir,0);
  pthread_join(tEnvoyer,0);
  pthread_join(tRecevoirFichier,0);

  return check;
}