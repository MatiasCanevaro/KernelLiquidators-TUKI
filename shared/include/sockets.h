#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>


    /**
    * @NAME: crear_conexion
	* @DESC: Utilizado por el cliente para conectar con el servidor 
	* @PARAMS:
    *        ip - ip del servidor al que se conecta
    *        puerto - puerto a traves del que se realiza la conexion
    * @RETURN:
    *       Retorna la conexion
    */
    int         crear_conexion(char* ip, char* puerto);


    /**
    * @NAME: liberar_conexion
	* @DESC: Cierra un socket
    */
    void        liberar_conexion(int socket_cliente);


    /**
    * @NAME: iniciar_servidor
	* @DESC: Utilizado por el servidor para iniciarse en un puerto y escuchar conexiones
	* @PARAMS:
    *        ip - ip del servidor 
    *        puerto - puerto a traves del que se realiza la conexion
    * @RETURN:
    *       Retorna el socket del servidor
    */
    int         iniciar_servidor(char* puerto);


int esperar_cliente(int socket_servidor);


    /**
    * @NAME: handshake_cliente
	* @DESC: Envia un handshake al servidor y espera el valor de respuesta. En caso de ser
    *    incorrecto cierra la conexion con el servidor.
	* @PARAMS:
    *        socket_servidor - socket del servidor con el que se efectua el handshake
    * @RETURN:
    *       Retorna -1 caso de error y 1 si es correcto.
    */
    int         handshake_cliente(int socket_servidor);


    /**
    * @NAME: handshake_servidor
	* @DESC: Recibe un handshake del cliente y le devuelve el valor segun corresponda. En caso de ser
    *    incorrecto cierra la conexion con ese cliente.
    *   Retorna -1 caso de error y 1 si es correcto.
	* @PARAMS:
            socket_cliente - socket del cliente del que recibiremos el handshake
    * @RETURN:
    *       Retorna -1 caso de error y 1 si es correcto.    
    */
    int         handshake_servidor(int socket_cliente);

#endif