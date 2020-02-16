digit [0-9]
letter [_a-zA-Z]
%option yylineno
%%

\/\/.*
";" {puts("SEMI");}
"," {puts("COMMA");}
"=" {puts("ASSIGNOP");}
">"|"<"|">="|"<="|"=="|"!=" {puts("RELOP");}
"+" {puts("PLUS");}
"-" {puts("MINUS");}
"*" {puts("STAR");}
"/" {puts("DIV");}
"&&" {puts("AND");}
"||" {puts("OR");}
"." {puts("DOT");}
"!" {puts("NOT");}
int|float {puts("TYPE");}
"(" {puts("LP");}
")" {puts("RP");}
"[" {puts("LB");}
"]" {puts("RB");}
"{" {puts("LC");}
"}" {puts("RC");}
"struct" {puts("STRUCT");}
"return" {puts("RETURN");}
"if" {puts("IF");}
"else" {puts("ELSE");}
"while" {puts("WHILE");}
" "
"\n"

{digit}+ {printf("INT:%d\n", atoi(yytext));}
{letter}({letter}|{digit})* {printf("ID:%s\n", yytext);}

. {printf("Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext);}

%%
