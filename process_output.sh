#!/bin/bash

echo "D,L,R,Clocks,ClocksPerAccess"

declare -A clocks_by_param
declare -A clocks_per_access_by_param

IFS=''
read # Leer primera línea, que es un resumen del Finisterrae
while IFS='=,' read _ D _ L _ R; do
    if [ "$D" = "" ]; then exit; fi     # Salir si no se leen datos (líneas finales del Finisterrae)
    IFS='=.'
    read _ clocks _
    IFS='='
    read _ clocks_per_access
    read

    echo "$D,$L,$R,$clocks,$clocks_per_access"
done
