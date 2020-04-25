#ifndef __LIST_H__
#define __LIST_H__

#define LIST_START(name) \
    struct name { \
        struct name* next;
        //Add other information needed here

#define LIST_END \
    }

#define LIST(name, type) \
    LIST_START(name##_list) \
        type info; \
    LIST_END

#define LIST_ENTRY(name) \
    struct name##_list

#define LIST_INSERT_HEAD(head, elm) \
    do { \
        (elm) -> next = head; \
        (head) = (elm); \
    } while(0)

#define LIST_FOREACH(var, head) \
        for((var) = (head); (var); (var) = (var) -> next)

#endif //__LIST_H__
