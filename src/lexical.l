%{
    #include "common.h"
    #include "syntax.tab.h"
    int yycolumn = 1;
    #define TOKEN(name)\
    do{ \
        yylval = Singleton(#name); \
        return name; \
    } while(0)
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
%}
digit [0-9]
oct [0-7]
hex [0-9a-fA-F]
letter [_a-zA-Z]
%option yylineno
%%

\/\/.*
"/*".*"*/"
";"         {TOKEN(SEMI);}
","         {TOKEN(COMMA);}
"="         {TOKEN(ASSIGNOP);}
">"|"<"|">="|"<="|"=="|"!=" {return RELOP;}
"+"         {TOKEN(PLUS);}
"-"         {TOKEN(MINUS);}
"*"         {TOKEN(STAR);}
"/"         {TOKEN(DIV);}
"&&"        {TOKEN(AND);}
"||"        {TOKEN(OR);}
"."         {TOKEN(DOT);}
"!"         {TOKEN(NOT);}
int|float {
    yylval = Singleton("TYPE: %s", yytext);
    return TYPE;
}
"("         {TOKEN(LP);}
")"         {TOKEN(RP);}
"["         {TOKEN(LB);}
"]"         {TOKEN(RB);}
"{"         {TOKEN(LC);}
"}"         {TOKEN(RC);}
"struct"    {TOKEN(STRUCT);}
"return"    {TOKEN(RETURN);}
"if"        {TOKEN(IF);}
"else"      {TOKEN(ELSE);}
"while"     {TOKEN(WHILE);}
" "
"\n"        {yycolumn = 1;}

0|([1-9]{digit}*)\.{digit}+ {
    yylval = Singleton("FLOAT: %lf", atof(yytext));
    return FLOAT;
}

0|([1-9]{digit}*) {
    yylval = Singleton("INT: %s", yytext);
    return INT;
}
0{oct}+ {
    yylval = Singleton("INT: %d", strtol(yytext + 1, NULL, 8));
    return INT;
}
0x{hex}+ {
    yylval = Singleton("INT: %d", strtol(yytext + 2, NULL, 16));
    return INT;
}
{letter}({letter}|{digit})* {
    yylval = Singleton("ID: %s", yytext);
    return ID;
}

. {printf("Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext);}

%%
