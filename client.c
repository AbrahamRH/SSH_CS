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
  if (argc != 3)
  {
    fprintf(stderr, "Uso: %s hostname puerto\n", argv[0]);
    exit(1);
  }

  int sockfd;
  struct hostent *he;
  struct sockaddr_in cliente; // Información de la dirección de destino

  if ((he = gethostbyname(argv[1])) == NULL)
  { // Obtener información de host servidor
    perror("gethostbyname");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }

  cliente.sin_family = AF_INET;                // Ordenación de bytes de la máquina
  cliente.sin_port = htons(atoi(argv[2]));    // Short, ordenación de bytes de la red
  cliente.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(cliente.sin_zero), '\0', 8);       // Poner a cero el resto de la estructura

  if (connect(sockfd, (struct sockaddr *)&cliente, sizeof(struct sockaddr)) == -1)
  {
    perror("connect");
    exit(1);
  }

  char comando[MAXDATASIZE];
  int len_comando;

  while (strcmp(comando,"salir") != 0)
  {
    printf("[%s@%s]: ", argv[1], argv[2]);
    fgets(comando, MAXDATASIZE, stdin);
    len_comando = strlen(comando) - 1;
    comando[len_comando] = '\0';

    /* Se envía el comando al servidor */
    if (send(sockfd, comando, len_comando, 0) == -1)
    {
      perror("send()");
      exit(1);
    }

    int numbytes;
    char buf[MAXDATASIZE_RESP];

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE_RESP - 1, 0)) == -1)
    {
      perror("recv");
      exit(1);
    }

    if (numbytes == 0)
    {
      printf("El servidor cerró la conexión\n");
      break;
    }

    buf[numbytes] = '\0';
    printf("%s\n", buf);
    sleep(1);
  }

  close(sockfd);

  return 0;
}
