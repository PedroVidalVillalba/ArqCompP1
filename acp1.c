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
#include <pmmintrin.h>

#define N 10 /* Número de veces que se repite la reducción */

#define max(a, b) ((a >= b) ? a : b)

/* Tipo de datos de los elementos del array; double por defecto */
#ifndef USE_INT
typedef double element_t;
/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))
#else // USE_INT
typedef int element_t;
#define get_rand() ((int)mrand48())
#endif // USE_INT

/* Tipo de acceso a los elementos del array; a través de un vector de índices por defecto */
#ifndef DIRECT_ACCESS
#define index(i) (ind[i])
#else // DIRECT_ACCESS
#define index(i) (D * i)
#endif // DIRECT_ACCESS

/*** Código rutinas clock ***/

void start_counter();
double get_counter();
double mhz();

/* Initialize the cycle counter */

static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

/* Set *hi and *lo to the high and low order bits of the cycle counter.
Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo) {
    asm("rdtsc; movl %%edx,%0; movl %%eax,%1" /* Read cycle counter */
        : "=r"(*hi), "=r"(*lo)                /* and move results to */
        : /* No input */                      /* the two outputs */
        : "%edx", "%eax");
}

/* Record the current value of the cycle counter. */
void start_counter() {
    access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter() {
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;

    /* Get cycle counter */
    access_counter(&ncyc_hi, &ncyc_lo);

    /* Do double precision subtraction */
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double)hi * (1 << 30) * 4 + lo;
    if (result < 0) {
        fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
    }
    return result;
}

double mhz(int verbose, int sleeptime) {
    double rate;

    start_counter();
    sleep(sleeptime);
    rate = get_counter() / (1e6 * sleeptime);
    if (verbose)
        printf(" Processor clock rate = %.1f MHz\n", rate);
    return rate;
}

int main(int argc, char **argv) {
    element_t *A;
    element_t S[N];
    int *ind;
    int D, L, R;
    int M; /* Tamaño del vector A */
    int i, j;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    FILE *_null;
    double ck;

    /* Procesamos los argumentos */
    if (argc != 3) {
        fprintf(stderr, "Formato de ejecución: ./acp1 Di Li ");
        exit(EXIT_FAILURE);
    }
    D = atoi(argv[1]);
    L = atoi(argv[2]);

    /* Calcular el valor de R necesario para que las lecturas de A[] correspondan a un total de L líneas caché diferentes */
    R = 1 + (L - 1) * max(1, line_size / (D * sizeof(element_t)));

#ifndef DIRECT_ACCESS
    /* Inicializar vector de índices */
    ind = (int *)alloca(R * sizeof(int));
    for (i = 0; i < R; i++) {
        ind[i] = D * i;
    }
#endif // DIRECT_ACCESS

    /* Reservamos memoria para A */
    M = R * D;
    A = _mm_malloc(M * sizeof(element_t), line_size);

    /* Inicializar A con valores aleatorios en el intervalo [1, 2) con signo aleatorio */
    for (i = 0; i < M; i++) {
        A[i] = get_rand();
    }

    /* TODO: justificar esto bien */
    /* Vamos acceder a R elementos del array avanzando de D en D y queremos L líneas caché.
     (R-1)*D*sizeof(element_t) te dice cuantos bytes avanzas en total. Si divides line_size, te da cuantas lineas ocupas. L = (R-1)*D*sizeof(element_t)/linesize */

    /**** Comenzamos la medida de esta ejecución ****/
    start_counter();

    for (j = 0; j < N; j++) {
        for (i = 0; i < R; i++) {
            S[j] += A[index(i)]; /* S simplemente es para que el compilador no haga optimizaciones */
        }
    }

    ck = get_counter();

    printf("Parameters:D=%i,L=%i,R=%i\n", D, L, R);
    printf(" Clocks=%1.10lf \n", ck);
    printf(" ClocksPerAcccessToA=%1.10lf\n\n", ck / (N * R));

    /* Esta rutina imprime a frecuencia de reloxo estimada coas rutinas start_counter/get_counter */
    /* mhz(1, 1); */

    /**** Fin de la medida ****/

    _mm_free(A); /* Liberamos la memoria */

    /* Imprimir resultados de S para que el compilador no lo optimice.
     * Como no los queremos para nada, los imprimimos a /dev/null para que el resultado no enturbie la salida del programa
     * El tiempo de esta parte no se mide */
    _null = fopen("/dev/null", "w");

    for (j = 0; j < N; j++) {
#ifndef USE_INT
        fprintf(_null, "S[%d] = %lf\n", j, S[j]);
#else  // USE_INT
        fprintf(_null, "S[%d] = %d\n", j, S[j]);
#endif // USE_INT
    }
    fclose(_null);
}
