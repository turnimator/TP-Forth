tpforth: tpforth.o variable.o task.o runtime.o program.o p_code.o parser.o logg.o dictionary.o custom.o compiler.o builtins.o
	cc $^ -o $@

builtins.o : builtins.c
	cc -c $< -o $@

compiler.o : compiler.c
	cc -c $< -o $@

custom.o : custom.c
	cc -c $< -o $@

dictionary.o : dictionary.c
	cc -c $< -o $@

logg.o : logg.c
	cc -c $< -o $@

parser.o : parser.c
	cc -c $< -o $@

p_code.o : p_code.c
	cc -c $< -o $@

program.o : program.c
	cc -c $< -o $@

runtime.o : runtime.c
	cc -c $< -o $@

task.o : task.c
	cc -c $< -o $@

tpforth.o : tpforth.c
	cc -c $< -o $@

variable.o : variable.c
	cc -c $< -o $@

clean :
	rm tpforth *.o