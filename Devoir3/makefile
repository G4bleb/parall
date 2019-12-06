d3-2: d3-2.c MyMPI.h
	mpicc d3-2.c -o d3-2 -lm -Wall -Wconversion
run: d3-2
	mpirun -c 2 d3-2 matrix
floyd: floyd.c MyMPI.c MyMPI.h
	mpicc floyd.c MyMPI.c -o floyd -lm
run_floyd: floyd
	mpirun -c 2 floyd matrix
v1: v1.c MyMPI.h
	mpicc v1.c -o v1 -lm -Wall -Wconversion
run_v1: v1
	mpirun -c 2 v1 matrix