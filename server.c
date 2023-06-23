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

  if(argc <= 1){
    printf("Se requiere especificar el puerto.");
    exit(-1);
  }

  int numbytes;
  char buf[100];

  int server_fd, cliente_fd;

  struct sockaddr_in servidor; // información sobre mi direccion (servidor)
  struct sockaddr_in cliente; // información sobre la dirección del cliente

  // La longitud o tamaño de servidor y de cliente
  int sin_size_servidor;
  int sin_size_cliente;

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

  servidor.sin_family = AF_INET;         // Ordenación de bytes de la máquina
  servidor.sin_port = htons( atoi(argv[1]) ); // short, Ordenación de bytes de la red
  servidor.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
  memset(&(servidor.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

  sin_size_servidor = sizeof( servidor );
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

  sin_size_cliente = sizeof( cliente );


 FILE* popenFile;
  char output[LENGTH];
  char response[LENGTH];
  
  int fd[2];
  pipe(fd);
  
  while((cliente_fd = accept(server_fd, (struct sockaddr *)&cliente, &sin_size_cliente)) > 0){  
  	printf("[Server]: conexion cliente desde %s\n", inet_ntoa(cliente.sin_addr));

	  // child == 0 es la logica del hijo
	  if(fork() == 0){
	  	
	    close(fd[READ_END]);
	    numbytes = recv(cliente_fd, buf, 100-1, 0);
	    if(numbytes == 0) break;

	    buf[numbytes] = '\0';
	    dup2(fd[WRITE_END],STDOUT_FILENO);
	    close(fd[WRITE_END]);

	    popenFile = popen(buf,"r");
	    while(fgets(output,LENGTH,popenFile)){
	      printf("%s",output);
	    }
	    pclose(popenFile);

	  } else {
	    close(fd[WRITE_END]);
	    dup2(fd[READ_END],STDIN_FILENO);
	    close(fd[READ_END]);
	    while (read(STDIN_FILENO, response, LENGTH) > 0 )	
	    {
	      if(send(cliente_fd,response,LENGTH,0) == -1) {
			perror("send");
			exit(EXIT_FAILURE);
	      }
	    }
	  }

	  
	  
  }
  printf("[Server]: Cerrando conexion con el cliente.\n");	
  close(cliente_fd);
  close(server_fd);
  shutdown(server_fd, SHUT_RDWR);
  exit(0);
}
