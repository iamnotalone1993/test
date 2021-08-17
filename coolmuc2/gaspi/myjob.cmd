#!/bin/bash
#SBATCH -J gaspi
#SBATCH -o ./jobs/%x.%j.%N.out
#SBATCH -D ./
#SBATCH --clusters=cm2_tiny
#SBATCH --partition=cm2_tiny
##SBATCH --qos=cm2_std
#SBATCH --get-user-env
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --mail-type=end
#SBATCH --mail-user=iamnotalone1993@gmail.com
#SBATCH --export=NONE
#SBATCH --time=00:10:00

# Load SLURM
module load slurm_setup

# Variables
gaspirun=/dss/dsshome1/lxc0B/ra56kom/gaspi/local/bin/gaspi_run

# Run the program
unset KMP_AFFINITY
export OMP_NUM_THREADS=28
export OMP_PROC_BIND="close"
export OMP_PLACES="cores"
$gaspirun -n $SLURM_NTASKS ./out/program
