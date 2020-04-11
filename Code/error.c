#include "common.h"
#include "error.h"
extern int error_cnt;

int lexical_error(int lineno, const char* fmt, ...) {
    ++error_cnt;
    va_list ap;
    va_start(ap, fmt);
    printf("Error type A at Line %d: ", lineno);
    vprintf(fmt, ap);
    putchar('\n');
    return 0;
}

int syntax_error(int lineno, const char* fmt, ...) {
    static int last_line = 0;
    if(lineno == last_line) {
        return -1;//Prevent reporting too much errors
    }
    last_line = lineno;
    ++error_cnt;
    va_list ap;
    va_start(ap, fmt);
    printf("Error type B at Line %d: ", lineno);
    vprintf(fmt, ap);
    putchar('\n');
    return 0;
}

static char* semantic_errors_msg[] = {
    [UNDEFINE_VARIABLE] = "Undefined variable \"%s\".\n",
    [UNDEFINE_FUNCTION] = "Undefined function \"%s\".\n",
    [REDEFINE_VARIABLE] = "Redefined variable \"%s\".\n",
    [REDEFINE_FUNCTION] = "Redefined function \"%s\".\n",
      [ASSIGN_MISMATCH] = "Type mismatched for assignment.\n",
               [LVALUE] = "The left-hand side of an assignment must be a variable.\n",
     [OPERAND_MISMATCH] = "Type mismatched for operands.\n",
      [RETURN_MISMATCH] = "Type mismatched for return.\n",
    [FUNCTION_MISMATCH] = "Function \"%s\" is not applicable for given arguments.\n",
         [NOT_ARRAY]    = "Not an array.\n",
         [NOT_FUNCTION] = "\"%s\" is not a function.\n",
         [NOT_INT]      = "Not an integer.\n",
     [ILLEGAL_USE]      = "Illegal use of \"%s\".\n",
    [NONEXIST_FIELD]    = "Non-existent field \"%s\".\n",
    [REDEFINE_FIELD]    = "Redefined field \"%s\".\n",
   [DUPLICATE_NAME]     = "Duplicated name \"%s\".\n",
    [UNDEFINE_STRUCTURE]= "Undefined structure \"%s\".\n",
[DEC_UNDEFINE_FUNCTION] = "Undefined function \"%s\".\n",
    [INCONSIS_DEC]      = "Inconsistent declaration of function \"%s\".\n",
};

int vsemantic_error(int lineno, int errorno, va_list ap) {
    int ret = 0;
    ret += printf("Error type %d at Line %d: ", errorno, lineno);
    ret +=  vprintf(semantic_errors_msg[errorno], ap);
    return ret;
}

int semantic_error(int lineno, int errorno, ...) {
    va_list ap;
    va_start(ap, errorno);
    return vsemantic_error(lineno, errorno, ap);
}
