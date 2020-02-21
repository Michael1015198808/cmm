%{
    #include "syntax.tab.h"
    #include "handlers.h"
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
letter_ [_a-zA-Z]
%option yylineno
demi_num 0|([1-9]{digit}*)
oct_num 0{oct}+
hex_num 0x{hex}+
%%

\/\/.*
"/*"        {
    char c = input();
    do {
        if(c == '*') {
            c = input();
            if(c == '/') {
                break;
            }
        } else {
            c = input();
        }
    } while(1);
}
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
"int" {
    yylval = Singleton("TYPE");
    yylval -> func = type_printer;
    yylval -> val_int = 1;
    return TYPE;
}
"float" {
    yylval = Singleton("TYPE");
    yylval -> func = type_printer;
    yylval -> val_int = 2;
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
[ \t\r]
"\n"        {yycolumn = 1;}

{digit}+\.{digit}*([Ee][\+\-]?{digit}+)? {
    yylval = Singleton("FLOAT");
    yylval -> val_float = atof(yytext);
    yylval -> func = float_printer;
    return FLOAT;
}

\.{digit}+([Ee][\+\-]?{digit}+)? {
    yylval = Singleton("FLOAT");
    yylval -> val_float = atof(yytext);
    yylval -> func = float_printer;
    return FLOAT;
}

{demi_num}|{oct_num}|{hex_num} {
    yylval = Singleton("INT");
    yylval -> val_int = strtol(yytext, NULL, 0);
    yylval -> func = int_printer;
    return INT;
}

{letter_}({letter_}|{digit})* {
    yylval = Singleton("ID");
    asprintf((char**)&yylval -> val_str, "%s", yytext);
    yylval -> func = id_printer;
    return ID;
}

(0x?)?[0-9a-zA-Z]+ {
    error e;
    e.type = 'A';
    e.lineno = yylineno;
    asprintf(&e.msg, "Bad INT format: \"%s\"", yytext);
    cmm_error(e);
    return INT;
}

{digit}*\.{digit}*([Ee][\+\-]?{digit}*)? {
    error e;
    e.type = 'A';
    e.lineno = yylineno;
    asprintf(&e.msg, "Bad FLOAT format: \"%s\"", yytext);
    cmm_error(e);
    return FLOAT;
}

. {
    error e;
    e.type = 'A';
    e.lineno = yylineno;
    asprintf(&e.msg, "Mysterious characters \"%s\"", yytext);
    cmm_error(e);
}

%%
