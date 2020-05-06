#ifndef __OPTIMIZATION_H__
#define __OPTIMIZATION_H__

extern int optimization_level;
#define OPTIMIZE(thing) \
    (optimization_level >= thing)

#define ARITH_CONSTANT 1
#define LOGIC_CONSTANT 1
#define A_PLUS_NEG_B   1//a + -b into a - b
                        //a - -b into a + b
#define DUAL_UMINUS    1// --exp into exp
#endif //__OPTIMIZATION_H__
