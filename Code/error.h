#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>

int lexical_error(int lineno, const char* fmt, ...);
int syntax_error(int lineno, const char* fmt, ...);
int semantic_error(int lineno, int errorno, ...);
int vsemantic_error(int lineno, int errorno, va_list);

typedef enum {
    UNDEFINE_VARIABLE = 1,
    UNDEFINE_FUNCTION = 2,
    REDEFINE_VARIABLE = 3,
    REDEFINE_FUNCTION = 4,
      ASSIGN_MISMATCH = 5,
               LVALUE = 6,
     OPERAND_MISMATCH = 7,
      RETURN_MISMATCH = 8,
    FUNCTION_MISMATCH = 9,
         NOT_ARRAY    = 10,
         NOT_FUNCTION = 11,
         NOT_INT      = 12,
     ILLEGAL_USE      = 13,
    NONEXIST_FIELD    = 14,
    REDEFINE_FIELD    = 15,
   DUPLICATE_NAME     = 16,
    UNDEFINE_STRUCTURE= 17,
DEC_UNDEFINE_FUNCTION = 18,
    INCONSIS_DEC      = 19,
} semantic_errors;

#endif // __ERROR_H__
