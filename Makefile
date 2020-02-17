.DEFAULT_GOAL := default
.PHONY: default test syntax
build/lex.yy.c: lexical.l
	@flex -o build/lex.yy.c lexical.l
syntax: syntax.y
	@bison -o build/syntax.tab.c -d syntax.y
default: syntax build/lex.yy.c main.c
	@gcc -lfl -ly build/syntax.tab.c build/lex.yy.c main.c -o build/cc
test: default
	@./build/cc < test/1.cmm
