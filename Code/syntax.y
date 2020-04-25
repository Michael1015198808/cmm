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
    Specifier ExtDecList x_SEMI   {$$ = Node3("ExtDef");$$ -> func = def_handler;}
  | Specifier x_SEMI              {$$ = Node2("ExtDef");$$ -> func = def_handler;}
  | Specifier FunDec CompSt       {$$ = Node3("ExtDef");$$ -> func = fun_def_handler;}
  | Specifier FunDec SEMI         {$$ = Node3("ExtDef");$$ -> func = fun_dec_handler;}
  | error SEMI                    {syntax_error(@1.first_line, "Something wrong with declaration.");}
  ;
ExtDecList :
    VarDec                  {$$ = Node1("ExtDecList");$$ -> func = vardec_handler;}
  | VarDec COMMA ExtDecList {$$ = Node3("ExtDecList");$$ -> func = extdeclist_handler;}
  ;

//Specifiers
Specifier :
    TYPE                {$$ = Node1("Specifier");$$->func = type_handler;}
  | StructSpecifier     {$$ = Node1("Specifier");$$->func = struct_specifier_handler;}
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
    ID                    {$$ = Node1("VarDec");$$ -> func = variable_handler;}
  | VarDec LB INT x_RB    {$$ = Node4("VarDec");$$ -> func = array_dec_handler;}
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
    LC DefList StmtList RC  {$$ = Node4("CompSt");$$ -> func = compst_handler;}
  | LC DefList error RC     {syntax_error(@3.first_line, "Error in the sentence(missing ;?).");$$ = Node3("CompSt");}
  ;
StmtList :
    Stmt StmtList   {$$ = Node2("StmtList");}
  | %empty          {$$ = Node0("StmtList");}
  ;
Stmt :
    Exp x_SEMI                              {$$ = Node2("Stmt");}
  | CompSt                                  {$$ = Node1("Stmt");}
  | RETURN Exp x_SEMI                       {$$ = Node3("Stmt");$$ -> func = return_handler;}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = Node5("Stmt");$$ -> func = if_handler;}
  | IF LP Exp RP Stmt ELSE Stmt             {$$ = Node7("Stmt");$$ -> func = if_handler;}
  | WHILE LP Exp RP Stmt                    {$$ = Node5("Stmt");$$ -> func = while_handler;}
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
    Specifier DecList x_SEMI  {$$ = Node3("Def");$$ -> func = def_handler;}
  ;
DecList :
    Dec                 {$$ = Node1("DecList");}
  | Dec COMMA DecList   {$$ = Node3("DecList");}
  ;
Dec :
    VarDec              {$$ = Node1("Dec");$$ -> func = vardec_handler;}
  | VarDec ASSIGNOP Exp {$$ = Node3("Dec");$$ -> func = vardec_handler;}
  ;

//Expressions
Exp :
    Exp ASSIGNOP Exp    {$$ = Node3("Exp");$$ -> func = assign_handler;$$ -> cond = int_to_bool_cond_handler;}
  | Exp AND Exp         {$$ = Node3("Exp");$$ -> func = and_handler;}
  | Exp OR Exp          {$$ = Node3("Exp");$$ -> func = or_handler;}
  | Exp RELOP Exp       {$$ = Node3("Exp");$$ -> func = relop_handler;}
  | Exp PLUS Exp        {$$ = Node3("Exp");$$ -> func = arith_handler; $$ -> val_int = '+'; $$ -> cond = int_to_bool_cond_handler;}
  | Exp MINUS Exp       {$$ = Node3("Exp");$$ -> func = arith_handler; $$ -> val_int = '-'; $$ -> cond = int_to_bool_cond_handler;}
  | Exp STAR Exp        {$$ = Node3("Exp");$$ -> func = arith_handler; $$ -> val_int = '*'; $$ -> cond = int_to_bool_cond_handler;}
  | Exp DIV Exp         {$$ = Node3("Exp");$$ -> func = arith_handler; $$ -> val_int = '/'; $$ -> cond = int_to_bool_cond_handler;}
  | LP Exp RP %prec ELSE{$$ = Node3("Exp");$$ -> func = parentheses_handler; $$ -> cond = int_to_bool_cond_handler;}
  | MINUS Exp %prec HIGHER_THAN_MINUS
                        {$$ = Node2("Exp");$$ -> func = uminus_handler; $$ -> cond = int_to_bool_cond_handler;}
  | NOT Exp             {$$ = Node2("Exp");$$ -> func = not_handler;}
  | ID LP Args RP       {$$ = Node4("Exp");$$ -> func = fun_call_handler; $$ -> cond = int_to_bool_cond_handler;}
  | ID LP RP            {$$ = Node3("Exp");$$ -> func = fun_call_handler; $$ -> cond = int_to_bool_cond_handler;}
  | Exp LB Exp x_RB     {$$ = Node4("Exp");$$ -> func = array_access_handler; $$ -> cond = int_to_bool_cond_handler;}
  | Exp DOT ID          {$$ = Node3("Exp");$$ -> func = struct_access_handler; $$ -> cond = int_to_bool_cond_handler;}
  | ID                  {$$ = Node1("Exp");$$ -> func = id_handler; $$ -> cond = int_to_bool_cond_handler;}
  | INT                 {$$ = Node1("Exp");$$ -> func = int_handler; $$ -> cond = int_to_bool_cond_handler;}
  | FLOAT               {$$ = Node1("Exp");$$ -> func = float_handler;}
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

