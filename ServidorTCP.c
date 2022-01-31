/*
Este programa hace de servidor concurrente a traves de TCP/IP derivando conexiones
solicitadas por clientes a distintos puertos.
El servidor sera capaz de procesar los comandos echo, hora, fecha, tiempo y quit.
El programa finaliza enviando la señal SIGUSR1. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> 
#include <sys/wait.h>

#include <sys/types.h>    
#include <sys/socket.h>  
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#include <sys/msg.h>

#include <fcntl.h>


/*----- Defines -------------------------------------------------------------*/
#define  PORT_NUM           1050  // Numero de Port
#define  IP_ADDR "127.0.0.1" // Direccion IP LOCALHOST
#define NCOLA 2
#define SOCKET_PROTOCOL 0

/*-----------------Estructura cola de mensajes---------------*/
typedef struct{
  long Id_Mensaje;
  int Dato_Numerico;
}mensaje;

/*-----------------Global---------------*/
int terminar=0, cerrar=0;
int Id_Cola_Mensajes;
mensaje Un_Mensaje;

void handler(int sig);
void muerte(int sig);



//===== Main program ========================================================
int main(int argc, char *argv[])
{

  unsigned int         server_s;        // Descriptor del socket
  unsigned int         connect_s=-1;    // Connection socket descriptor
  struct sockaddr_in   server_addr;     // Estructura con los datos del servidor
  struct sockaddr_in   client_addr;     // Estructura con los datos del cliente
  struct in_addr       client_ip_addr;  // Client IP address
  int                  addr_len;        // Tamaño de las estructuras
  char                 buf_tx[1500];    // Buffer de 1500 bytes para los datos a transmitir
  char                 buf_rx[1500];    // Buffer de 1500 bytes para los datos a transmitir
  int                  bytesrecibidos, bytesaenviar, bytestx;  // Contadores
  int                  i=0;             // Contador de mensajes
  char                 servicio[10];
  time_t               tiempo, tiempop, tiempoh;
  int                  diftime=0;
  struct tm *tm;
  char fechayhora[100];
  pid_t pid_n;

  key_t Clave1;

// Creacion del archivo
  FILE* fichero; 
  char mens[1000];

// Declaracion de la cola de mensajes
  Clave1 = ftok ("/bin/ls", 33);
  if (Clave1 == (key_t)-1){
    printf("Error al obtener clave para cola mensajes" "\n");
    exit(-1);
  }
  Id_Cola_Mensajes = msgget (Clave1, 0600 | IPC_CREAT);
  if (Id_Cola_Mensajes == -1){
    printf("Error al obtener identificador para cola mensajes" "\n");
    exit (-1);
  }

// Instalo un manejadores de señales
  signal(SIGHUP,handler);
  signal(SIGUSR1,muerte);
 

  server_s = socket(AF_INET, SOCK_STREAM, SOCKET_PROTOCOL);
  if (server_s==-1)
    {
    perror("socket");
    return 1;
    }
  printf("Cree el descriptor del socket %d\n",server_s);
  
  int flags = fcntl(server_s, F_GETFL);
  fcntl(server_s, F_SETFL, flags | O_NONBLOCK);     // Flags para evitar que la funcion accept sea bloqueante

  tiempop=time(NULL);

  server_addr.sin_family      = AF_INET;            // Familia TCP/IP
  server_addr.sin_port        = htons(PORT_NUM);    // Número de Port, htons() lo convierte al orden de la red
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY = cualquier direccion IP, htonl() lo convierte al orden de la red
  
  bind(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));

  printf("asocie el descriptor %u con el port %u acepta conexiones desde %u\n", server_s,PORT_NUM, INADDR_ANY) ;

  listen(server_s, NCOLA);
  addr_len = sizeof(client_addr);

  while(terminar!=1){   // Mientras no llegue la señal SIGHUP

    if(cerrar!=0){      // Cuando llega la señal SIGUSR1
      msgrcv (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje,sizeof(Un_Mensaje.Dato_Numerico), 1, 0);
      fichero = fopen("log.txt", "a");
      tiempop=time(NULL);
      tm=localtime(&tiempop);
      strftime(fechayhora, 100, "%X", tm);
      fprintf(fichero, "Se cerro el hijo con PID=%d a la hora %s\n", Un_Mensaje.Dato_Numerico, fechayhora);
      fclose(fichero);
      cerrar=0;
    }
    
    connect_s = accept(server_s, (struct sockaddr *)&client_addr, &addr_len);
    
    if(connect_s==-1){
      sleep(0.01);
    }else{
      if((pid_n=fork())==0){
        tiempo=time(NULL); 

        buf_tx[0]='\n'; //(un solo byte)
        bytestx=send(connect_s, buf_tx, 1 , 0);

        printf("El IP del cliente es: %s y el port del cliente es %hu \n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        do{
          close (server_s);
          
          bytesrecibidos=recv(connect_s, buf_rx, sizeof(buf_rx), 0);
          if (bytesrecibidos==-1){
            perror ("recv");
            return 3;
          }
          
        // Comienzo a derivar los servicios para cada comando
          
          //*************** SERVICIO DE ECHO **************
          if(strncmp(buf_rx,"echo",4) == 0){
            strcpy(buf_rx,"0");
            buf_tx[0]='\n'; //(un solo byte)
            bytestx=send(connect_s, buf_tx, 1 , 0);

            do{
              bytesrecibidos=recv(connect_s, buf_rx, sizeof(buf_rx), 0);
              if (bytesrecibidos==-1){
                perror ("recv");
                return 3;
              }
              sprintf(buf_tx,"echo: %s",buf_rx);
              bytesaenviar =  strlen(buf_tx);
              bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
              if (bytestx==-1){
                perror ("send");
                return 3;
              }
              buf_tx[0]='\n';     // Un solo byte
              bytestx=send(connect_s, buf_tx, 1 , 0);
            }while(strncmp(buf_rx, "echo",4) != 0 && strncmp(buf_rx, "quit",4) != 0); 
            sprintf(buf_tx,"Finalizo el servicio de echo");
            bytesaenviar =  strlen(buf_tx);
            bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
            if (bytestx==-1){
              perror ("send");
              return 3;
            }
          }
          //*************** SERVICIO DE FECHA **************
          else if(strncmp(buf_rx, "fecha",5) == 0){
            tiempoh=time(NULL);
            tm=localtime(&tiempoh);
            strftime(fechayhora, 100, "%d/%m/%Y", tm);
            sprintf(buf_tx,"Hoy es: %s\n", fechayhora);
            bytesaenviar =  strlen(buf_tx);
            bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
            if (bytestx==-1){
              perror ("send");
              return 3;
            }
            buf_tx[0]='\n';     // Un solo byte
            bytestx=send(connect_s, buf_tx, 1 , 0);
          }
          //*************** SERVICIO DE HORA **************
          else if(strncmp(buf_rx, "hora",4) == 0){
            tiempoh=time(NULL);
            tm=localtime(&tiempoh);
            strftime(fechayhora, 100, "%X", tm);
            sprintf(buf_tx,"La hora es: %s\n", fechayhora);
            bytesaenviar =  strlen(buf_tx);
            bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
            if (bytestx==-1){
              perror ("send");
              return 3;
            }
            buf_tx[0]='\n';     // Un solo byte
            bytestx=send(connect_s, buf_tx, 1 , 0);

          }
          //*************** SERVICIO DE TIEMPO **************
          else if(strncmp(buf_rx, "tiempo",6) == 0){
            diftime=time(NULL)-tiempo;
            sprintf(buf_tx,"El tiempo pasado desde la conexion es de %d segundos" ,diftime);
            bytesaenviar =  strlen(buf_tx);
            bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
            if (bytestx==-1){
              perror ("send");
              return 3;
            }
            buf_tx[0]='\n';     // Un solo byte
            bytestx=send(connect_s, buf_tx, 1 , 0);
          }
          //*************** SERVICIO DESCONOCIDO  **************
          else if(strncmp(buf_rx, "quit",4) != 0){
            sprintf(buf_tx,"Servicio no encontrado\nServicios disponibles:\n1-echo\n2-tiempo\n3-hora\n4-fecha\n");
            bytesaenviar =  strlen(buf_tx);
            bytestx=send(connect_s, buf_tx, bytesaenviar, 0);
            if (bytestx==-1){
              perror ("send");
              return 3;
            }
            buf_tx[0]='\n';   // Un solo byte
            bytestx=send(connect_s, buf_tx, 1 , 0);

          }
          
        } while (strncmp(buf_rx,"quit",4)!=0); 

        Un_Mensaje.Id_Mensaje = 1;
        Un_Mensaje.Dato_Numerico =getpid();
        msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, sizeof(Un_Mensaje.Dato_Numerico), 0);

        kill(getppid(),SIGUSR1);

        printf("Se cerro la conexion con el cliente %hu \n", ntohs(client_addr.sin_port));
        close(connect_s);

        return 0;
      }

      //*************** PROCESO PADRE ************** 
      else{                  
        printf("Pedido de conexion: Padre PID=%d --> hijo PID=%d\n", getpid(),pid_n); 
        fichero = fopen("log.txt", "a");
        tiempop=time(NULL);
        tm=localtime(&tiempop);
        strftime(fechayhora, 100, "%X", tm);
        fprintf(fichero, "Se creo el hijo con PID=%d a la hora %s\n",pid_n, fechayhora );
        fclose(fichero);
        close(connect_s);
        connect_s=-1;
      } 
    }
  //*************** SIGHUP **************
  }
  wait(NULL);
  close(server_s);
  msgctl (Id_Cola_Mensajes, IPC_RMID, (struct msqid_ds *)NULL);
  return 0;
} 


void handler(int sig){
  if (sig==SIGHUP){
    terminar=1;
    printf("El servidor sera cerrado\n");
  }
}

void muerte(int sig){
  cerrar=1;
}
