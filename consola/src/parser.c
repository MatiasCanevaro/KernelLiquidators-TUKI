#include <parser.h>

int calcular_longitud_archivo(FILE* archivo){

    // ME MUEVO A LA POSICION FINAL DEL ARCHIVO
    fseek(archivo,0,SEEK_END);

    // LEO EL TAMAÑO
    int len=ftell(archivo);

    // VUELVO A LA POSICION INICIAL
    fseek(archivo,0,SEEK_SET);

    return len; 
}

// RECIBE ARCHIVO Y DEVUELVE EL STRING CON TODO EL TEXTO
char* leer_archivo_completo(FILE* archivo){

    int len=calcular_longitud_archivo(archivo);

    // RESERVO UN ESPACIO EXTRA PARA EL \0
    char* instrucciones=malloc(len+1);
    fread(instrucciones,len,1,archivo);
    instrucciones[len]='\0';

    return instrucciones;
}

// FUNCION QUE RECIBE 1 LINEA Y 1 LISTA DE INSTRUCCIONES, Y LA AGREGA A LA LISTA
void leer_instruccion(char* linea,t_list* lista_instrucciones){
    if(strcmp(linea,"")==0) return;

    // CREO LA INSTRUCCION Y LA AGREGO A LA LISTA
    t_instruccion* instruccion = crear_instruccion();
    list_add(lista_instrucciones,instruccion);

    // SEPARO LA LINEA RECIBIDA EN SUBSTRINGS, QUITANDO LOS ESPACIOS
    char** cadena_instruccion=string_split(linea," ");

    // ASIGNO EL PRIMER STRING DE LA LINEA COMO EL CODIGO DE INSTRUCCION
    instruccion->codigo=convertir_instruccion_a_entero(cadena_instruccion[0]);

    // POR CADA OTRO STRING EN LA LINEA, AGREGO UN PARAMETRO A LA INSTRUCCION
    int n=1;
    free(cadena_instruccion[0]);
    while(cadena_instruccion[n]!=NULL){
        list_add(instruccion->parametros,cadena_instruccion[n]);
        n++;
    }

    // LIBERO LA LINEA Y LA LISTA DE STRINGS, PERO NO LIBERO CADA STRING, YA QUE LA LISTA DE INSTRUCCIONES ESTA APUNTANDO A ELLOS
    free(cadena_instruccion);
}

t_list* obtener_lista_instrucciones(FILE* archivoFuente){
    
    t_list* lista_instrucciones=list_create();

    // LEO TODO EL ARCHIVO EN UN SOLO CHAR*
    char* archivo_completo= leer_archivo_completo(archivoFuente);

    // LO SEPARO POR LINEAS
    char** lineas_separadas= string_split(archivo_completo, "\n");

    int n=0;

    // COMO LA FUNCION STRING_SPLIT DEVUELVE NULL EN LA ULTIMA POSICION, CHEQUEO QUE CADA POSICION DEL CHAR** SEA DISTINTA DE NULL Y LEO CADA LINEA
    while(lineas_separadas[n]!=NULL){
        leer_instruccion(lineas_separadas[n],lista_instrucciones);
        free(lineas_separadas[n]);
        n++;
    }

    // LIBERO LOS PUNTEROS QUE GUARDABAN TODAS LAS LINEAS SEPARADAS Y EL ARCHIVO COMPLETO
    free(lineas_separadas);
    free(archivo_completo);

    return lista_instrucciones;
}

t_list* obtener_instrucciones_de_archivo(char* archivo_pseudocodigo) {
    
    FILE* pseudocodigo = fopen(archivo_pseudocodigo,"r"); //cfg/pseudo.txt
    if(!pseudocodigo)log_error(logger,"no se pudo abrir el archivo\n");

    return obtener_lista_instrucciones(pseudocodigo);
}