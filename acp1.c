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

/* Macro para calcular el máximo entre dos números */
#define max(a, b) ((a >= b) ? a : b)

/* Tipo de datos de los elementos del array; double por defecto. Se cambia con la opción de compilación -D USE_INT.
 * También aprovechamos aquí para definir una macro con la que rellenar el array A con valores aleatorios, de forma unificada
 * y que devuelva diferentes valores en función del tipo de dato a utilizar */
#ifndef USE_INT
typedef double element_t;
/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))
#else // USE_INT
typedef int element_t;
/* mrand48 devuelde un long con signo uniformemento distribuido en [-2^31, 2^31), que posteriormente casteamos a int */
#define get_rand() ((int)mrand48())
#endif // USE_INT

/* Tipo de acceso a los elementos del array; a través de un vector de índices por defecto. Se cambia con la opción de compilación -D DIRECT_ACCESS */
#ifndef DIRECT_ACCESS
#define index(i) (ind[i])   /* Accedemos leyendo el vector ind */
#else // DIRECT_ACCESS
#define index(i) (D * i)    /* El acceso al array A se hace calculando D * i directamente */ 
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

/*** Fin código rutinas clock ***/


int main(int argc, char **argv) {
    element_t* A;       /* Array con números aleatorios sobre el que realizar los accesos */
    element_t S[N];     /* Array con para almacenar los resultados de la reducción */
    element_t mean;     /* Media de los elementos de S (solo para que el compilador no optimice el código)*/
    int* ind;           /* Vector de índices para el acceso indirecto a A */
    int D;              /* Distancia entre los elementos sucesivos de A a los que se acceden; primer argumento */
    int L;              /* Número de líneas caché diferentes a las que se accede al leer los valores de A; segundo argumento */
    int R;              /* Número de lecturas que se hacen al vector A, de D en D elementos, para leer L líneas caché */
    int M;              /* Tamaño del vector A */
    int i, j;           /* Índices para iterar en los bucles */
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);   /* Tamaño de línea caché (en bytes) */
    FILE *dev_null;     /* Puntero al archivo /dev/null en el que imprimir los valores de S al terminar sin que ensucien la salida estándar,
                           para que el compilador no optimizaciones no deseadas */
    double ck;          /* Variable para almacenar el número de ciclos de reloj que se usan en los accesos a A */

    /* Procesamos los argumentos */
    if (argc != 3) {
        fprintf(stderr, "Formato de ejecución: ./acp1 Di Li ");
        exit(EXIT_FAILURE);
    }
    D = atoi(argv[1]);  /* Primer argumento del programa */
    L = atoi(argv[2]);  /* Segundo argumento del programa */

    /* Calcular el valor de R necesario para que las lecturas de A correspondan a un total de L líneas caché diferentes */
    R = 1 + (L - 1) * max(1, line_size / (D * sizeof(element_t)));
    /* Vamos acceder a R elementos del array, avanzando de D en D, y queremos acceder a L líneas caché diferentes en total.
     * Podemos distinguir dos casos distintos, según el valor de D:
     *  1. Si el valor de D es menor que el número de elementos del array por línea caché (line_size / sizeof(element_t)),
     *     entonces necesitamos realizar al menos una lectura de las primeras L líneas caché desde el inicio de A.
     *     Así, debemos leer las L-1 primeras líneas caché enteras, y solamente hacer una lectura de la número L. Obtenemos
     *     R - 1 = (L - 1) * (line_size / (D * sizeof(element_t)))
     *  2. Si el valor de D es mayor que el número de elementos del array por línea caché, entonces simplemente tenemos que hacer L
     *     lecturas de A, pues al avanzar D elementos estaremos leyendo de una línea caché diferente en cada lectura.
     * Juntanto ambos casos en una única fórmula, se obtiene
     *      R = 1 + (L - 1) * max(1, line_size / (D * sizeof(element_t)))
     */

#ifndef DIRECT_ACCESS
    /* Inicializar vector de índices, solo si no estamos con acceso directo */
    ind = (int *)alloca(R * sizeof(int));   /* Alojamos el vector de índices en el stack, para aumentar la eficiencia y no tener que liberarlo manualmente al acabar */
    for (i = 0; i < R; i++) {               /* Rellenar el vector de índices con las direcciones a acceder */
        ind[i] = D * i;
    }
#endif // DIRECT_ACCESS

    /* Reservamos memoria para A */
    M = R * D;
    A = _mm_malloc(M * sizeof(element_t), line_size);   /* Alojamos dinámicamente memoria para A, y alineamos el comienzo del array con el inicio de una línea caché */

    /* Inicializar A con valores aleatorios en el intervalo [1, 2) con signo aleatorio en el caso de double,
     * o con ints aleatorios uniformemente distribuidos en todo el rango de valores posibles */
    for (i = 0; i < M; i++) {
        A[i] = get_rand();
    }


    /**** Comenzamos la medida de esta ejecución ****/
    start_counter();

    for (j = 0; j < N; j++) {
        for (i = 0; i < R; i++) {
            S[j] += A[index(i)]; /* S simplemente es para que el compilador no haga optimizaciones */
        }
    }

    ck = get_counter();
    /**** Fin de la medida ****/

    /* Imprimimos los parámetros de la ejecución y el número de ciclos de reloj y número de ciclos de reloj por acceso a A,
     * con un formato consistente para un posterior postprocesado */
    printf("Parameters:D=%i,L=%i,R=%i\n", D, L, R);
    printf(" Clocks=%1.10lf \n", ck);
    printf(" ClocksPerAcccessToA=%1.10lf\n\n", ck / (N * R));

    /* Esta rutina imprime la frecuencia de reloj estimada con las rutinas start_counter/get_counter */
    /* Tras unas pruebas preliminares, vemos que la frecuencia de reloj en el Finisterrae III es 2200.1 MHz de forma absolutamente consistente.
     * Como no necesitamos este valor para el análisis de los resultados, y la ejecución de esta función tarda un segundo entero,
     * se decidió no ejecutarla para aumentar la velocidad a la que se generan los resultados y poder obtener antes el turno en el Cesga. */
    /* mhz(1, 1); */

    _mm_free(A); /* Liberamos la memoria del array A */

    /* Imprimir resultados de S para que el compilador no lo optimice.
     * Como no los queremos para nada, los imprimimos a /dev/null para que el resultado no enturbie la salida del programa.
     * El tiempo de esta parte no se mide */
    dev_null = fopen("/dev/null", "w");

    mean = 0;   /* También calculamos la media del array S, para asegurarnos aún más que no se hagan optimizaciones innecesarias */
    for (j = 0; j < N; j++) {
        mean += S[j];
        /* Imprimimos con formato diferente en función de si usamos double o int */
#ifndef USE_INT
        fprintf(dev_null, "S[%d] = %lf\n", j, S[j]);
#else  // USE_INT
        fprintf(dev_null, "S[%d] = %d\n", j, S[j]);
#endif // USE_INT
    }
    mean /= N;

    fclose(dev_null);   /* Cerramos el fichero /dev/null */

    exit(EXIT_SUCCESS);
}
