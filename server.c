#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define LENGTH 20000
#define MSG_SIZE 255
#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[])
{
  if (argc <= 1)
  {
    printf("Se requiere especificar el puerto.\n");
    exit(-1);
  }

  int numbytes;
  char buf[100];

  int server_fd, cliente_fd;

  struct sockaddr_in servidor; // información sobre mi direccion (servidor)
  struct sockaddr_in cliente;  // información sobre la dirección del cliente

  // La longitud o tamaño de servidor y de cliente
  socklen_t sin_size_servidor;
  socklen_t sin_size_cliente;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
  {
    perror("Server-setsockopt() error!");
    exit(1);
  }

  servidor.sin_family = AF_INET;                // Ordenación de bytes de la máquina
  servidor.sin_port = htons(atoi(argv[1]));    // short, Ordenación de bytes de la red
  servidor.sin_addr.s_addr = INADDR_ANY;        // Rellenar con mi dirección IP
  memset(&(servidor.sin_zero), '\0', 8);       // Poner a cero el resto de la estructura

  sin_size_servidor = sizeof(servidor);
  if (bind(server_fd, (struct sockaddr *)&servidor, sin_size_servidor) == -1)
  {
    perror("bind");
    exit(1);
  }

  if (listen(server_fd, 1) == -1)
  {
    perror("listen");
    exit(1);
  }

  sin_size_cliente = sizeof(cliente);

  FILE *popenFile;
  char output[LENGTH];
  char response[LENGTH];

  while ((cliente_fd = accept(server_fd, (struct sockaddr *)&cliente, &sin_size_cliente)) > 0)
  {
    printf("[Server]: conexión cliente desde %s\n", inet_ntoa(cliente.sin_addr));

    // child == 0 es la lógica del hijo
    if (fork() == 0)
    {
      close(server_fd); // Cerrar el socket del servidor en el proceso hijo

      while ((numbytes = recv(cliente_fd, buf, sizeof(buf) - 1, 0)) > 0)
      {
        buf[numbytes] = '\0';
        FILE *popenFile = NULL;
        if(strcmp(buf,"salir") != 0){
          popenFile = popen(buf, "r");
          if (popenFile == NULL)
          {
            perror("popen");
            exit(EXIT_FAILURE);
          }
        }else{
          char* response = "Cerrando conexion";
          send(cliente_fd, response, strlen(response), 0);
          close(server_fd);
          close(cliente_fd);
          exit(0);
        }
        while (fgets(output, LENGTH, popenFile))
        {
          if (send(cliente_fd, output, strlen(output), 0) == -1)
          {
            perror("send");
            exit(EXIT_FAILURE);
          }
        }
        pclose(popenFile);
      }

      if (numbytes == -1)
      {
        perror("recv");
        exit(EXIT_FAILURE);
      }

      exit(0); // Salir del proceso hijo
    }

    close(cliente_fd); // Cerrar el socket del cliente en el proceso padre
  }

  if (cliente_fd == -1)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  printf("[Server]: Cerrando conexión con el cliente.\n");
  close(server_fd);
  exit(0);
}
