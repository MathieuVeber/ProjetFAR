#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

#define taille 150
/*taille choisit abritrairement ici*/

#define port 10400
#define id1 0
#define id2 1

int end(char* message){
  int fin=strncmp("fin", message,3);
  if (fin==0){
    printf("/////// Fin du tchat /////// \n");
    return 1;
  } else {
    return 0;
  }

}

int main(){
  int end_tchat;
  char message[taille];
  int cl1;
  int cl2;
  int idclient;
  
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
    end_tchat=0;

    /*connexion client 1*/
    cl1 = accept(dS, (struct sockaddr*) &aC,&lg);
    if (cl1 < 0){
      perror("Erreur, client 1 non accepte");
    }
    printf("Client 1 connecte \n");
    sprintf(message, "Bienvenue client 1 ! \n");
    int s = send(cl1, message, taille, 0);
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

    /*debut tchat*/

    sprintf(message,"> Client 2 connecte, debut du tchat \n");
    send(cl1, message, taille, 0); 

    idclient=0;    
    while(!end_tchat){
      bzero(message, taille);
      if (idclient==0){
        int r = recv(cl1,message,taille,0);
        if (r < 0){
          perror("Erreur reception client 1");
        }
        r = send(cl2,message,strlen(message),0);
        if (r < 0){
          perror("Erreur envoi client 2");
        }
        idclient=id2;
      } else {
        int s = recv(cl2,message,taille,0);
        if (s < 0){
          perror("Erreur reception client 2");
        }
        s = send(cl1,message,strlen(message),0);
        if (s < 0){
          perror("Erreur envoi client 1");
        }
        idclient=id1;        
      }

      /*si fin du tchat*/

      if(end(message)){
        int c = close(cl1);
        if (c != 0){
          perror("Erreur fermeture cl1");
        }
        c = close(cl2);
        if (c != 0){
          perror("Erreur fermeture cl2");
        }
        end_tchat=1;
      }
      
    }
    printf("En attente de connexion \n");
    
    
  }
  
  int c = close (dS);
  if (c != 0){
    perror("Erreur fermeture socket");
  }

	return 0;
}