#!/bin/bash

echo "D,L,R,Clocks,ClocksPerAccess"

declare -A clocks_by_param
declare -A clocks_per_access_by_param

while IFS='=,' read _ D _ L _ R; do
    IFS='=.'
    read -r _ clocks _
    IFS='='
    read -r _ clocks_per_access
    read

    echo "$D,$L,$R,$clocks,$clocks_per_access"
done
