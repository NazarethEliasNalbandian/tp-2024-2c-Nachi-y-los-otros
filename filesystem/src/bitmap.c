#include "../src/bitmap.h"

void imprimir_bitarray(t_bitarray* bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    size_t max_bits = bitarray_get_max_bit(bitarray);
    printf("BITMAP: ");
    for (size_t i = 0; i < max_bits; i++) {
        if (bitarray_test_bit(bitarray, i)) {
            printf("1");
        } else {
            printf("0");
        }
        // Para mejor legibilidad, puedes agregar un espacio cada 8 bits (1 byte)
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
    pthread_mutex_unlock(&mutex_bitmap);
}

int encontrar_primer_bit_libre(t_bitarray* bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    size_t max_bits = bitarray_get_max_bit(bitarray);
    for (size_t i = 0; i < max_bits; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            pthread_mutex_unlock(&mutex_bitmap);
            return i;  // Retorna el índice del primer bit 0 encontrado
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);
    return -1;  // No se encontró ningún bit en 0
}

int cantidad_bloques_libres(t_bitarray *bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    int count = 0;
    size_t max_bits = bitarray_get_max_bit(bitarray);

    for (size_t i = 0; i < max_bits; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            count++;
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);

    return count;
}

void actualizar_bitmap_en_memoria() {
    // Se actualizan los bits en bitmapChar desde la estructura de bitmap
    // Se asume que bitmap y bitmapChar están en la misma región de memoria
    memcpy(bitmapChar, bitmap->bitarray, (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0));
}

void setear_bloque_bits(t_bitarray *bitarray, int indice) {
    pthread_mutex_lock(&mutex_bitmap);

    bitarray_set_bit(bitarray, indice);
    actualizar_bitmap_en_memoria();
    
    pthread_mutex_unlock(&mutex_bitmap);

}

void cleanear_bloque_bits(t_bitarray *bitarray, int indice) {
    pthread_mutex_lock(&mutex_bitmap);

    bitarray_clean_bit(bitarray, indice);
    actualizar_bitmap_en_memoria();
    
    pthread_mutex_unlock(&mutex_bitmap);
}