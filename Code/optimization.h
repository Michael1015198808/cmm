#ifndef __OPTIMIZATION_H__
#define __OPTIMIZATION_H__

extern int optimization_level;
#define OPTIMIZE(thing) \
    (optimization_level >= thing)
#define EXP_CONSTANT 1
#endif //__OPTIMIZATION_H__