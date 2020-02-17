OBJ := build/cc
CC := gcc
FLAGS := -lfl -ly -g -Iinclude

.DEFAULT_GOAL := $(OBJ)
.PHONY: test syntax clean gdb


build/lex.yy.c: src/lexical.l
	@flex -o build/lex.yy.c src/lexical.l
syntax: src/syntax.y
	@bison -o build/syntax.tab.c -d src/syntax.y
$(OBJ): syntax build/lex.yy.c src/main.c
	@$(CC) $(FLAGS) build/syntax.tab.c build/lex.yy.c src/main.c -o build/cc
test: $(OBJ)
	@./build/cc test/3.cmm
clean:
	@rm -rf build
gdb: $(OBJ)
	@gdb $(OBJ)
run: $(OBJ)
	@$(OBJ)
