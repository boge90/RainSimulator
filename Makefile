all:
	gcc -O3 main.c -o main -lm -fopenmp -lglut -lGL -lGLU -std=c99
