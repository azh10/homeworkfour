default: virtualmachine.c lexer.c parser.c
	echo 'nothing selected program only compiled'
	gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	gcc -o parser parser.c

lex: virtualmachine.c lexer.c parser.c
	@gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	@gcc -o parser parser.c
	./lexer
	@cat lexemetable.txt 

parse: virtualmachine.c lexer.c parser.c
	@gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	gcc -o parser parser.c
	./lexer
	@cat -n lexemetable.txt
	./parser
	@cat input.txt

parseL: virtualmachine.c lexer.c parser.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	gcc -o parser parser.c
	./lexer
	./parser -l

compile: virtualmachine.c lexer.c parser.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	gcc -o parser parser.c
	./lexer
	./parser
	./virtualmachine

compileL: virtualmachine.c lexer.c parser.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o virtualmachine virtualmachine.c
	gcc -o lexer lexer.c
	gcc -o parser parser.c
	./lexer
	./parser -l
	./virtualmachine
