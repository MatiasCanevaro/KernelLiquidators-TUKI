#include <sockets.h>

// -----------------------------------------CLIENTE/SERVIDOR-----------------------------------------

void liberar_conexion(int socket)
{
	close(socket);
}

// ------------------------------------------SOCKET CLIENTE------------------------------------------

int crear_conexion(char* ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// CREO EL SOCKET
	int socket_cliente = socket(server_info->ai_family,
                    server_info->ai_socktype,
                    server_info->ai_protocol);

	// HAGO LA CONEXION CON UN SERVIDOR
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	
	freeaddrinfo(server_info);

	return socket_cliente;
}

// -----------------------------------------SOCKET SERVIDOR-----------------------------------------

int iniciar_servidor(char* puerto)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	// CREO EL SOCKET
	int socket_servidor = socket(server_info->ai_family,
                    server_info->ai_socktype,
                    server_info->ai_protocol);

	// TODO (arreglo para el broken pipe)
    const int enable = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

	// ASOCIAMOS EL SOCKET A UN PUERTO
	bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);
	
	// ESCUCHAMOS CONEXIONES ENTRANTES
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(server_info);
	
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// ACEPTAMOS NUEVO CLIENTE
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	
	return socket_cliente;
}

int handshake_cliente(int socket_servidor){
    uint32_t handshake = 1;
    uint32_t handshake_result;

    send(socket_servidor, &handshake, sizeof(uint32_t), NULL);
    recv(socket_servidor, &handshake_result, sizeof(uint32_t), MSG_WAITALL);

		if(handshake_result==2){
    	return 1;
		} else {
			liberar_conexion(socket_servidor);
			return -1;
		}
}

int handshake_servidor(int socket_cliente){
   	uint32_t handshake;
    uint32_t resultOk = 2;
    uint32_t resultError = 1;

    recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);
    if(handshake == 1){
       send(socket_cliente, &resultOk, sizeof(uint32_t), NULL);
	   return 1;
	} else{
       send(socket_cliente, &resultError, sizeof(uint32_t), NULL);
	   liberar_conexion(socket_cliente);
	   return -1;
	}
}