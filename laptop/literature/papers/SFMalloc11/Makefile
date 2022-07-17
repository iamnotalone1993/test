# The input file of your program
IN = stack

# The main file name of your program
OUT = out

# The compiler
CC = g++

# The performance flags for the compiler
CFLAGS = -std=c++17 -O3 -pthread -latomic

.PHONY: all run clean

# Compile your program
all : $(OUT)

$(OUT) : $(IN).cpp
	$(CC) $(IN).cpp -o $(OUT) $(CFLAGS)

# Run your program
run : $(OUT)
	for i in `seq 100`; do ./$(OUT); done

# Remove your executable
clean :
	rm -f $(OUT)

