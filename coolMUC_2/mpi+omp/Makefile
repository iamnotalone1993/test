# The input file of your program
IN = hello_world

# The output file of your program
OUT = program

# The path of input files
DIR_IN = ./examples

# The path of output files
DIR_OUT = ./out

# The path of GASPI's header files
# DIR_INC = /dss/dsshome1/lxc0B/ra56kom/gaspi/local/include

# The path of GASPI's library files
# DIR_LIB = /dss/dsshome1/lxc0B/ra56kom/gaspi/local/lib64

# Compiler
CC = mpiicc

# The performance flags for the compiler
FLAGS = -std=gnu++17 -O3 -pthread -fopenmp

.PHONY : all run clean

# Compile your program
all : $(DIR_OUT)/$(OUT)

$(DIR_OUT)/$(OUT) : $(DIR_IN)/$(IN).cpp
	export LANG=C; \
	$(CC) $(DIR_IN)/$(IN).cpp -o $(DIR_OUT)/$(OUT) $(FLAGS)

# Submit your job
run : $(DIR_OUT)/$(OUT)
	sbatch ./myjob.cmd

# Remove your executable
clean :
	rm -f $(DIR_OUT)/*
