/*
Este programa actua como cliente que solicita una conexion a un servidor.
El cliente finaliza su conexion al enviar el comando quit. 
*/
#include <stdio.h>
#include <string.h>

#include <sys/types.h>    
#include <sys/socket.h>  
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>

/****************************************************************************
PROGRAMA CLIENTE TCP
****************************************************************************/

//----- Defines -------------------------------------------------------------

#define  PORT_NUM           1050            // Port number used
#define  IP_ADDR "127.0.0.1"                // Direccion IP LOCALHOST
#define SOCKET_PROTOCOL 0

//===== Main program ========================================================

int main(int argc, char *argv[])
{

  unsigned int         client_s;        // Descriptor del socket
  struct sockaddr_in   server_addr;     // Estructura con los datos del servidor
  struct sockaddr_in   client_addr;     // Estructura con los datos del cliente
  int                  addr_len;        // Tamaño de las estructuras
  char                 buf_tx[1500];    // Buffer de 1500 bytes para los datos a transmitir
  char                 buf_rx[1500];    // Buffer de 1500 bytes para los datos a transmitir
  char                 ipserver[16];
  int                  bytesrecibidos,bytesaenviar, bytestx;               // Contadores
  int                  conectado;       // Variable auxiliar

/*  Leo argumentos desde la linea de comandos*/

  if (argc!=2)
    {
    printf("uso: clienteTCP www.xxx.yyy.zzz\n");
    return -1; 
    }
    strncpy(ipserver,argv[1],16);
  
      
  // Crear el socket
  // AF_INET es  "Address Family: Internet (Familia de Direcciones TCP/IP) 
  // SOCK_DGRAM es para datagramas ( Protocolo UDP) 
  // SOCKET_PROTOCOL fue previamente #defined
  
  client_s = socket(AF_INET, SOCK_STREAM, SOCKET_PROTOCOL);
  if (client_s==-1)
    {
    perror("socket");
    return 2;
    }
  printf("Cree el descriptor del socket %d\n",client_s);
  
  server_addr.sin_family      = AF_INET;              // Familia TCP/IP
  server_addr.sin_port        = htons(PORT_NUM);      // Número de Port, htons() lo convierte al orden de la red
  
  if  (inet_aton(ipserver, &server_addr.sin_addr)==0) // Cargo la estructura con la dirección IP del servidor
    {
    printf ("La dirección  IP_ADDR no es una dirección IP válida. Programa terminado\n"); 
    return 3;
    }
  addr_len = sizeof(server_addr);
   
  conectado=connect(client_s, (struct sockaddr *)&server_addr, sizeof(server_addr));  // Conecto con el servidor
  if (conectado==-1)
    {
    perror("connect");
    return 4;
    }
     printf("El IP del servidor es: %s y el port del servidor  es %hu \n",inet_ntoa(server_addr.sin_addr),
            ntohs(server_addr.sin_port));
  do{  
    bytesrecibidos=recv(client_s, buf_rx, sizeof(buf_rx), 0);    // Quedo a la espera de recibir el mensaje del servidor
    buf_rx[bytesrecibidos]=0;         // Me aseguro que termine en NULL 
    
    printf("%s", buf_rx);

    printf("-Ingrese mensaje\n");
    fgets(buf_tx,1499,stdin);
    bytesaenviar=strlen(buf_tx)+1;    // Me aseguro que la cantidad de bytes a enviar sea la correcta 
    buf_tx[bytesaenviar]=0;
    
    printf("\n-Respuesta del servidor:\n");
  
    bytestx=send(client_s, buf_tx, bytesaenviar,0);  // Envio al servidor 
  } while (strncmp(buf_tx,"quit",4)!=0);

  printf("Se desconecto del servidor\n");
  
  close(client_s);    // Cierro el socket
  return 0;
}
