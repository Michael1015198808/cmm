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
    void stop() {

    }
%}
digit [0-9]
oct [0-7]
hex [0-9a-fA-F]
letter_ [_a-zA-Z]
%option yylineno
demi_num 0|([1-9]{digit}*)
oct_num 0{oct}+
hex_num 0[Xx]{hex}+
float_exp [Ee][\+\-]?{digit}+
%x C_COMMENT

%%

"//".*

"/*"                {BEGIN(C_COMMENT);}
<C_COMMENT>"*/"     {BEGIN(INITIAL);}
<C_COMMENT>\n   
<C_COMMENT>.    
<C_COMMENT><<EOF>>  {
    lexical_error(yylineno, "unterminated comment.");
    BEGIN(INITIAL);
}

";"         {TOKEN(SEMI);}
","         {TOKEN(COMMA);}
"="         {TOKEN(ASSIGNOP);}
">"|"<"|">="|"<="|"=="|"!=" {TOKEN(RELOP);}
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
    yylval -> val_int = T_INT;
    return TYPE;
}
"float" {
    yylval = Singleton("TYPE");
    yylval -> func = type_printer;
    yylval -> val_int = T_FLOAT;
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

{digit}+\.{digit}+{float_exp}? {
    yylval = Singleton("FLOAT");
    yylval -> val_float = strtof(yytext, NULL);
    yylval -> func = float_printer;
    return FLOAT;
}

(({digit}+\.{digit}*)|(\.{digit}+)){float_exp} {
    yylval = Singleton("FLOAT");
    yylval -> val_float = strtof(yytext, NULL);
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


0[Xx][0-9a-zA-Z]+ {
    lexical_error(yylineno, "Illegal hexademical INT number: \"%s\".", yytext);
    return INT;
}

0{digit}+ {
    lexical_error(yylineno, "Illegal octal INT number: \"%s\".", yytext);
    return INT;
}

{digit}*\.{digit}+[Ee] {
    lexical_error(yylineno, "Illegal FLOAT number: \"%s\".", yytext);
    return FLOAT;
}

({letter_}|{digit})+ {
    lexical_error(yylineno, "Illegal ID format: \"%s\".", yytext);
    return ID;
}

{digit}*\.{digit}* {
    lexical_error(yylineno, "Illegal FLOAT number: \"%s\".", yytext);
    return FLOAT;
}

{digit}+\.{digit}*e(-+)? {
    lexical_error(yylineno, "Illegal FLOAT number: \"%s\".", yytext);
    return FLOAT;
}

. {
    lexical_error(yylineno, "Mysterious characters \"%s\".", yytext);
}

%%
