#include <signal.h>

/* RemoteServer.c
   Se introducen las primitivas necesarias para establecer una conexión simple
   dentro del lenguaje C utilizando sockets.
 */
/* Cabeceras de Sockets */
#include <sys/types.h>
#include <sys/socket.h>
/* Cabecera de direcciones por red */
#include <netinet/in.h>
/**********/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

//** creaciion de hilos
#include <pthread.h>

#define LISTEN_MAX 5

/*
 * En la cabecera que incluimos `<netinet/in.h>`, una dirección sockaddr_in se
 * define como:
 * struct sockaddr_in {
 *      short          sin_family;
 *      u_short        sin_port;
 *      struct in_addr sin_addr;
 *      char           sin_zero[8];
 * };
 *
 */

/*
 * El servidor va a tomar el puerto como argumento.
 */

void error(char *msg){
  exit((perror(msg), 1));
}

int indexsoclientConectados = 0;
int soclientConectados[LISTEN_MAX] = {0};

// void reenviarMsj(int soclient, char* msj){
//   int i=0;
//   while (i < indexsoclientConectados ) {
//     if(soclientConectados[i] != soclient){
//       printf("%s\n",msj);
//       send(soclientConectados[i],msj,sizeof(msj), 0);
//     }
//     i++;
//   }
// }


void* administradorDeUsuario(void* dato) {
  int soclient = *((int*)dato);

  // tempDelMsj
  char buf[1024]="🤖 Online, escribe algo";

  /*Connection Successful*/
  send(soclient,buf,sizeof(buf), 0);

  printf("Connected! new Cliente: >>");


  while (strcmp(buf,"quit")!= 0) {
      recv(soclient, buf, sizeof(buf), 0);
      printf("%s\n",buf);
      // reenviarMsj(soclient,buf); no funciona bien si lo invoco por eso el codigo de abajo
      // reenviarMsj a todos los clientes
      if(strcmp(buf,"quit")!= 0){
        int i = 0;
        while (i < indexsoclientConectados ) {
          if(soclientConectados[i] != soclient){
            send(soclientConectados[i],buf,sizeof(buf), 0);
          }
          i++;
        }
      }
  }
  printf("Cliente desconectado\n");

}

int SOCK;
void manejadorSenal(int sig){

   int i = 0;
   while (i < indexsoclientConectados ) {
       send(soclientConectados[i],"quit", sizeof("quit"),0);
     i++;
   }
   i = 0;
   while (i < indexsoclientConectados ) {
       close(soclientConectados[i]);
     i++;
   }

   exit(0);

}


int main(int argc, char **argv){
  signal(SIGINT, manejadorSenal); //para poder salir sin romper


  // ** arreglo para almacenar lo que requieran los hilos
  pthread_t nhilos[LISTEN_MAX];

  int sock, soclient;

  struct sockaddr_in servidor, clientedir;
  socklen_t clientelen;

  if (argc <= 1) error("Faltan argumentos");

  /* Creamos el socket */
  if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    error("Socket Init");

  /* Creamos a la dirección del servidor.*/
  servidor.sin_family = AF_INET; /* Internet */
  servidor.sin_addr.s_addr = INADDR_ANY; /**/
  servidor.sin_port = htons(atoi(argv[1]));

  /* Inicializamos el socket */
  if (bind(sock, (struct sockaddr *) &servidor, sizeof(servidor)))
    error("Error en el bind");

  printf("Binding successful, and listening on %s\n",argv[1]);


  // **para poder recibir varios clientes
  int i=0; // indice del arreglo de hilos
  while (1) {
    /* Ya podemos aceptar conexiones */
    if(listen(sock, LISTEN_MAX) == -1){
      perror(" Listen error ");
      exit(1);
    }

    /* Now we can accept connections as they come*/
    clientelen = sizeof(clientedir);
    if ((soclient = accept(sock, (struct sockaddr *) &clientedir
                           , &clientelen)) == -1){
      perror("Accepting error");
      exit(1);
    }

    // añadimos el new client al arreglo
    soclientConectados[indexsoclientConectados] = soclient;
    indexsoclientConectados++;

    // **Creamos el hilo e invocamos al q se encargara de atender al cliente
    pthread_create(&nhilos[i],NULL,administradorDeUsuario,&soclient);
    i++;

  }

  return 0;
}
