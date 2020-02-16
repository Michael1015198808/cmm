.DEFAULT_GOAL := default
.PHONY: default test
build/lex.yy.c: lexical.l
	@flex -o build/lex.yy.c lexical.l
default: build/lex.yy.c main.c
	@gcc -lfl build/lex.yy.c main.c -o build/cc
test: default
	@./build/cc < test/1.cmm
