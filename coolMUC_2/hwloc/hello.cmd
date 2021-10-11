#!/bin/bash
#SBATCH -J hello
#SBATCH -o ./%x.%j.%N.out
#SBATCH -D ./
#SBATCH --clusters=cm2_tiny
#SBATCH --partition=cm2_tiny
#SBATCH --get-user-env
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=28
#SBATCH --mail-type=end
#SBATCH --mail-user=dangcse11hcmut@gmail.com
#SBATCH --export=NONE
#SBATCH --time=00:10:00
module load slurm_setup
mpiexec -n $SLURM_NTASKS ./hello
