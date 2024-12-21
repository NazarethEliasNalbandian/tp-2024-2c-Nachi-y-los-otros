#include "../src/inicializar_filesystem.h"
#include "../src/bitmap.h"

void inicializar(char* archivo_config)
{
    inicializar_configs(archivo_config);
    inicializar_logs();
	iniciar_semaforos();
    inicializar_archivo_bloques();
    inicializar_archivo_bitmap();
    inicializar_pthreads();
    lista_fcb = list_create();
}

void inicializar_logs(){

	fs_log_debug = log_create("filesystem_debug.log","[Filesystem - Debug]",1,LOG_LEVEL_TRACE);

	if(fs_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	fs_log_obligatorio = log_create("filesystem_log_obligatorio.log", "[Filesystem - Log Obligatorio]", 1, log_level_from_string(LOG_LEVEL));
}

void inicializar_configs(char* archivo_config){
	if((fs_config = config_create(archivo_config)) == NULL)
	{
		log_error(fs_log_debug,"Error al crear el archivo de configuracion");
		exit(2);
	}

    PUERTO_ESCUCHA  = config_get_string_value(fs_config, "PUERTO_ESCUCHA");
    MOUNT_DIR = config_get_string_value(fs_config, "MOUNT_DIR");
    BLOCK_SIZE = config_get_int_value(fs_config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(fs_config, "BLOCK_COUNT");
    RETARDO_ACCESO_BLOQUE = config_get_int_value(fs_config, "RETARDO_ACCESO_BLOQUE");
    LOG_LEVEL = config_get_string_value(fs_config, "LOG_LEVEL");
}

void iniciar_semaforos(){
	sem_init(&se_desconecto_memoria,0,0);
}

void inicializar_archivo_bloques(){
    size_t path_length = strlen(MOUNT_DIR) + strlen("/bloques.dat") + 1;
    size_t path_length_files = strlen(MOUNT_DIR) + strlen("/files/") + 1;
    size_t path_length_mount = strlen(MOUNT_DIR) + strlen("/") + 1;
    PATH_ARCHIVO_BLOQUES = malloc(path_length);
    char* path_archivo_mount = malloc(path_length_mount);
    char* path_archivo_files = malloc(path_length_files);
	// O_CREAT: Si el archivo no existe, se creará.
	// O_RDWR: Permite lectura y escritura en el archivo.

    if (PATH_ARCHIVO_BLOQUES == NULL) {
        log_error(fs_log_debug, "Error al asignar memoria para PATH_ARCHIVO_BLOQUES");
        exit(EXIT_FAILURE);
    }

    snprintf(PATH_ARCHIVO_BLOQUES, path_length, "%s/bloques.dat", MOUNT_DIR);
    snprintf(path_archivo_files, path_length_files, "%s/files/", MOUNT_DIR);
    snprintf(path_archivo_mount, path_length_mount, "%s", MOUNT_DIR);
    mkdir(path_archivo_mount, 0755);
    mkdir(path_archivo_files, 0755);

    printf("ARCHIVO DE BLOQUES: %s\n", PATH_ARCHIVO_BLOQUES);

    fd_archivoBloques = open(PATH_ARCHIVO_BLOQUES, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	// CREAR EL fd_archivoBloques
	int tamanio_archivo_bloques = BLOCK_SIZE * BLOCK_COUNT;
	ftruncate(fd_archivoBloques, tamanio_archivo_bloques);


	// MAPEO EL ARCHIVO A UN VOID*. AMBOS ESTÁN RELACIONADOS.
	bloquesEnMemoria = mmap(NULL, BLOCK_SIZE * BLOCK_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoBloques, 0);
	if (bloquesEnMemoria == MAP_FAILED) {
		log_error(fs_log_debug, "Error al mapear los bloques en memoria");
		exit(EXIT_FAILURE);
	}

    log_info(fs_log_debug, "Archivo de bloques inicializado correctamente.\n");

    safe_free(path_archivo_files);
    safe_free(path_archivo_mount);
}

void inicializar_archivo_bitmap(){
	size_t path_length = strlen(MOUNT_DIR) + strlen("/bitmap.dat") + 1;
    PATH_BITMAP = malloc(path_length);

    if (PATH_BITMAP == NULL) {
        log_error(fs_log_debug,"Error al asignar memoria para PATH_BITMAP");
        exit(EXIT_FAILURE);
    }

    snprintf(PATH_BITMAP, path_length, "%s/bitmap.dat", MOUNT_DIR);

    fd_bitmap = open(PATH_BITMAP, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1) {
        log_error(fs_log_debug, "Error al abrir/crear el archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    int tamanio_bitmap = (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0); // En bytes
    if (ftruncate(fd_bitmap, tamanio_bitmap) == -1) {
        log_error(fs_log_debug, "Error al definir el tamaño del archivo de bitmap");
        close(fd_bitmap);
        exit(EXIT_FAILURE);
    }

    bitmapChar = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (bitmapChar == MAP_FAILED) {
        log_error(fs_log_debug, "Error al mapear el archivo de bitmap en memoria");
        close(fd_bitmap);
        exit(EXIT_FAILURE);
    }

    bitmap = bitarray_create_with_mode(bitmapChar, tamanio_bitmap, LSB_FIRST);
    imprimir_bitarray(bitmap);

    log_info(fs_log_debug, "Archivo de bitmap inicializado correctamente.\n");
}

void inicializar_pthreads(){
    pthread_mutex_init(&mutex_bloquesEnMemoria, NULL);
    pthread_mutex_init(&mutex_lista_fcb, NULL);
    pthread_mutex_init(&mutex_bitmap, NULL);
}