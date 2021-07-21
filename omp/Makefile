#The main file name of your program
OUT = atomic

#The performance flags for the compiler
FLAGS = -std=gnu++17 -O3 -fopenmp

.PHONY: all run clean

#Compile your program
all : $(OUT)

$(OUT) : $(OUT).cpp
	g++ $(OUT).cpp -o $(OUT) $(FLAGS)

#Run your program
run : $(OUT)
	for i in `seq 1`; do ./$(OUT); done

#Remove your executable
clean :
	rm -f $(OUT)

