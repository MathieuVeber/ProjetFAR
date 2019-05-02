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
#include <fcntl.h>
#include <sys/sendfile.h>


#define taille 256 /*taille choisit abritrairement*/
#define port 10400

int dS; // en tant que client avec le serveur
int dS2; // en tant que serveur avec le client
int dS3; // en tant que client avec le client
int res;
int cl;
int res3;



// TRANSFERT DE FICHIER //


void *receivFile(void *arg) {
  ssize_t len;
  char buffer[BUFSIZ];
  int file_size;
  FILE *received_file;
  int remain_data = 0;

  dS3 = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
  if (dS3<0){
      perror("Erreur creation de socket");
    }
  struct sockaddr_in adServ2;
  adServ2.sin_family = AF_INET;
  adServ2.sin_port = htons(arg[1]);  /* fait reference au port passe en parametre*/

  res2 = inet_pton(AF_INET, argv[0], &(adServ2.sin_addr)); //creation client et verification
  if (res2 == -1) {
    printf("/////// error creating client /////// \n");
  } else {
    printf("/////// success creating client ///////\n");
  }

  socklen_t lgA = sizeof(struct sockaddr_in);
  res = connect(dS3, (struct sockaddr *) &adServ2, lgA); /*demande de connexion au serveur*/
  if (res2!=0) {
    printf("error connecting to server \n");
    return 0;
  }

  // Taille du fichier
  recv(dS3, buffer, BUFSIZ, 0);
  file_size = atoi(buffer);

  // Réception du fichier
  received_file = fopen("/dir/lapin", "w"); // <!> fichier d'arrivée à définir
  if (received_file == NULL) {
          fprintf(stderr, "Erreur d'ouverture du fichier %s\n", strerror(errno));
          exit(EXIT_FAILURE);
  }

  remain_data = file_size;

  while ((remain_data > 0) && ((len = recv(dS3, buffer, BUFSIZ, 0)) > 0)) {
          fwrite(buffer, sizeof(char), len, received_file);
          remain_data -= len;
  }

  fclose(received_file);
  close(dS3);
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



void *sendFile() {

  // SECTION SERVEUR //

  int peer_socket;
  socklen_t sock_len;
  ssize_t len;
  struct sockaddr_in peer_addr;
  int fd;
  int sent_bytes = 0;
  char file_size[256];
  struct stat file_stat;
  int offset;
  int remain_data;

  // creation socket
	int dS2 = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
  if (dS2<0){
    perror("Erreur creation de socket");
  }
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(port); /*port*/

  int b = bind(dS2, (struct sockaddr*)&ad, sizeof(ad));
  if (b!=0){
    perror("Erreur de nommage socket \n");
  }
  int l = listen(dS2,7);
  if (l != 0){
      perror("Erreur de listening \n");
    }

  struct sockaddr_in aC ;
  socklen_t lg = sizeof(struct sockaddr_in) ;

  // Connexion client
  cl = accept(dS2, (struct sockaddr*) &aC,&lg);
  if (cl < 0){
    perror("Erreur, client non accepte");
  }

  // SECTION DIALOGUE UTILISATEUR //

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

  // SECTION ENVOIE DU FICHIER //

  // Traitement fichier à envoyer
  fd = open(fileName, O_RDONLY);
  if (fd == -1)
  {
          fprintf(stderr, "Erreur d'ouverture du fichier %s", strerror(errno));
          exit(EXIT_FAILURE);
  }

  // On récupère la taille du fichier
  if (fstat(fd, &file_stat) < 0)
  {
          fprintf(stderr, "Erreur fstat : %s", strerror(errno));
          exit(EXIT_FAILURE);
  }

  sock_len = sizeof(struct sockaddr_in);

  // Appairage
  peer_socket = accept(dS2, (struct sockaddr *)&peer_addr, &sock_len);
  if (peer_socket == -1)
  {
          fprintf(stderr, "Erreur d'accaptation %s", strerror(errno));
          exit(EXIT_FAILURE);
  }

  sprintf(file_size, "%d", file_stat.st_size);

  // Envoie de la taille du fichier
  len = send(peer_socket, file_size, sizeof(file_size), 0);
  if (len < 0)
  {
        fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));
        exit(EXIT_FAILURE);
  }

  offset = 0;
  remain_data = file_stat.st_size;

  // Envoie des données du fichier
  while (((sent_bytes = sendfile(peer_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)) {
          remain_data -= sent_bytes;
  }

  close(fd);
  close(peer_socket);
  close(dS2);

}



// TRANSFERT DE MESSAGE //


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
      /*
      Rajouter réception IP+Port à passer dans tableau
      */
      pthread_t tReceivFile;
      pthread_create(&tReceivFile,0,receivFile,0);
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



// MAIN //


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
