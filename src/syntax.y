%{
    #include "common.h"

    //#define Node0(name) Node(name, yylsp[0].first_line, 0)
    #define Node0(name) NULL
    #define Node1(name) Node(name, yylsp[0].first_line, 1, yyvsp[0])
    #define Node2(name) Node(name, yylsp[-1].first_line, 2, yyvsp[-1], yyvsp[0])
    #define Node3(name) Node(name, yylsp[-2].first_line, 3, yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node4(name) Node(name, yylsp[-3].first_line, 4, yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node5(name) Node(name, yylsp[-4].first_line, 5, yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node6(name) Node(name, yylsp[-5].first_line, 6, yyvsp[-5], yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node7(name) Node(name, yylsp[-6].first_line, 7, yyvsp[-6], yyvsp[-5], yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
%}

%token INT
%token FLOAT
%token ID

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
    ExtDefList {
        $$ = Node1("Program");
        root = $$;
    }
  ;
ExtDefList :
    ExtDef ExtDefList   {$$ = Node2("ExtDefList");}
  | %empty              {$$ = Node0("ExtDefList");}
  ;
ExtDef :
    Specifier ExtDecList SEMI   {$$ = Node3("ExtDef");}
  | Specifier SEMI              {$$ = Node2("ExtDef");}
  | Specifier FunDec CompSt     {$$ = Node3("ExtDef");}
  ;
ExtDecList :
    VarDec                  {$$ = Node1("ExtDecList");}
  | VarDec COMMA ExtDecList {$$ = Node3("ExtDecList");}
  ;

Specifier :
    TYPE                {$$ = Node1("Specifier");}
  | StructSpecifier     {$$ = Node1("Specifier");}
  ;
StructSpecifier :
    STRUCT OptTag LC DefList RC {$$ = Node5("StructSpecifier");}
  | STRUCT Tag                  {$$ = Node2("StructSpecifier");}
  ;
OptTag :
    ID      {$$ = Node1("OptTag");}
  | %empty  {$$ = Node0("OptTag");}
  ;
Tag :
    ID {$$ = Node1("Tag");}
  ;

VarDec :
    ID                  {$$ = Node1("VarDec");}
  | VarDec LB INT RB    {$$ = Node4("VarDec");}
  ;
FunDec :
    ID LP VarList RP    {$$ = Node4("FunDec");}
  | ID LP RP            {$$ = Node3("FunDec");}
  ;
VarList :
    ParamDec COMMA VarList  {$$ = Node3("VarList");}
  | ParamDec                {$$ = Node1("VarList");}
  ;
ParamDec :
    Specifier VarDec    {$$ = Node2("ParamDec");}
  ;

CompSt :
    LC DefList StmtList RC  {$$ = Node4("CompSt");}
  ;
StmtList :
    Stmt StmtList   {$$ = Node2("StmtList");}
  | %empty          {$$ = Node0("StmtList");}
  ;
Stmt :
    Exp SEMI                                {$$ = Node2("Stmt");}
  | CompSt                                  {$$ = Node1("Stmt");}
  | RETURN Exp SEMI                         {$$ = Node3("Stmt");}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = Node5("Stmt");}
  | IF LP Exp RP Stmt ELSE Stmt             {$$ = Node7("Stmt");}
  | WHILE LP Exp RP Stmt                    {$$ = Node5("Stmt");}
  | error SEMI                              {$$ = Node1("Stmt");}
  ;

DefList :
    Def DefList {$$ = Node2("DefList");}
  | %empty      {$$ = Node0("DefList");}
  ;
Def :
    Specifier DecList SEMI  {$$ = Node3("Def");}
  ;
DecList :
    Dec                 {$$ = Node1("DecList");}
  | Dec COMMA DecList   {$$ = Node3("DecList");}
  ;
Dec :
    VarDec              {$$ = Node1("Dec");}
  | VarDec ASSIGNOP Exp {$$ = Node3("Dec");}
  ;

Exp :
    Exp ASSIGNOP Exp    {$$ = Node3("Exp");}
  | Exp AND Exp         {$$ = Node3("Exp");}
  | Exp OR Exp          {$$ = Node3("Exp");}
  | Exp RELOP Exp       {$$ = Node3("Exp");}
  | Exp PLUS Exp        {$$ = Node3("Exp");}
  | Exp MINUS Exp       {$$ = Node3("Exp");}
  | Exp STAR Exp        {$$ = Node3("Exp");}
  | Exp DIV Exp         {$$ = Node3("Exp");}
  | LP Exp RP           {$$ = Node3("Exp");}
  | MINUS Exp %prec HIGHER_THAN_MINUS
                        {$$ = Node2("Exp");}
  | NOT Exp             {$$ = Node2("Exp");}
  | ID LP Args RP       {$$ = Node4("Exp");}
  | ID LP RP            {$$ = Node3("Exp");}
  | Exp LB Exp RB       {$$ = Node4("Exp");}
  | Exp DOT ID          {$$ = Node3("Exp");}
  | ID                  {$$ = Node1("Exp");}
  | INT                 {$$ = Node1("Exp");}
  | FLOAT               {$$ = Node1("Exp");}
  ;
Args :
    Exp COMMA Args  {$$ = Node3("Args");}
  | Exp             {$$ = Node1("Args");}
  ;
%%
