#!/bin/bash
#SBATCH -J mv16proc
#SBATCH -o output_16.txt
#SBATCH -e error_16.txt
#SBATCH -n 16
#SBATCH -t 00:5:00
#SBATCH -p scit5

mpirun -np 16 ./testmv
