#!/bin/bash
# Solicitamos un nodo con 64 cores y 256 GB de memoria durante 2 horas
#SBATCH -n 1 -c 64 -t 02:00:00 --mem=256G
# Ponemos nombre a nuestro trabajo para poder identificarlo.
# ATENCIÓN - Debes sustituir el NN por el número de equipo.
#SBATCH --job-name p1acg09

# Sustituir los valores de Di y Li por los calculados para la realización de la práctica.


# Opciones de compilación obligatorias
FLAGS="-msse3 -O0"

# Variantes de compilación para cambiar el comportamiento del programa.
# Comentar y descomentar para cambiar el funcionamiento.
FLAGS+=" -D USE_INT"				# Usar datos de tipo int en lugar de double
FLAGS+=" -D DIRECT_ACCESS"		# Utilizar referencias directas a los elementos de A[] en lugar de a través del vector de índices

gcc acp1.c -o acp1 $FLAGS

S1=$[$(getconf LEVEL1_DCACHE_SIZE)/$(getconf LEVEL1_DCACHE_LINESIZE)]
S2=$[$(getconf LEVEL2_CACHE_SIZE)/$(getconf LEVEL2_CACHE_LINESIZE)]

#Repetimos el experimento 10 veces
for i in {1..10}
do
	for D in {$[1<<0],$[1<<],$[1<<],$[1<<],$[1<<]}
	do
		for L in {$[$S1/2],$[3*$S1/2],$[$S2/2],$[3*$S2/4],$[2*$S2],$[4*$S2],$[8*S2]}
		do
			./acp1 $D $L
		done
	done
done


