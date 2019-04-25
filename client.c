#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

#define taille 150
/*taille choisit abritrairement*/

#define port 10400

int end(char* message){
  int fin=strncmp("fin", message,3);
  if (fin==0){
    printf("> Fin du tchat \n");
    return 1;
  } else {
    return 0;
  }

}

int main(int argc, char const *argv[]){

	if (argc < 2) {
		printf("Pas le bon nombre d'argument \n");
		return 0;
	}

	char receiv[taille];
	char sender[taille];
	int end_tchat = 0;
	int res;

	int dS = socket(PF_INET, SOCK_STREAM , 0);  /* creation socket, IPv4, protocole TCP */
	if (dS<0){
    	perror("Erreur creation de socket");
  	}
	struct sockaddr_in adServ;
	adServ.sin_family = AF_INET;
	adServ.sin_port = htons(port);  /* port */

	res = inet_pton(AF_INET, argv[1], &(adServ.sin_addr));
  
	if (res == -1) {
		printf("/////// error creating client /////// \n");
	} else {
		printf("/////// success creating client ///////\n");
	}
	
	socklen_t lgA = sizeof(struct sockaddr_in);
	res = connect(dS, (struct sockaddr *) &adServ, lgA); /*demande de co au serveur*/
	if (res!=0) {
		perror("error connecting to server \n");
	}

	/*attente message du server*/

	res = recv(dS,receiv,taille,0);
	if (res<0) {
		printf("error receiving from server \n");
	}
	printf("> %s\n",receiv);

  while(!end_tchat){
  	bzero(receiv, taille);
  	printf("\n /////// En attente d'un message /////// \n");
    res = recv(dS,receiv,taille,0);
	if (res<0) {
		printf("error receiving from server \n");
	}
	printf("\n > %s\n",receiv);

	end_tchat=end(receiv);
	if(!end_tchat){
		printf("\n /////// Tapez votre message /////// \n");
		fgets(sender,256,stdin);
    	char *pos1 = strchr(sender, '\n');
    	*pos1 = '\0';
    
    	send(dS,sender,sizeof(sender),0);

    	end_tchat=end(sender);
	}
  } /* boucle de tchat qui continue tant que l'un des deux clients n'a pas tappe "fin [...]" */

	int c = close(dS);
	if (c != 0){
        perror("Erreur fermeture socket");
    }
	return res;

}
