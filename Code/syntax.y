%{
    #include "lex.yy.c"
%}

%{

    //#define Node0(name) Node(name, yylsp[0].first_line, 0)
    #define Node0(name) NULL
    #define Node1(name) Node(name, yylsp[0].first_line,  1, yyvsp[0])
    #define Node2(name) Node(name, yylsp[-1].first_line, 2, yyvsp[-1], yyvsp[0])
    #define Node3(name) Node(name, yylsp[-2].first_line, 3, yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node4(name) Node(name, yylsp[-3].first_line, 4, yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node5(name) Node(name, yylsp[-4].first_line, 5, yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node6(name) Node(name, yylsp[-5].first_line, 6, yyvsp[-5], yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])
    #define Node7(name) Node(name, yylsp[-6].first_line, 7, yyvsp[-6], yyvsp[-5], yyvsp[-4], yyvsp[-3], yyvsp[-2], yyvsp[-1], yyvsp[0])

    #define set_semantic(name) \
    do{ \
        (yyval) -> kind = STMT; \
        (yyval) -> semantic = name##_semantic_handler; \
    } while(0)

    #define set_arith(name) \
    do{ \
        (yyval) -> kind = ARITH; \
        (yyval) -> arith = name##_arith_handler; \
    } while(0)
    #define set_cond(name) \
    do{ \
        (yyval) -> kind = COND; \
        (yyval) -> cond = name##_cond_handler; \
    } while(0)
    int yyerror(char* msg) {}
    char* get_vardec_name();
    node* root;
%}

%nonassoc LOWEST

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
//High-level Definitions
Program :
    ExtDefList {
        $$ = Node1("Program");
        root = $$;
    }
  ;
ExtDefList :
    ExtDef ExtDefList   {$$ = Node2("ExtDefList");}
  | %empty              {$$ = Node0("ExtDefList"); @$.first_line = yylineno;}
  ;
ExtDef :
    Specifier ExtDecList x_SEMI   {$$ = Node3("ExtDef");set_semantic(def);}
  | Specifier x_SEMI              {$$ = Node2("ExtDef");set_semantic(def);}
  | Specifier FunDec CompSt       {$$ = Node3("ExtDef");set_semantic(fun_def);}
  | Specifier FunDec SEMI         {$$ = Node3("ExtDef");set_semantic(fun_dec);}
  | error SEMI                    {syntax_error(@1.first_line, "Something wrong with declaration.");}
  ;
ExtDecList :
    VarDec                  {$$ = Node1("ExtDecList");set_semantic(vardec);}
  | VarDec COMMA ExtDecList {$$ = Node3("ExtDecList");set_semantic(extdeclist);}
  ;

//Specifiers
Specifier :
    TYPE                {$$ = Node1("Specifier");set_semantic(type);}
  | StructSpecifier     {$$ = Node1("Specifier");set_semantic(struct_specifier);}
  ;
StructSpecifier :
    STRUCT OptTag LC DefList RC {$$ = Node5("StructSpecifier");}
  | STRUCT OptTag LC error RC   {syntax_error(@4.first_line, "Some thing wrong inside the structure.");}
  | STRUCT Tag                  {$$ = Node2("StructSpecifier");}
  ;
OptTag :
    ID      {$$ = Node1("OptTag");}
  | %empty  {$$ = Node0("OptTag");}
  ;
Tag :
    ID {$$ = Node1("Tag");}
  ;

//Declarators
VarDec :
    ID                    {$$ = Node1("VarDec");set_semantic(variable);}
  | VarDec LB INT x_RB    {$$ = Node4("VarDec");set_semantic(array_dec);}
  | VarDec LB error RB    {syntax_error(@3.first_line, "Size of array %s should be an integer.", get_vardec_name($1));$$ = Node0("VarDec");}
  ;
FunDec :
    ID LP VarList x_RP  {$$ = Node4("FunDec");}
  | ID LP error RP      {syntax_error(@3.first_line, "Something wrong inside parameter definitions.", get_vardec_name($1));$$ = Node0("FunDec");}
  | ID LP x_RP          {$$ = Node3("FunDec");}
  ;
VarList :
    ParamDec COMMA VarList  {$$ = Node3("VarList");}
  | ParamDec                {$$ = Node1("VarList");}
  ;
ParamDec :
    Specifier VarDec    {$$ = Node2("ParamDec");}
  | VarDec           {syntax_error(@1.last_line, "Missing type of %s.", get_vardec_name($1));$$ = Node0("ParamDec");}
  | Specifier           {syntax_error(@1.last_line, "parameter name omitted.");$$ = Node0("ParamDec");}
  ;

//Statements
CompSt :
    LC DefList StmtList RC  {$$ = Node4("CompSt");set_semantic(compst);}
  | LC DefList error RC     {syntax_error(@3.first_line, "Error in the sentence(missing ;?).");$$ = Node3("CompSt");}
  ;
StmtList :
    Stmt StmtList   {$$ = Node2("StmtList");}
  | %empty          {$$ = Node0("StmtList");}
  ;
Stmt :
    Exp x_SEMI                              {$$ = Node2("Stmt");set_semantic(stmt_exp);}
  | CompSt                                  {$$ = Node1("Stmt");}
  | RETURN Exp x_SEMI                       {$$ = Node3("Stmt");set_semantic(return);}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = Node5("Stmt");set_semantic(if);}
  | IF LP Exp RP Stmt ELSE Stmt             {$$ = Node7("Stmt");set_semantic(if_else);}
  | WHILE LP Exp RP Stmt                    {$$ = Node5("Stmt");set_semantic(while);}
  | LP Exp SEMI %prec LOWEST                {syntax_error(@1.first_line, "Missing \")\".");}
  | Bad_exp error                           {syntax_error(@2.first_line, "Something wrong with the expression(expect expression before token '%s' ).", yylval->name);} SEMI                      
  | LP error RP SEMI                        {syntax_error(@2.first_line, "Expect expression before ')'.");}
  | IF LP error RP Stmt                     {syntax_error(@3.first_line, "Expected expression before ')'.");$$ = Node0("Stmt");}
  | error SEMI                              {syntax_error(@1.first_line, "Something wrong with the statement.");}
  ;

//Local Definitions
DefList :
    Def DefList {$$ = Node2("DefList");}
  | %empty      {$$ = Node0("DefList");}
  ;
Def :
    Specifier DecList x_SEMI  {$$ = Node3("Def");set_semantic(def);}
  ;
DecList :
    Dec                 {$$ = Node1("DecList");}
  | Dec COMMA DecList   {$$ = Node3("DecList");}
  ;
Dec :
    VarDec              {$$ = Node1("Dec");set_semantic(vardec);}
  | VarDec ASSIGNOP Exp {$$ = Node3("Dec");set_semantic(vardec);}
  ;

//Expressions
Exp :
    Exp ASSIGNOP Exp    {$$ = Node3("Exp");set_arith(assign);}
  | Exp PLUS Exp        {$$ = Node3("Exp");set_arith(arith);$$ -> val_int = '+';}
  | Exp MINUS Exp       {$$ = Node3("Exp");set_arith(arith);$$ -> val_int = '-';}
  | Exp STAR Exp        {$$ = Node3("Exp");set_arith(arith);$$ -> val_int = '*';}
  | Exp DIV Exp         {$$ = Node3("Exp");set_arith(arith);$$ -> val_int = '/';}
  | LP Exp RP %prec ELSE{$$ = $2;}
  | ID LP Args RP       {$$ = Node4("Exp");set_arith(fun_call);}
  | ID LP RP            {$$ = Node3("Exp");set_arith(fun_call);}
  | Exp LB Exp x_RB     {$$ = Node4("Exp");set_arith(array_access);}
  | Exp DOT ID          {$$ = Node3("Exp");set_arith(struct_access);}
  | ID                  {$$ = Node1("Exp");set_arith(id);}
  | INT                 {$$ = Node1("Exp");set_arith(int);}
  | FLOAT               {$$ = Node1("Exp");set_arith(float);}
  | MINUS Exp %prec HIGHER_THAN_MINUS
                        {$$ = Node2("Exp");set_arith(uminus);}
  | Exp AND Exp         {$$ = Node3("Exp");set_cond(and);}
  | Exp OR Exp          {$$ = Node3("Exp");set_cond(or);}
  | Exp RELOP Exp       {$$ = Node3("Exp");set_cond(relop);}
  | NOT Exp             {$$ = Node2("Exp");set_cond(not);}
  | ID LP error RP      {syntax_error(@3.first_line, "Expect expression before ')'.");}
  ;
Bad_exp :
    Exp ASSIGNOP
  | Exp AND
  | Exp OR
  | Exp RELOP
  | Exp PLUS
  | Exp MINUS
  | Exp STAR
  | Exp DIV
  | Exp DOT
  ;
Args :
    Exp COMMA Args  {$$ = Node3("Args");}
  | Exp             {$$ = Node1("Args");}
  ;

x_SEMI:
    SEMI
  | error {syntax_error(@1.first_line, "Missing \";\".");}
  ;
x_RB:
    RB
  | error {syntax_error(@1.first_line, "Missing \"]\".");}
  ;
x_RP:
    RP
  | error {syntax_error(@1.first_line, "Missing \")\".");}
  ;
%%

