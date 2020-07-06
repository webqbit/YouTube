
#include <signal.h>


/* RemoteClient.c
   Se introducen las primitivas necesarias para establecer una conexión simple
   dentro del lenguaje C utilizando sockets.
*/
/* Cabeceras de Sockets */
#include <sys/types.h>
#include <sys/socket.h>

/* Cabecera de direcciones por red */
#include <netdb.h>
/**********/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

//** creaciion de hilos
#include <pthread.h>


/*** kpress.c  (c) 2004 ackbar */
// #include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <sys/ioctl.h>

#define STDINFD  0
#undef getc
// lee entrada de caracteres por teclado
char inkey(void) {
  char c;
  struct termio param_ant, params;

  ioctl(STDINFD,TCGETA,&param_ant);

  params = param_ant;
  params.c_lflag &= ~(ICANON|ECHO);
  params.c_cc[4] = 1;

  ioctl(STDINFD,TCSETA,&params);
  fflush(stdin); fflush(stderr); fflush(stdout);
  read(STDINFD,&c,1);

  ioctl(STDINFD,TCSETA,&param_ant);
  return c;
}
/* EOF: kpress.c */

// VARIABLES GLOBALES
#define TAMMSJ 15  // cantidad de msj a mostrar
int init  = 0;     // indice del mensaje mas viejo

char INPUTBUFF[1024] = ""; //Buuf temporal de la entrada por teclado
char MSJALL[TAMMSJ][1024] = {" "," ","","","","","","", "",""}; // ultimos TAMMSJ msjs

char USERNAME[200];  // nombre de usuaro
char USERAVATAR[10]; // avatar del usuario


// muestra el estado actual de las variables globales por pantalla
void mostrarMsjs(){
  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
  int idex = init;
  printf("╭────────────────────────────────┐\n");
  printf("│ ◉ %s %s\n",USERAVATAR,USERNAME);
  printf("╰╥───────────────────────────╥───┘ \n");

  for (size_t i = 0; i < TAMMSJ-1; i++) {
    printf(" ║%s\n",MSJALL[idex]);
    idex = (idex+1)%TAMMSJ;
  }

  printf(" ║%s\n",MSJALL[idex]);
  printf("╭╨───────────────────────────╨───┐\n");
  printf("│⮑ tú: %s|\n",INPUTBUFF);
  printf("╰────────────────────────────────┘\n");
}

// ñade el ultimmo mensaje al arreglo de todos los msj
void addMsj(char* nmensaje){
  sprintf(MSJALL[init]," %s",nmensaje);
  init=(init+1)%TAMMSJ;
}

//a medida que le caracteres por teclado, invoca a mostrarMsjs
void inputMsj(){
  char c;
  int i=0;
  while((char)(c = inkey()) != '\n'){
    INPUTBUFF[i] = (char)c; i++;
    INPUTBUFF[i] = '\0';
    mostrarMsjs();
  }
}


/*
  El archivo describe un sencillo cliente que se conecta al servidor establecido
  en el archivo RemoteServer.c. Se utiliza de la siguiente manera:
  $cliente IP port
 */

void error(char *msg){
  exit((perror(msg), 1));
}


// Muestra los mesajes entrantes por pantalla
void* mostradorDeMsjEntrantes(void* arg){
  char buf[1024];
  while (strcmp(buf,"quit")!= 0) {
    recv(*((int*)arg), buf, sizeof(buf),0);
    addMsj(buf);
    mostrarMsjs();
    // buf[0]='\0';
  }

  addMsj("El servidor a cerrado la coneccion");
  mostrarMsjs();
  exit(0);
}

int SOCK;
void manejadorSenal(int sig){
  send(SOCK,"quit", sizeof("quit"),0);
  exit(0);
}


int main(int argc, char **argv){

  signal(SIGINT, manejadorSenal); //para poder salir sin romper


  int sock;
  char buf[1024];
  struct addrinfo *resultado;

  /*Chequeamos mínimamente que los argumentos fueron pasados*/
  if(argc != 3){
    fprintf(stderr,"El uso es \'%s IP port\'", argv[0]);
    exit(1);
  }

  /* Inicializamos el socket */
  if( (sock = socket(AF_INET , SOCK_STREAM, 0)) < 0 )
    error("No se pudo iniciar el socket");

  /* Buscamos la dirección del hostname:port */
  if (getaddrinfo(argv[1], argv[2], NULL, &resultado)){
    fprintf(stderr,"No se encontro el host: %s \n",argv[1]);
    exit(2);
  }

  // nos conectamos con el servidor
  if(connect(sock, (struct sockaddr *) resultado->ai_addr, resultado->ai_addrlen) != 0)
    /* if(connect(sock, (struct sockaddr *) &servidor, sizeof(servidor)) != 0) */
    error("No se pudo conectar :(. ");

  SOCK = sock; // para el signal

  // pedimos datos al usuario
  // nombre para identificarlo
  addMsj("🤖 ingrese su nombre ");addMsj("   de usuario");
  mostrarMsjs();
  inputMsj();strcpy(USERNAME,INPUTBUFF);INPUTBUFF[0]='\0';
  mostrarMsjs();

  //  USERAVATAR para que la lectura de los msj sea mas facil
  addMsj("");addMsj("🤖 seleccione un numero");addMsj("   1:🐊 2:🐋 3:🐌 4:🐑 5:🐘");
  mostrarMsjs();
  char c;
  switch ((char)(c = inkey())) {
    case '1':strcpy(USERAVATAR,"🐊");
    break;
    case '2':strcpy(USERAVATAR,"🐋");
    break;
    case '3':strcpy(USERAVATAR,"🐌");
    break;
    case '4':strcpy(USERAVATAR,"🐑");
    break;
    case '5':strcpy(USERAVATAR,"🐘");
    break;
    default:strcpy(USERAVATAR,"🐊");
  }mostrarMsjs();addMsj("");


  pthread_t nhilos;
  // pthread_create(&nhilos,NULL,mostradorDeMsjEntrantes,NULL);
  pthread_create(&nhilos,NULL,mostradorDeMsjEntrantes,&sock);


  char tempSend[1024];
  while(1){

    inputMsj();

    sprintf(tempSend," %stú: %s",USERAVATAR,INPUTBUFF);
    addMsj(tempSend);

    sprintf(tempSend,"%s%s: %s",USERAVATAR,USERNAME,INPUTBUFF);
    if(strlen(tempSend)<1024)send(sock,tempSend, sizeof(tempSend),0);

    INPUTBUFF[0]='\0';
    mostrarMsjs();
  };

  /* Cerramos :D!*/
  freeaddrinfo(resultado);
  close(sock);

  return 0;
}
