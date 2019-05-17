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
#define portServer 10400
#define id1 0
#define id2 1

int cl;
int cl1;
int cl2;
int s;
char message[taille];
char *listSaloon[]={
	"1 : projetWeb",
	"2 : PLS",
	"3 : pinou"
};
int port=20400;
int NumSaloon[3]={0,0,0};

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
				perror("soket fermée");
				exit(1);
			}

			res = recv(client1, &nomFichier, tailleNom,0); /* Réception du nom du fichier */

			if(res < 0){
				perror("Problème lors de la réception du nom du fichier");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			nomFichier[tailleNom] = '\0';

			printf("Nom du fichier : %s\n", nomFichier); /* Affichage du nom du fichier */

			res = send(client2, &tailleNom, sizeof(int),0);

			if(res < 0){
				perror("Problème lors de l'envoi de la taille du nom du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			res = send(client2, &nomFichier, tailleNom,0);

			if(res < 0){
				perror("Problème lors de l'envoi du nom du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			res = recv(client1, &tailleContenu, sizeof(int),0); /* Réception de la taille du contenu du fichier */

			if(res < 0){
				perror("Problème lors de la réception de la taille du contenu du fichier");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			res = recv(client1, &contenuFichier, tailleContenu,0); /* Réception du contenu du fichier */

			if(res < 0){
				perror("Problème lors de la réception du contenu du fichier");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			contenuFichier[tailleContenu] = '\0';

			printf("Contenu du fichier : %s\n", contenuFichier); /* Affichage du nom du fichier */

			res = send(client2, &tailleContenu, sizeof(int),0);

			if(res < 0){
				perror("Problème lors de l'envoi de la taille du contenu du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
			}

			res = send(client2, &contenuFichier, tailleContenu,0);

			if(res < 0){
				perror("Problème lors de l'envoi du contenu du fichier au client2");
				exit(1);
			} else if(res == 0){
				perror("soket fermée");
				exit(1);
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
    r = send(cl2,message,strlen(message),0);
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


void *salon(){

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
    sprintf(message, "Bienvenue client 1 ! \n");
    s = send(cl1, message, taille, 0);
    if (s<0){
      perror("Erreur de transmission \n");
    }

    /*conexion client 2*/

    cl2 = accept(dS, (struct sockaddr*) &aC,&lg);
    if (cl2 < 0){
      perror("Erreur, client 1 non accepte");
    }
    printf("Client 2 connecté \n");
    sprintf(message, "Bienvenue client 2 ! \n");
    s=send(cl2, message, taille, 0);
    if (s<0){
      perror("Erreur de transmission \n");
    }

    sprintf(message,"> Client 2 connecte, debut du tchat \n");
    send(cl1, message, taille, 0);

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
}

int main(int argc, char const *argv[])
{
	  /*creation socket*/

	int dS = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
  if (dS<0){
    perror("Erreur creation de socket");
  }
	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(portServer); /*port*/

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

	    /*connexion client*/
	    cl = accept(dS, (struct sockaddr*) &aC,&lg);
	    if (cl < 0){
	      perror("Erreur, client non accepte");
	    }
	    printf("Client connecte \n");
	    sprintf(message, "Bienvenue client ! \n");
	    s = send(cl, message, taille, 0);
	    if (s<0){
	      perror("Erreur de transmission \n");
	    }

	    //envoi liste salon
	    s=send(cl,listSaloon,sizeof(listSaloon)+1,0);
	    if (s<0){
	      perror("Erreur de transmission \n");
	    }

	    //reception du choix
	    bzero(message,taille);
	    s=recv(cl,message,taille,0);
	    if (s < 0){
	      perror("Erreur reception client 2");
	    }

	    if(atoi(message)>0 && atoi(message)<4){
	    	port+=(atoi(message));
	    	if (NumSaloon[atoi(message)+1]==0){
	    		NumSaloon[atoi(message)+1]+=1;
	    		bzero(message,taille);
	    		sprintf(message,"%d",port);
	    		send(cl,&message,taille,0);
	    		pthread_t saloon;
	    		pthread_create(&saloon,0,saloon,port);
	    		pthread_join(saloon,0);
	    	}
	    	else if (NumSaloon[atoi(message)+1]==1)
	    	{
	    		NumSaloon[atoi(message)+1]+=1;
	    		bzero(message,taille);
	    		sprintf(message,"%d",port);
	    		send(cl,&message,taille,0);
	       	}	
	    }

	}

	return 0;
}
