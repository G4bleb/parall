all:
	make -s gen
	make -s solve
gen: gen.c Sudoku.c
	gcc Sudoku.c gen.c -o gen
solve: solve.c Sudoku.c
	gcc Sudoku.c solve.c -o solve
clean:
	rm -f gen solve
remake:
	make -s clean
	make -s all