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

#define taille 256 /*taille choisit abritrairement*/
#define port 10400

int dS;
int res;

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

void *envoyerFichier() {
  FILE* fp1 = new_tty();
  fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

  // Demander à l'utilisateur quel fichier afficher
  DIR *dp;
  struct dirent *ep;
  dp = opendir ("./fichier"); // Dossier de départ
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
  fileName[strlen(fileName)-1]='\0';
  FILE *fps = fopen(fileName, "r");
  if (fps == NULL){
    printf("Ne peux pas ouvrir le fichier suivant : %s",fileName);
  }
  else {
    char str[1000];
    // Lire le contenu du fichier
    while (fgets(str, 1000, fps) != NULL) {
      fprintf(fp1,"%s",str); //ATTENTION Fichier d'arrivée
    }
  }
  fclose(fps);	
  return 0;
}

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

    // Se prépare pour recevoir un fichier  
    int file=strcmp("file\0", receiv);
    if (file==0){
      pthread_t tReceivFile;
      pthread_create(&tReceivFile,0,ReceivFile,0);
    }
    // Ou affiche le message reçu
    else {
      printf("reçu > %s\n",receiv);
    }

    // Si fin de la conversation
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

    // Si envoie du fichier
    int file=strcmp("file\0", sender);
    if (file==0){
      pthread_t tSendFile;
      pthread_create(&tSendFile,0,sendFile,0);
    }

    // Si fin de la conversation
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