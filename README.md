# Descripción

El Trabajo Práctico trata sobre un emulador de un sistema operativo programado en un lenguaje de relativo bajo nivel como C.

Éste se despliega en un sistema distribuido y sus módulos se conectan por red y se envían mensajes serializados.

Los módulos son Kernel, CPU, Memoria y Filesystem.

# Kernel

Se ocupa de gestionar la creación, finalización, ejecución y bloqueo de los procesos e hilos.

Posee un planificador corto plazo que decide qué hilo ejecutará mandando sus identificadores a CPU.

Este planificicador puede utilizar un algoritmo de planificación de FIFO, Prioridades o Cola Multinivel con Round Robin según los archivos de configuración.

Posee un planificador de largo plazo que crea y elimina procesos e hilos comunicándolo a Memoria para que actualice la memoria del sistema.

# CPU

Recibe hilos que ejecutan instrucciones, algunas instrucciones pueden ser Syscalls y deberá comunicarselo a Kernel para que actúe en consecuencia.

También, puede recibir interrupciones por parte de Kernel y se desalojará el hilo en ejecución.

Solicita las instrucciones a Memoria.

# Memoria

Utiliza un sistema de Particionamiento para almacenar los procesos recibidos por Kernel.

Este sistema puede ser de Particionamiento Fijo o Dinámico según los archivos de configuración.

Envía las instrucciones a CPU y escribe o lee en la Memoria de Usuario en caso de que el hilo ejecute las instrucciones READ_MEM, WRITE_MEM o DUMP_MEMORY.

# Filesystem

Utiliza un sistema de asignación de bloques indexada.

Memoria le envía la información escrita por un proceso entre todos sus hilos.

Ante esto, Filesystem almacena esa información en bloques de datos.

Almacena el índice de los bloques de datos en un bloque extra que es el bloque de índices.

Almacena por cada DUMP_MEMORY un archivo de metadata que incluye el índice del bloque de índice y el tamaño del archivo.

Posee un archivo de bitmap que permite saber que bloques están libres.

# Enunciado

El enunciado se encuentra en el archivo The Last Of C

# tp-scaffold

Esta es una plantilla de proyecto diseñada para generar un TP de Sistemas
Operativos de la UTN FRBA.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint, ejemplo: `checkpoint-1`.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

> [!WARNING]
> Asegúrense de que el código compila y cumple con los requisitos del checkpoint
> antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=filesystem "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

## Guías útiles

- [Cómo interpretar errores de compilación](https://docs.utnso.com.ar/primeros-pasos/primer-proyecto-c#errores-de-compilacion)
- [Cómo utilizar el debugger](https://docs.utnso.com.ar/guias/herramientas/debugger)
- [Cómo configuramos Visual Studio Code](https://docs.utnso.com.ar/guias/herramientas/code)
- **[Guía de despliegue de TP](https://docs.utnso.com.ar/guías/herramientas/deploy)**

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy

## DESPLIEGUE PROYECTO

Abrimos la Virtual Box

Se abre una terminal en Virtual Box

En esa terminal, escribimos ifconfig

Averiguamos la IP de la máquina

Abrimos Putty e insertamos esa IP tocando Aceptar

Ingresamos a la terminal de Putty

Allí, clonamos el repositorio so-deploy

git clone https://github.com/sisoputnfrba/so-deploy.git

Accedemos a la carpeta so-deploy

cd so-deploy

./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=filesystem -c=IP_KERNEL=192.168.1. -c=IP_CPU=192.168.1. -c=IP_MEMORIA=192.168.1. -c=IP_FILESYSTEM=192.168.1. -c=MOUNT_DIR=/home/utnso/so-deploy/tp-2024-2c-C-puede/filesystem/mount_dir "tp-2024-2c-C-puede"

Ponemos la contraseña para sudo: utnso

Usuario: usuarioGitHub
Contraseña: tokenGithub

## PRUEBA PLANI (FUNCIONA)

./filesystem "../configs/PLANI.config"

./memoria "../configs/PLANI.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/PLANI_PROC" 32 "../configs/PLANI.config"

## PRUEBA RACE CONDITIONING (FUNCIONA)

./filesystem "../configs/RACE_COND.config"

./memoria "../configs/RACE_COND.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/RECURSOS_MUTEX_PROC" 32 "../configs/RACE_COND.config"

## PRUEBA PARTICION FIJA (FUNCIONA)

./filesystem "../configs/PARTICION_FIJA.config"

./memoria "../configs/PARTICION_FIJA.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/MEM_FIJA_BASE" 12 "../configs/PARTICION_FIJA.config"

## PRUEBA PARTICION DINAMICA (FUNCIONA)

./filesystem "../configs/PARTICION_DINAMICA.config"

./memoria "../configs/PARTICION_DINAMICA.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/MEM_DINAMICA_BASE" 128 "../configs/PARTICION_DINAMICA.config"

## PRUEBA FIBONACCI (FUNCIONA)

./filesystem "../configs/FIBONACCI.config"

./memoria "../configs/FIBONACCI.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/PRUEBA_FS" 8 "../configs/FIBONACCI.config"

## PRUEBA STRESS (FUNCIONA)

./filesystem "../configs/STRESS.config"

./memoria "../configs/STRESS.config"

./cpu "../configs/cpu.config"

./kernel "../../the-last-of-c-pruebas/THE_EMPTINESS_MACHINE" 16 "../configs/STRESS.config"

## PRUEBA PLANI (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/PLANI.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/PLANI.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/PLANI_PROC" 32 "../configs/PLANI.config"

## PRUEBA RACE CONDITIONING (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/RACE_COND.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/RACE_COND.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/RECURSOS_MUTEX_PROC" 32 "../configs/RACE_COND.config"

## PRUEBA PARTICION FIJA (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/PARTICION_FIJA.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/PARTICION_FIJA.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/MEM_FIJA_BASE" 12 "../configs/PARTICION_FIJA.config"

## PRUEBA PARTICION DINAMICA (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/PARTICION_DINAMICA.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/PARTICION_DINAMICA.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/MEM_DINAMICA_BASE" 128 "../configs/PARTICION_DINAMICA.config"

## PRUEBA FIBONACCI (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/FIBONACCI.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/FIBONACCI.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/PRUEBA_FS" 8 "../configs/FIBONACCI.config"

## PRUEBA STRESS (VALGRIND) (FUNCIONA)

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./filesystem "../configs/STRESS.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memoria "../configs/STRESS.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cpu "../configs/cpu.config"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./kernel "../../the-last-of-c-pruebas/THE_EMPTINESS_MACHINE" 16 "../configs/STRESS.config"
