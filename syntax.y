
%{
    #include <stdio.h>
    int yylex();
    int yyerror(char* msg);
%}
%union {
    int type_int;
    float type_float;
    char* type_charp;
    double type_double;
}

%token <type_int> INT
%token <type_float> FLOAT
%token <type_charp> ID

%token SEMI COMMA
%token TYPE
%token STRUCT

%nonassoc LOWER_THAN_ELSE
%nonassoc RETURN IF ELSE WHILE

%right ASSIGNOP

%left OR

%left AND 

%left RELOP

%left PLUS MINUS 

%left STAR DIV

%right HIGHER_THAN_MINUS
%right NOT

%left DOT
%left LP RP LB RB LC RC
%locations


%%
Program :
    ExtDefList
  ;
ExtDefList :
    ExtDef ExtDefList
  |
  ;
ExtDef :
    Specifier ExtDecList SEMI
  | Specifier SEMI
  | Specifier FunDec CompSt
  ;
ExtDecList :
    VarDec
  | VarDec COMMA ExtDecList
  ;

Specifier :
    TYPE
  | StructSpecifier
  ;
StructSpecifier :
    STRUCT OptTag LC DefList RC
  | STRUCT Tag
  ;
OptTag :
    ID
  |
  ;
Tag :
    ID
  ;

VarDec :
    ID
  | VarDec LB INT RB
  ;
FunDec :
    ID LP VarList RP
  | ID LP RP
  ;
VarList :
    ParamDec COMMA VarList
  | ParamDec
  ;
ParamDec :
    Specifier VarDec
  ;

CompSt :
    LC DefList StmtList RC
  ;
StmtList :
    Stmt StmtList
  | 
  ;
Stmt :
    Exp SEMI
  | CompSt
  | RETURN Exp SEMI
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
  | IF LP Exp RP Stmt ELSE Stmt
  | WHILE LP Exp RP Stmt
  ;

DefList :
    Def DefList
  | 
  ;
Def :
    Specifier DecList SEMI
  ;
DecList :
    Dec
  | Dec COMMA DecList
  ;
Dec :
    VarDec
  | VarDec ASSIGNOP Exp
  ;

Exp :
    Exp ASSIGNOP Exp
  | Exp AND Exp
  | Exp OR Exp
  | Exp RELOP Exp
  | Exp PLUS Exp
  | Exp MINUS Exp
  | Exp STAR Exp
  | Exp DIV Exp
  | LP Exp RP
  | MINUS Exp %prec HIGHER_THAN_MINUS
  | NOT Exp
  | ID LP Args RP
  | ID LP RP
  | Exp LB Exp RB
  | Exp DOT ID
  | ID
  | INT
  | FLOAT
  ;
Args :
    Exp COMMA Args
  | Exp
  ;
%%
