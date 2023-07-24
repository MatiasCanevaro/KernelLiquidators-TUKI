#include <paquete.h>

void eliminar_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(codigo_operacion op_code)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = op_code;
	crear_buffer(paquete);
	return paquete;
}

void* serializar_paquete(t_paquete* paquete){
    void* paquete_serializado = malloc(sizeof(uint8_t) + sizeof(uint32_t) + paquete->buffer->size);
    int offset = 0;
    memcpy(paquete_serializado + offset, &paquete->codigo_operacion, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(paquete_serializado + offset, &paquete->buffer->size, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete_serializado + offset, paquete->buffer->stream, paquete->buffer->size);

    return paquete_serializado;
}
int enviar_paquete(int socket_fd, t_paquete* paquete) {
    // SERIALIZAMOS EL PAQUETE
    //t_buffer* buffer = paquete->buffer;
    void* paquete_serializado= serializar_paquete(paquete);

    // ENVIAMOS EL PAQUETE
    int bytes_enviados = send(socket_fd, paquete_serializado, sizeof(uint8_t) + sizeof(uint32_t) + paquete->buffer->size, 0);

    // LIBERAMOS LA MEMORIA
    free(paquete_serializado);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return bytes_enviados;
}

t_paquete* recibir_paquete(int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    // RECIBIMOS EL CÓDIGO DE OPERACIÓN
    uint8_t cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(uint8_t), MSG_WAITALL) != sizeof(uint8_t)) {
        free(paquete);
        return NULL;
    }
    paquete->codigo_operacion = cod_op;

    // RECIBIMOS EL TAMAÑO DEL PAYLOAD
    uint32_t size;
    if (recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
        free(paquete);
        return NULL;
    }

    // RESERVAMOS MEMORIA PARA EL PAYLOAD
    void* stream = malloc(size);
    if (stream == NULL) {
        free(paquete);
        return NULL;
    }

    // RECIBIMOS EL PAYLOAD
    if (recv(socket_cliente, stream, size, MSG_WAITALL) != size) {
        free(paquete);
        free(stream);
        return NULL;
    }

    // GUARDAMOS EL PAYLOAD EN UN BUFFER
    t_buffer* buffer = malloc(sizeof(t_buffer));
    if (buffer == NULL) {
        free(paquete);
        free(stream);
        return NULL;
    }
    buffer->size = size;
    buffer->stream = stream;

    paquete->buffer = buffer;

    return paquete;
}