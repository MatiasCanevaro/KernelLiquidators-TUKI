#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <init_consola.h>
#include <finish_consola.h>
#include <parser.h>
// config
extern t_config* config;
// logger
extern t_log* logger;

// recibidos de config
extern char* ip_kernel;
extern char* puerto_kernel;

// sockets
extern int socket_kernel;

#endif