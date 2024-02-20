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

#define N 10    /* Número de veces que se repite la reducción */

#define min(a, b) ( (a <= b) ? a : b )

/* Tipo de datos de los elementos del array; double por defecto */
#ifndef USE_INT
typedef double element_t;
#else  // USE_INT
typedef int element_t;
#endif // USE_INT

/* Tipo de acceso a los elementos del array; a través de un vector de índices por defecto */
#ifndef DIRECT_ACCESS
#define index(i) (ind[i])
#else // DIRECT_ACCESS
#define index(i) (D * i)
#endif // DIRECT_ACCESS


int main(int argc, char** argv) {
    element_t *A;
    element_t S[N];
    int *ind;
    int D, L, R;
    int i, j;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    /* Procesamos los argumentos */
    if (argc != 3){
        fprintf(stderr, "Formato de ejecución: ./acp1 Di Li ");
        exit(EXIT_FAILURE);
    }
    D = atoi(argv[1]);
    L = atoi(argv[2]);

    /* Calcular el valor de R necesario para que las lecturas de A[] correspondan a un total de L líneas caché diferentes */
    R = 1 + (L - 1) * min(1, line_size / ( D * sizeof(element_t) ));
    
    /* TODO: justificar esto bien */
    /* Vamos acceder a R elementos del array avanzando de D en D y queremos L líneas caché.
     (R-1)*D*sizeof(element_t) te dice cuantos bytes avanzas en total. Si divides line_size, te da cuantas lineas ocupas. L = (R-1)*D*sizeof(element_t)/linesize */

    for (j = 0; j < N; j++) {
        for (i = 0; i < R; i++) {
            S[j] += A[index(i)];
        }
    }
    
    

}

