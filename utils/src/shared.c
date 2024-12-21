#include "../src/shared.h"

// SOCKET

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = 0;

	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		perror("Error al conectar el socket");
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* puerto, t_log* un_log, char* msj_server)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);

	if( setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		printf("setsockopt(SO_REUSEADDR) failed");

	bind(socket_servidor,servinfo->ai_addr, servinfo->ai_addrlen );

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(un_log, "SERVER: %s", msj_server);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* un_log, char* msj)
{
	int socket_cliente;

	socket_cliente = accept(socket_servidor, NULL, NULL);

	if (socket_cliente == -1) {
		perror("Error al aceptar la conexión");
	}

	log_info(un_log, "SE CONECTO EL CLIENTE: %s!", msj);

	return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
	// printf("SOCKET LIBERADO: %d \n", socket_cliente);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
	{
		return cod_op;
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

// PROTOCOLO

t_paquete* crear_paquete(op_code code_op){
	t_paquete* super_paquete = malloc(sizeof(t_paquete));
	super_paquete->codigo_operacion = code_op;
	crear_buffer(super_paquete);
	return  super_paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


// EXTRACCIÓN


t_buffer* recibir_paquete(int conexion){
	t_buffer* unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream = recibir_buffer(&size, conexion);
	unBuffer->size = size;
	return unBuffer;
}

void* recibir_buffer(int* size, int socket_cliente)
{
    void *buffer = NULL;

    if (size == NULL) {
        fprintf(stderr, "Error: El puntero 'size' es NULL.\n");
        return NULL;
    }

    int bytes_recibidos = recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    if (bytes_recibidos <= 0) {
        if (bytes_recibidos == 0) {
            fprintf(stderr, "Error: El cliente cerró la conexión.\n");
        } else {
            perror("Error al recibir el tamaño del buffer");
        }
        return NULL;
    }

    if (*size <= 0) {
        fprintf(stderr, "Error: Tamaño inválido recibido (%d).\n", *size);
        return NULL;
    }

    buffer = malloc(*size);
    if (buffer == NULL) {
        perror("Error al asignar memoria para el buffer");
        return NULL;
    }

    bytes_recibidos = recv(socket_cliente, buffer, *size, MSG_WAITALL);
    if (bytes_recibidos <= 0) {
        if (bytes_recibidos == 0) {
            fprintf(stderr, "Error: El cliente cerró la conexión.\n");
        } else {
            perror("Error al recibir el contenido del buffer");
        }
        free(buffer);
        return NULL;
    }

    if (bytes_recibidos != *size) {
        fprintf(stderr, "Error: No se recibieron todos los bytes esperados. Recibidos: %d, Esperados: %d.\n", bytes_recibidos, *size);
        free(buffer);
        return NULL;
    }

    return buffer;
}



void* recibir_generico_del_buffer(t_buffer* un_buffer){
	if(un_buffer == NULL)
		return NULL;
		
	if(un_buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(un_buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_generico;
	void* generico;
	memcpy(&size_generico, un_buffer->stream, sizeof(int));
	generico = malloc(size_generico);
	memcpy(generico, un_buffer->stream + sizeof(int), size_generico);

	int nuevo_size = un_buffer->size - sizeof(int) - size_generico;
	if(nuevo_size == 0){
		un_buffer->size = 0;
		free(un_buffer->stream);
		un_buffer->stream = NULL;
		return generico;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_CHICLO]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		exit(EXIT_FAILURE);
	}
	void* nuevo_generico = malloc(nuevo_size);
	memcpy(nuevo_generico, un_buffer->stream + sizeof(int) + size_generico, nuevo_size);
	free(un_buffer->stream);
	un_buffer->stream = nuevo_generico;
	un_buffer->size = nuevo_size;

	return generico;
}

int recibir_int_del_buffer(t_buffer* coso){
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int));

	int nuevo_size = coso->size - sizeof(int);
	if(nuevo_size == 0){

		if(coso->stream != NULL){
			free(coso->stream);
			coso->stream = NULL;
		}

		coso->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_INT]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int), nuevo_size);
	if(coso->stream != NULL){
		free(coso->stream);
		coso->stream = NULL;
	}
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}

char* recibir_string_del_buffer(t_buffer* coso){

    //----------------- Formato Inicial----------------------------
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_string;
	char* string;
	memcpy(&size_string, coso->stream, sizeof(int));
	//string = malloc(sizeof(size_string));
	string = malloc(size_string);
	memcpy(string, coso->stream + sizeof(int), size_string);

	int nuevo_size = coso->size - sizeof(int) - size_string;
	if(nuevo_size == 0){
		if(coso->stream != NULL){
			free(coso->stream);
			coso->stream = NULL;
		}
		coso->stream = NULL;
		coso->size = 0;
		return string;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_STRING]: BUFFER CON TAMAÑO NEGATIVO\n\n");

		if(string != NULL){
			free(string);
			string = NULL;
		}
		//return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int) + size_string, nuevo_size);
	if(coso->stream != NULL){
		free(coso->stream);
		coso->stream = NULL;
	}
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return string;
}

size_t recibir_size_t_del_buffer(t_buffer* un_buffer){
	size_t* un_size_t = recibir_generico_del_buffer(un_buffer);
	size_t valor_retorno = *un_size_t;
	if(un_size_t != NULL){

		free(un_size_t);
		un_size_t = NULL;
	}
	return valor_retorno;
}

uint32_t recibir_uint32_del_buffer(t_buffer* un_buffer){
	uint32_t* un_entero_32 = recibir_generico_del_buffer(un_buffer);
	uint32_t valor_retorno = *un_entero_32;

	if(un_entero_32 != NULL){
		free(un_entero_32);
		un_entero_32 = NULL;
	}

	return valor_retorno;
}

char recibir_char_del_buffer(t_buffer* un_buffer){
	char* un_char = recibir_generico_del_buffer(un_buffer);
	char valor_retorno = *un_char;

	if(un_char != NULL){
		free(un_char);
		un_char = NULL;
	}
	return valor_retorno;
}

void recibir_contexto(t_buffer * unBuffer, t_contexto* contextoRecibido) {
	contextoRecibido->pid = recibir_int_del_buffer(unBuffer);
	contextoRecibido->tid = recibir_int_del_buffer(unBuffer);
	contextoRecibido->maximo_PC = recibir_int_del_buffer(unBuffer);
	contextoRecibido->r_cpu->base = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->limite = recibir_uint32_del_buffer(unBuffer);
	recibir_registros(unBuffer, contextoRecibido);
}

void recibir_registros(t_buffer* unBuffer, t_contexto* contextoRecibido){
	contextoRecibido->r_cpu->PC = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->AX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->BX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->CX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->DX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->EX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->FX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->GX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->HX = recibir_uint32_del_buffer(unBuffer);
}

void recibir_mochila(t_buffer *unBuffer, t_mochila* mochilaRecibida, t_log* logger) {
    mochilaRecibida->instruccionAsociada = recibir_int_del_buffer(unBuffer);
    mochilaRecibida->cantidad_parametros_inicial = recibir_int_del_buffer(unBuffer);

    tipo_dato_parametro TIPO_DATO;
    int i;
    for (i = 0; i < mochilaRecibida->cantidad_parametros_inicial; i++) {
        TIPO_DATO = recibir_int_del_buffer(unBuffer);

        switch (TIPO_DATO) {
            case T_INT: {
                int* valor_int = malloc(sizeof(int));
                if (valor_int == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro INT");
                    continue;
                }
                *valor_int = recibir_int_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_int);
                log_info(logger, "CARGUE VALOR INT: %d", *valor_int);
                break;
            }
            case T_STRING: {
                char* valor_string = recibir_string_del_buffer(unBuffer);
                if (valor_string == NULL) {
                    log_error(logger, "No se pudo recibir el parámetro STRING");
                    continue;
                }
                queue_push(mochilaRecibida->parametros, valor_string);
                log_info(logger, "CARGUE VALOR STRING: %s", valor_string);
                break;
            }
            case T_SIZE_T: {
                size_t* valor_size_t = malloc(sizeof(size_t));
                if (valor_size_t == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro SIZE_T");
                    continue;
                }
                *valor_size_t = recibir_size_t_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_size_t);
                log_info(logger, "CARGUE VALOR SIZE_T: %zu", *valor_size_t);
                break;
            }
            case T_UINT32: {
                uint32_t* valor_uint32 = malloc(sizeof(uint32_t));
                if (valor_uint32 == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro UINT32");
                    continue;
                }
                *valor_uint32 = recibir_uint32_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_uint32);
                log_info(logger, "CARGUE VALOR UINT32: %u", *valor_uint32);
                break;
            }
            default:
                log_error(logger, "TIPO DATO NO VALIDO");
                break;
        }
    }
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

// CARGA

void cargar_generico_al_paquete(t_paquete* paquete, void* choclo, int size){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + size);
		memcpy(paquete->buffer->stream, &size, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), choclo, size);
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
												paquete->buffer->size + sizeof(int) + size);

		memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), choclo, size);
	}

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += size;
}

void cargar_int_al_paquete(t_paquete* paquete, int numero){

	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int));
		memcpy(paquete->buffer->stream, &numero, sizeof(int));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
											paquete->buffer->size + sizeof(int));
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
	}

	paquete->buffer->size += sizeof(int);
}

void cargar_char_al_paquete(t_paquete* paquete, char caracter){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(char));
		memcpy(paquete->buffer->stream, &caracter, sizeof(char));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
		paquete->buffer->size + sizeof(char));

		memcpy(paquete->buffer->stream + paquete->buffer->size, &caracter, sizeof(char));
	}

	paquete->buffer->size += sizeof(char);
}

void cargar_string_al_paquete(t_paquete* paquete, char* string){
	int size_string = strlen(string)+1;

	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char)*size_string);
		memcpy(paquete->buffer->stream, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char)*size_string);

	}else {
		paquete->buffer->stream = realloc(paquete->buffer->stream,
		paquete->buffer->size + sizeof(int) + sizeof(char)*size_string);

		memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*size_string);

	}
	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char)*size_string;
}

void cargar_uint8_al_paquete(t_paquete* un_paquete, uint8_t uint8_value){
	cargar_generico_al_paquete(un_paquete, &uint8_value, sizeof(uint8_t));
}

void cargar_uint32_al_paquete(t_paquete* un_paquete, uint32_t uint32_value){
	cargar_generico_al_paquete(un_paquete, &uint32_value, sizeof(uint32_t));
}

void cargar_size_t_al_paquete(t_paquete* un_paquete, size_t size_t_value){
	cargar_generico_al_paquete(un_paquete, &size_t_value, sizeof(size_t));
}

void cargar_registros_al_paquete(t_paquete * un_paquete, t_registrosCPU* registroRecibido){

	cargar_uint32_al_paquete(un_paquete, registroRecibido->PC);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->AX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->BX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->CX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->DX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->EX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->FX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->GX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->HX);
}

void cargar_contexto_al_paquete(t_paquete * un_paquete, t_contexto* un_contexto){

	cargar_int_al_paquete(un_paquete, un_contexto->pid);
    cargar_int_al_paquete(un_paquete, un_contexto->tid);
	cargar_int_al_paquete(un_paquete, un_contexto->maximo_PC);
	cargar_uint32_al_paquete(un_paquete, un_contexto->r_cpu->base);
	cargar_uint32_al_paquete(un_paquete, un_contexto->r_cpu->limite);
	cargar_registros_al_paquete(un_paquete, un_contexto->r_cpu);
}

void cargar_registros_al_contexto(t_contexto * un_contexto, t_registrosCPU* registroRecibido){

	un_contexto->r_cpu->PC = registroRecibido->PC;
	un_contexto->r_cpu->AX = registroRecibido->AX;
	un_contexto->r_cpu->BX = registroRecibido->BX;
	un_contexto->r_cpu->CX = registroRecibido->CX;
	un_contexto->r_cpu->DX = registroRecibido->DX;
	un_contexto->r_cpu->EX = registroRecibido->EX;
	un_contexto->r_cpu->FX = registroRecibido->FX;
	un_contexto->r_cpu->GX = registroRecibido->GX;
	un_contexto->r_cpu->HX = registroRecibido->HX;
}

// SERVICIOS

void safe_free(void* elemento) {
    if (elemento != NULL) {
        free(elemento);
		elemento = NULL;
    }
}

void ejecutar_en_un_hilo_nuevo_detach(void (*f)(void*) ,void* struct_arg){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)f, struct_arg);
	pthread_detach(thread);
}

// HAY QUE AGREGAR AL SHARED
void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)f, struct_arg);
	pthread_join(thread, NULL);
}

t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo, t_log* logger) {
    FILE* archivo = fopen(path_archivo, "rt");
    t_list* instrucciones = list_create();
    char* instruccion_formateada = NULL;
    int i = 0;
    if (archivo == NULL) {
        perror("No se encontró el archivo");
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(int));
    while (fgets(linea_instruccion, 256, archivo)) {
    	//Comprobar si el ultimo caracter del string capturado tiene un salto delinea
    	//Si lo tiene hay que sacarlo
    	//[0][1][2][3][4]["\n"]["\0"] -> Size:6
    	int size_linea_actual = strlen(linea_instruccion);
    	if(size_linea_actual > 2){
    		if(linea_instruccion[size_linea_actual - 1] == '\n'){
				char* linea_limpia = string_new();
				string_n_append(&linea_limpia, linea_instruccion, size_linea_actual - 1);
				free(linea_instruccion);
				linea_instruccion = malloc(256 * sizeof(int));
//				linea_instruccion = linea_limpia;
				strcpy(linea_instruccion,linea_limpia);
				free(linea_limpia);
    		}
    	}
    	//-----------------------------------------------
        char** l_instrucciones = string_split(linea_instruccion, " ");
		
        // log_info(logger, "Intruccion: [%s]", linea_instruccion);
        while (l_instrucciones[i]) {
        	i++;
        }
        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->pseudo_c = strdup(l_instrucciones[0]);
        pseudo_cod->fst_param = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->snd_param = (i > 2) ? strdup(l_instrucciones[2]) : NULL;
		pseudo_cod->thd_param = (i > 3) ? strdup(l_instrucciones[3]) : NULL;

		if(i == 4){
			instruccion_formateada = string_from_format("%s %s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param, pseudo_cod->thd_param);
		}else if (i == 3) {
        	instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param);
        } else {
            instruccion_formateada = strdup(pseudo_cod->pseudo_c);
        }

//        log_info(memoria_logger, "Se carga la instrucción [%d] %s", (int)strlen(instruccion_formateada),instruccion_formateada);
        list_add(instrucciones, instruccion_formateada);
        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }
        free(l_instrucciones);
        free(pseudo_cod->pseudo_c);
		if(pseudo_cod->fst_param) free(pseudo_cod->fst_param);
		if(pseudo_cod->snd_param) free(pseudo_cod->snd_param);
		if(pseudo_cod->thd_param) free(pseudo_cod->thd_param);
		free(pseudo_cod);
        i = 0;
    }
    fclose(archivo);
    free(linea_instruccion);
    return instrucciones;
}

bool esDumpMemory(const char* str1) {
    if(str1[0] == 'D' && str1[1] == 'U' && str1[2] == 'M' && str1[3] == 'P' && str1[4] == '_' && str1[5] == 'M' && str1[6] == 'E' && str1[7] == 'M' && str1[8] == 'O' && str1[9] == 'R' && str1[10] == 'Y')
        return true;
    else
        return false;
}

bool esProcessExit(const char* str1) {
    if (str1[0] == 'P' && str1[1] == 'R' && str1[2] == 'O' && str1[3] == 'C' && 
        str1[4] == 'E' && str1[5] == 'S' && str1[6] == 'S' && str1[7] == '_' && 
        str1[8] == 'E') {
        return true;
    } else {
        return false;
    }
}

bool esThreadExit(const char* str1) {
    if (str1[0] == 'T' && str1[1] == 'H' && str1[2] == 'R' && str1[3] == 'E' && 
        str1[4] == 'A' && str1[5] == 'D' && str1[6] == '_' && str1[7] == 'E' && 
        str1[8] == 'X' && str1[9] == 'I' && str1[10] == 'T') {
        return true;
    } else {
        return false;
    }
}

bool esMutexUnlock(const char* str1) {
    if (str1[0] == 'M' && str1[1] == 'U' && str1[2] == 'T' && str1[3] == 'E' && 
        str1[4] == 'X' && str1[5] == '_' && str1[6] == 'U' && str1[7] == 'N' && 
        str1[8] == 'L' && str1[9] == 'O' && str1[10] == 'C' && str1[11] == 'K') {
        return true;
    } else {
        return false;
    }
}

bool esMutexLock(const char* str1) {
    if (str1[0] == 'M' && str1[1] == 'U' && str1[2] == 'T' && str1[3] == 'E' && 
        str1[4] == 'X' && str1[5] == '_' && str1[6] == 'L' && str1[7] == 'O' && 
        str1[8] == 'C' && str1[9] == 'K') {
        return true;
    } else {
        return false;
    }
}

bool esMutexCreate(const char* str1) {
    if (str1[0] == 'M' && str1[1] == 'U' && str1[2] == 'T' && str1[3] == 'E' && 
        str1[4] == 'X' && str1[5] == '_' && str1[6] == 'C' && str1[7] == 'R' && 
        str1[8] == 'E' && str1[9] == 'A' && str1[10] == 'T' && str1[11] == 'E') {
        return true;
    } else {
        return false;
    }
}

bool esThreadCancel(const char* str1) {
    if (str1[0] == 'T' && str1[1] == 'H' && str1[2] == 'R' && str1[3] == 'E' && 
        str1[4] == 'A' && str1[5] == 'D' && str1[6] == '_' && str1[7] == 'C' && 
        str1[8] == 'A' && str1[9] == 'N' && str1[10] == 'C' && str1[11] == 'E' && str1[12] == 'L') {
        return true;
    } else {
        return false;
    }
}

bool esThreadJoin(const char* str1) {
    if (str1[0] == 'T' && str1[1] == 'H' && str1[2] == 'R' && str1[3] == 'E' && 
        str1[4] == 'A' && str1[5] == 'D' && str1[6] == '_' && str1[7] == 'J' && 
        str1[8] == 'O' && str1[9] == 'I' && str1[10] == 'N') {
        return true;
    } else {
        return false;
    }
}

bool esThreadCreate(const char* str1) {
    if (str1[0] == 'T' && str1[1] == 'H' && str1[2] == 'R' && str1[3] == 'E' && 
        str1[4] == 'A' && str1[5] == 'D' && str1[6] == '_' && str1[7] == 'C' && 
        str1[8] == 'R' && str1[9] == 'E' && str1[10] == 'A' && str1[11] == 'T' && str1[12] == 'E') {
        return true;
    } else {
        return false;
    }
}

bool esProcessCreate(const char* str1) {
    if (str1[0] == 'P' && str1[1] == 'R' && str1[2] == 'O' && str1[3] == 'C' && 
        str1[4] == 'E' && str1[5] == 'S' && str1[6] == 'S' && str1[7] == '_' && 
        str1[8] == 'C' && str1[9] == 'R' && str1[10] == 'E' && str1[11] == 'A' && str1[12] == 'T' && str1[13] == 'E') {
        return true;
    } else {
        return false;
    }
}

bool esIO(const char* str1) {
    if (str1[0] == 'I' && str1[1] == 'O') {
        return true;
    } else {
        return false;
    }
}

bool esLOG(const char* str1) {
    if (str1[0] == 'L' && str1[1] == 'O' && str1[2] == 'G') {
        return true;
    } else {
        return false;
    }
}

bool esJNZ(const char* str1) {
    if (str1[0] == 'J' && str1[1] == 'N' && str1[2] == 'Z') {
        return true;
    } else {
        return false;
    }
}

bool esSUB(const char* str1) {
    if (str1[0] == 'S' && str1[1] == 'U' && str1[2] == 'B') {
        return true;
    } else {
        return false;
    }
}

bool esSUM(const char* str1) {
    if (str1[0] == 'S' && str1[1] == 'U' && str1[2] == 'M') {
        return true;
    } else {
        return false;
    }
}

bool esWriteMem(const char* str1) {
    if (str1[0] == 'W' && str1[1] == 'R' && str1[2] == 'I' && str1[3] == 'T' && 
        str1[4] == 'E' && str1[5] == '_' && str1[6] == 'M' && str1[7] == 'E' && 
        str1[8] == 'M') {
        return true;
    } else {
        return false;
    }
}

bool esReadMem(const char* str1) {
    if (str1[0] == 'R' && str1[1] == 'E' && str1[2] == 'A' && str1[3] == 'D' && 
        str1[4] == '_' && str1[5] == 'M' && str1[6] == 'E' && str1[7] == 'M') {
        return true;
    } else {
        return false;
    }
}

bool esSet(const char* str1) {
    if (str1[0] == 'S' && str1[1] == 'E' && str1[2] == 'T') {
        return true;
    } else {
        return false;
    }
}

nombre_instruccion_comando convertirInstruccionAEnum(char* nombre_instruccion){
    
    if((strcmp(nombre_instruccion,"SET") == 0) || esSet(nombre_instruccion))
        return SET;
    else if((strcmp(nombre_instruccion,"READ_MEM") == 0) || esReadMem(nombre_instruccion))
        return READ_MEM;
    else if((strcmp(nombre_instruccion,"WRITE_MEM") == 0) || esWriteMem(nombre_instruccion))
        return WRITE_MEM;
    else if((strcmp(nombre_instruccion,"SUM") == 0) || esSUM(nombre_instruccion))
        return SUM;
    else if((strcmp(nombre_instruccion,"SUB") == 0) || esSUB(nombre_instruccion))
        return SUB;
    else if((strcmp(nombre_instruccion,"JNZ") == 0) || esJNZ(nombre_instruccion))
        return JNZ;
    else if((strcmp(nombre_instruccion,"LOG") == 0) || esLOG(nombre_instruccion))
        return LOG;
    else if(strcmp(nombre_instruccion,"DUMP_MEMORY") == 0 || esDumpMemory(nombre_instruccion))
        return DUMP_MEMORY;
    else if((strcmp(nombre_instruccion,"IO") == 0) || esIO(nombre_instruccion))
        return IO;
    else if((strcmp(nombre_instruccion,"PROCESS_CREATE") == 0) || esProcessCreate(nombre_instruccion))
        return PROCESS_CREATE;
    else if((strcmp(nombre_instruccion,"THREAD_CREATE") == 0) || esThreadCreate(nombre_instruccion))
        return THREAD_CREATE;
    else if((strcmp(nombre_instruccion,"THREAD_JOIN") == 0) || esThreadJoin(nombre_instruccion))
        return THREAD_JOIN;
    else if((strcmp(nombre_instruccion,"THREAD_CANCEL") == 0) || esThreadCancel(nombre_instruccion))
        return THREAD_CANCEL;
    else if((strcmp(nombre_instruccion,"MUTEX_CREATE") == 0) || esMutexCreate(nombre_instruccion))
        return MUTEX_CREATE;
    else if((strcmp(nombre_instruccion,"MUTEX_LOCK") == 0) || esMutexLock(nombre_instruccion))
        return MUTEX_LOCK;
    else if((strcmp(nombre_instruccion,"MUTEX_UNLOCK") == 0) || esMutexUnlock(nombre_instruccion))
        return MUTEX_UNLOCK;
    else if((strcmp(nombre_instruccion,"THREAD_EXIT") == 0) || esThreadExit(nombre_instruccion))
        return THREAD_EXIT;
    else
        return PROCESS_EXIT;
}

char* convertirEnumAInstruccionString(nombre_instruccion_comando nombre_instruccion_com){

	switch(nombre_instruccion_com){
		case SET:
			return "SET";
		case READ_MEM:
			return "READ_MEM";
		case WRITE_MEM:
			return "WRITE_MEM";
		case SUM:
			return "SUM";
		case SUB:
			return "SUB";
		case JNZ:
			return "JNZ";
		case LOG:
			return "LOG";
		case DUMP_MEMORY:
			return "DUMP_MEMORY";
		case IO:
			return "IO";
		case PROCESS_CREATE:
			return "PROCESS_CREATE";
		case THREAD_CREATE:
			return "THREAD_CREATE";
		case THREAD_JOIN:
			return "THREAD_JOIN";
		case THREAD_CANCEL:
			return "THREAD_CANCEL";
		case MUTEX_CREATE:
			return "MUTEX_CREATE";
		case MUTEX_LOCK:
			return "MUTEX_LOCK";
		case MUTEX_UNLOCK:
			return "MUTEX_UNLOCK";
		case THREAD_EXIT:
			return "THREAD_EXIT";
		case PROCESS_EXIT:
			return "PROCESS_EXIT";
		default:
			return "INSTRUCCION NO RECONOCIDA";
		
	}
}

t_registros convertirRegistroAEnum(char* registro){
    
    if(registro[0] == 'P' && registro[1] == 'C')
        return PC;
    else if(registro[0] == 'A' && registro[1] == 'X')
        return AX;
    else if(registro[0] == 'B' && registro[1] == 'X')
        return BX;
    else if(registro[0] == 'C' && registro[1] == 'X')
        return CX;
    else if(registro[0] == 'D' && registro[1] == 'X')
        return DX;
    else if(registro[0] == 'E' && registro[1] == 'X')
        return EX;
    else if(registro[0] == 'F' && registro[1] == 'X')
        return FX;
    else if(registro[0] == 'G' && registro[1] == 'X')
        return GX;
    else if(registro[0] == 'H' && registro[1] == 'X')
        return HX;
    else{
		printf("REGISTRO NO ENCONTRADO: %s \n, TAM REG: %ld, TAM AX: %ld \n", registro, strlen(registro), strlen("AX"));
		return -1;
	}
}

const char* convertirCodOpAString(op_code codigo) {
    switch (codigo) {
        // CPU-MEMORIA
        case PETICION_CONTEXTO_CPU_MEMORIA: return "PETICION_CONTEXTO_CPU_MEMORIA";
        case PETICION_DE_INSTRUCCION_CPU_MEMORIA: return "PETICION_DE_INSTRUCCION_CPU_MEMORIA";
        case LECTURA_CPU_MEMORIA: return "LECTURA_CPU_MEMORIA";
        case ESCRITURA_CPU_MEMORIA: return "ESCRITURA_CPU_MEMORIA";
        case ACTUALIZACION_CONTEXTO_CPU_MEMORIA: return "ACTUALIZACION_CONTEXTO_CPU_MEMORIA";
        case SOLICITAR_BASE_CPU_MEMORIA: return "SOLICITAR_BASE_CPU_MEMORIA";

        // CPU-KERNEL
        case ATENDER_INSTRUCCION_CPU: return "ATENDER_INSTRUCCION_CPU";
        case ATENDER_THREAD_EXIT: return "ATENDER_THREAD_EXIT";
        case ATENDER_PROCESS_EXIT: return "ATENDER_PROCESS_EXIT";
        case ATENDER_SEGMENTATION_FAULT: return "ATENDER_SEGMENTATION_FAULT";
        case FORZAR_DESALOJO_CPU_KERNEL: return "FORZAR_DESALOJO_CPU_KERNEL";
        case ATENDER_DESALOJO_HILO_CPU: return "ATENDER_DESALOJO_HILO_CPU";
        case EJECUTAR_HILO_KERNEL_CPU: return "EJECUTAR_HILO_KERNEL_CPU";
        case ATENDER_RTA_KERNEL: return "ATENDER_RTA_KERNEL";
        case ATENDER_RTA_MUTEX_CREATE: return "ATENDER_RTA_MUTEX_CREATE";
        case ATENDER_RTA_MUTEX_LOCK: return "ATENDER_RTA_MUTEX_LOCK";
        case ATENDER_RTA_MUTEX_UNLOCK: return "ATENDER_RTA_MUTEX_UNLOCK";
        case ATENDER_RTA_THREAD_CREATE: return "ATENDER_RTA_THREAD_CREATE";
        case ATENDER_RTA_THREAD_CANCEL: return "ATENDER_RTA_THREAD_CANCEL";
        case ATENDER_RTA_THREAD_JOIN: return "ATENDER_RTA_THREAD_JOIN";
        case ATENDER_RTA_PROCESS_CREATE: return "ATENDER_RTA_PROCESS_CREATE";

        // KERNEL-MEMORIA
        case CREACION_PROCESO_KERNEL_MEMORIA: return "CREACION_PROCESO_KERNEL_MEMORIA";
        case CREACION_HILO_KERNEL_MEMORIA: return "CREACION_HILO_KERNEL_MEMORIA";
        case FINALIZACION_HILO_KERNEL_MEMORIA: return "FINALIZACION_HILO_KERNEL_MEMORIA";
        case FINALIZACION_PROCESO_KERNEL_MEMORIA: return "FINALIZACION_PROCESO_KERNEL_MEMORIA";
        case DUMP_MEMORY_KERNEL_MEMORIA: return "DUMP_MEMORY_KERNEL_MEMORIA";

        // MEMORIA-FILESYSTEM
        case MEMORY_DUMP_MEMORIA_FILESYSTEM: return "MEMORY_DUMP_MEMORIA_FILESYSTEM";
        case FINALIZACION_MEMORIA_FILESYSTEM: return "FINALIZACION_MEMORIA_FILESYSTEM";

        // HANDSHAKES
        case HANDSHAKE_MEMORIA_FILESYSTEM: return "HANDSHAKE_MEMORIA_FILESYSTEM";
        case HANDSHAKE_CPU_MEMORIA: return "HANDSHAKE_CPU_MEMORIA";
        case HANDSHAKE_KERNEL_CPU_DISPATCH: return "HANDSHAKE_KERNEL_CPU_DISPATCH";
        case HANDSHAKE_KERNEL_CPU_INTERRUPT: return "HANDSHAKE_KERNEL_CPU_INTERRUPT";
        case HANDSHAKE_KERNEL_MEMORIA: return "HANDSHAKE_KERNEL_MEMORIA";

        default: return "Código de operación desconocido";
    }
}

// DESTRUIR 

void destruir_buffer(t_buffer* buffer) {
    if (buffer == NULL) {
        return;
    }

	safe_free(buffer->stream);
	safe_free(buffer);
}