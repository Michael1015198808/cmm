
%union {
    int type_int;
    float type_float;
    char* type_charp;
    double type_double;
}

%token <type_int> INT
%token <type_charp> ID
%token PLUS MINUS STAR DIV
%token SEMI COMMA
%token ASSIGNOP
%token RELOP
%token AND OR
%token DOT
%token NOT
%token TYPE
%token LP RP LB RB LC RC
%token STRUCT
%token RETURN IF ELSE WHILE
%locations


%%
Calc : 
%%
