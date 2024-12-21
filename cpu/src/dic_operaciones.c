#include "../src/dic_operaciones.h"

void diccionario_operaciones(){
	op_autorizada = string_array_new();
	string_array_push(&op_autorizada, "SET");
	string_array_push(&op_autorizada, "READ_MEM");
	string_array_push(&op_autorizada, "WRITE_MEM");
	string_array_push(&op_autorizada, "SUM");
	string_array_push(&op_autorizada, "SUB");
	string_array_push(&op_autorizada, "JNZ");
	string_array_push(&op_autorizada, "LOG");
	string_array_push(&op_autorizada, "DUMP_MEMORY");
	string_array_push(&op_autorizada, "IO");
	string_array_push(&op_autorizada, "PROCESS_CREATE");
	string_array_push(&op_autorizada, "THREAD_CREATE");
	string_array_push(&op_autorizada, "THREAD_JOIN");
	string_array_push(&op_autorizada, "THREAD_CANCEL");
	string_array_push(&op_autorizada, "MUTEX_CREATE");
	string_array_push(&op_autorizada, "MUTEX_LOCK");
	string_array_push(&op_autorizada, "MUTEX_UNLOCK");
	string_array_push(&op_autorizada, "THREAD_EXIT");
	string_array_push(&op_autorizada, "PROCESS_EXIT");
}