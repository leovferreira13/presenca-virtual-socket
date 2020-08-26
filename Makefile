all:
	gcc -Wall cliente-aluno.c -o aluno
	gcc -Wall cliente-prof.c -o prof
	gcc -Wall servidor.c -o servidor
	
clean:
	rm -f aluno prof servidor
