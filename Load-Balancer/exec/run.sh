#!/bin/bash

rm -f log.txt
make

sleep 5

mpirun --oversubscribe -np 5 --map-by node --hostfile ./hostfile $1

make clean
