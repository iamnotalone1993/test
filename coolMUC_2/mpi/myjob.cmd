#!/bin/bash
#SBATCH -J mpi
#SBATCH -o ./jobs/%x.%j.%N.out
#SBATCH -D ./
#SBATCH --clusters=cm2_tiny
#SBATCH --partition=cm2_tiny
##SBATCH --qos=cm2_std
#SBATCH --get-user-env
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=28
#SBATCH --mail-type=end
#SBATCH --mail-user=iamnotalone1993@gmail.com
#SBATCH --export=NONE
#SBATCH --time=00:10:00

# Load SLURM
module load slurm_setup

# Variables
# abc=

# Run your program
mpiexec -n $SLURM_NTASKS ./out/program
