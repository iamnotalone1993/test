#The input file name of your program
IN = sequential_alternating

#The output file name of your program
OUT = main

#The path of the benchmarks
BENDIR = ./dds/stack/ben

#The path of BCL Core
LIBDIR = ./dds/bcl_ext

#The performance flags for the compiler
FLAGS = -std=gnu++17 -O3

.PHONY : all run clean

#Compile your program
all : $(OUT)

$(OUT) : $(BENDIR)/$(IN).cpp
	export LANG=C; \
	mpicxx $(BENDIR)/$(IN).cpp -o $(OUT) $(FLAGS) -I$(LIBDIR)

#Run your program
run : $(OUT)
	sbatch ./script

#Remove your executable
clean :
	rm -f $(OUT)*
