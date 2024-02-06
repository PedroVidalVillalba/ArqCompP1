/**
 * PRÁCTICA 1 ARQUITECTURA DE COMPUTADORES
 * Jerarquía y comportamiento de la memoria caché: Estudio del efecto de la localidad
 * de los accesos en las prestaciones de programas en microprocesadores
 * 
 * @date 06/02/2024
 * @authors Cao López, Carlos 
 * @authors Vidal Villalba, Pedro
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./rutinas_clock.c"

ssize_t S1;     /* Número de líneas caché que caben en la caché L1 de datos */
ssize_t S2;     /* Número de líneas caché que caben en la caché L2 */


int main(int argc, char** argv) {
    double *A;
    int *ind;
    int D, L;

    /* Obtener los tamaños de las cachés */
    S1 = sysconf(_SC_LEVEL1_DCACHE_SIZE) / sysconf(_SC_LEVEL1_DCACHE_LINESIZE); 
    S2 = sysconf(_SC_LEVEL2_CACHE_SIZE) / sysconf(_SC_LEVEL2_CACHE_LINESIZE);

    /* Procesamos los argumentos */

    if (argc != 3){
        fprintf(stderr, "Formato de ejecución: ./acp1 Di Li ");
        exit(EXIT_FAILURE);
    }



    
    

}

