#!/bin/bash
#SBATCH -J omp
#SBATCH -o ./jobs/%x.%j.%N.out
#SBATCH -D ./
#SBATCH --clusters=cm2_tiny
#SBATCH --partition=cm2_tiny
##SBATCH --qos=cm2_std
#SBATCH --get-user-env
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=28
#SBATCH --mail-type=end
#SBATCH --mail-user=iamnotalone1993@gmail.com
#SBATCH --export=NONE
#SBATCH --time=00:10:00

# Important
module load slurm_setup

# Variables
# gaspirun=

# Run the program
unset KMP_AFFINITY
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND="close"
export OMP_PLACES="cores"
export OMP_DISPLAY_ENV="true"
export OMP_DISPLAY_AFFINITY="true"
./out/program
