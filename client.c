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
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    if(strcmp(receiv, "file") == 0){
			    int tailleNom;
			    int tailleContenu;
			    char nomFichier[taille];
			    char contenuFichier[taille];
			    char* res2;
			    printf("Prêt à recevoir un ficher...\n");

			    res = recv(dS, &tailleNom, sizeof(int),0); /* Réception de la taille du nom du fichier */

			    if(res < 0){
				    perror("Problème lors de la réception de la taille du nom du fichier");
				    exit(1);
			    } else if(res == 0){
				    perror("Socket fermé");
				    exit(0);
			    }

			    res = recv(dS, &nomFichier, tailleNom,0); /* Réception du nom du fichier */

			    if(res < 0){
				    perror("Problème lors de la réception du nom du fichier");
				    exit(1);
			    } else if(res == 0){
				    perror("Socket fermé");
				    exit(0);
			    }

			    printf("Nom du fichier : %s\n", nomFichier); /* Affichage du nom du fichier */

			    res = recv(dS, &tailleContenu, sizeof(int),0); /* Réception de la taille du contenu du fichier */

			    if(res < 0){
				    perror("Problème lors de la réception de la taille du contenu du fichier");
				    exit(1);
			    } else if(res == 0){
				    perror("Socket fermé");
				    exit(1);
			    }

			    res = recv(dS, &contenuFichier, tailleContenu,0); /* Réception du contenu du fichier */

			    if(res < 0){
				    perror("Problème lors de la réception du contenu du fichier");
				    exit(1);
			    } else if(res == 0){
				    perror("Socket fermé");
				    exit(1);
			    }

			    contenuFichier[tailleContenu] = '\0';

			    printf("Contenu du fichier : %s\n", contenuFichier); /* Affichage du nom du fichier */

			    FILE* newFile = NULL;

        		newFile = fopen(nomFichier, "a+");

			    if (newFile != NULL){
				    res2 = fputs(contenuFichier, newFile);
				    if(res2 == EOF){
					    perror("Erreur lors de l'écriture dans le fichier");
				    }
				    fclose(newFile);
			    } else {
				    perror("Erreur lors de la création/ouverture du fichier");
			    }

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
	  if(strcmp(sender, "file") == 0){
		  char chaine [taille]="";
		  char copie [taille] = {0};
		  char nomFichier [15];
		  int tailleNom;
		  int tailleContenu;

		  DIR * rep = opendir (".");
		  if (rep != NULL){
			  struct dirent * ent;
			  while ((ent = readdir (rep)) != NULL){
				  printf ("%s\n", ent->d_name);
			  }
			  closedir (rep);
		  } else {
			perror ("le dossier est vide ou n'existe pas\n");
		}
		printf("Quel fichier voulez vous envoyer ? \n");
		fgets(nomFichier, taille, stdin);
		*strchr(nomFichier, '\n') = '\0';

			tailleNom = strlen(nomFichier);
			res = send(dS,&tailleNom,sizeof(int),0); /* Envoie de la taille du nom du fichier */

			if (res<0){
				perror("Taille du nom du fichier non envoyé");
				exit(1);
			} else if(res == 0){
				perror("Socket fermée");
				exit(0);
			}

			res = send(dS,&nomFichier,tailleNom,0); /* Envoie du nom du fichier */

			if (res<0){
				perror("Nom du fichier non envoyé");
				exit(1);
			} else if(res == 0){
				perror("Socket fermée");
				exit(0);
			}

			FILE* file = NULL;
			file = fopen(nomFichier, "r");

			while (fgets(chaine, taille, file) != NULL){
				printf("Données copiées : %s\n",chaine);
				strcat(copie, chaine);  /* pour pouvoir concaténer toutes les lignes du fichier */
			}

			fclose(file);
			
			tailleContenu = strlen(chaine);
			
			res = send(dS,&tailleContenu,sizeof(int),0); /* Envoie de la taille du contenu du fichier */

			if (res<0){
				perror("Taille du contenu du fichier non envoyé");
				exit(1);
			} else if(res == 0){
				perror("Socket fermée");
				exit(0);
			}

			res = send(dS,&chaine,strlen(chaine),0); /* Envoie du contenu du fichier */

			if(res<0){
				perror("Contenu du fichier non envoyé");
				exit(1);
			} else if(res == 0){
				perror("Socket fermée");
				exit(0);
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
