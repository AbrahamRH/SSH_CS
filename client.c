 /*
 ** client.c -- Ejemplo de cliente de sockets de flujo
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #define MAXDATASIZE 100
 #define MAXDATASIZE_RESP 20000

 int main(int argc, char *argv[])
 {

  if(argc <= 2){
    printf("Es necesario especificar el host y el puerto");
    exit(-1);
  }
  // Estos 2 son para el comando
  char comando[MAXDATASIZE];
  int len_comando;

  // Estos 2 son para la respuesta
  int numbytes;
  char buf[MAXDATASIZE_RESP];

  int sockfd;  
  struct hostent *he;
  struct sockaddr_in cliente; // información de la dirección de destino 

  if (argc != 3) {
    fprintf(stderr,"usage: client hostname puerto\n");
    exit(1);
  }

  if ((he=gethostbyname(argv[1])) == NULL) {  // obtener información de host servidor 
   perror("gethostbyname");
   exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
   perror("socket");
   exit(1);
  }

  cliente.sin_family = AF_INET;    // Ordenación de bytes de la máquina 
  cliente.sin_port = htons( atoi(argv[2]) );  // short, Ordenación de bytes de la red 
  cliente.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(cliente.sin_zero), '\0',8);  // poner a cero el resto de la estructura 

  if (connect(sockfd, (struct sockaddr *)&cliente, sizeof(struct sockaddr)) == -1) {
   perror("connect");
   exit(1);
  }

  while( strcmp(comando, "exit\n") != 0 ){
    fgets(comando,MAXDATASIZE-1,stdin);
    len_comando = strlen(comando) - 1;
    comando[len_comando] = '\0';
    printf("[Client]: Comando: %s\n",comando);
    /* Se envia el comando al server */
    if(send(sockfd,comando, len_comando, 0) == -1) {
      perror("send()");
      exit(1);
    } else 
      printf("[Client]: Comando enviado...\n");

    if ((numbytes=recv(sockfd, buf, MAXDATASIZE_RESP-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    buf[numbytes] = '\0';
    printf("Recibido:\n%s\n",buf);
  }
  close(sockfd);

  return 0;
} 
