main: main.c
	mpicc main.c -o main

test: test.c
	mpicc test.c -o test

	