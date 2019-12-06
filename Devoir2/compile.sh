#!/bin/bash

rm d2s
rm d2p
gcc -fopenmp d2s.c -o d2s
gcc -fopenmp d2p.c -o d2p