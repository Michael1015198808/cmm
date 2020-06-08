#include "common.h"
#include "table.h"
#include "type.h"

const static struct Type_ type_int_ = {
    .size = 4,
    .kind = BASIC,
    .basic = T_INT
}, type_float_ = {
    .size = 8,
    .kind = BASIC,
    .basic = T_FLOAT
};

CType type_int = &type_int_;
CType type_float = &type_float_;

Type to_array(CType base, unsigned size) {
    Type ret = new(struct Type_);
    ret -> size = base -> size * size;
    ret -> kind = ARRAY;
    ret -> array.elem = (Type)base;
    ret -> array.size = size;
    return ret;
}

Type to_struct(int cnt, ...) {
    Type ret = new(struct Type_);
    ret -> kind = STRUCTURE;
    if(cnt == 0) {
        ret -> structure = NULL;
    } else {
        ret -> structure = new(struct FieldList_);
        FieldList last = ret -> structure;
        va_list ap;
        va_start(ap, cnt);
        for(int i = 0; i < cnt; ++i) {
            last -> name = va_arg(ap, char*);
            last -> type = va_arg(ap, Type);
            ret -> size += last -> type -> size;
            if(i != cnt - 1) {
                last -> next = new(struct FieldList_);
                last = last -> next;
            } else {
                last -> next = NULL;
            }
        }
    }
    return ret;
}

Type to_func(CType ret_val, int cnt, ...) {
    Type ret = new(struct Type_);
    ret -> kind = FUNCTION;
    ret -> structure = new(struct FieldList_);
    ret -> structure -> name = "";
    ret -> structure -> type = (Type)ret_val;

    FieldList last = ret -> structure;
    va_list ap;
    va_start(ap, cnt);
    for(int i = 0; i < cnt; ++i) {
        last -> next = new(struct FieldList_);
        last = last -> next;
        last -> name = va_arg(ap, char*);
        last -> type = va_arg(ap, Type);
    }
    last -> next = NULL;
    return ret;
}

static int fieldcmp(FieldList f1, FieldList f2) {
    if(typecmp(f1 -> type, f2 -> type)) {
        return 1;
    }
    if(f1 -> next && f2 -> next) {
        return fieldcmp(f1 -> next, f2 -> next);
    } else {
        return f1 -> next || f2 -> next;
    }
}

Type typecpy(CType t) {
    switch(t -> kind) {
        case BASIC:
            return (Type)t;
        case ARRAY:
            return to_array(typecpy(t -> array.elem), t -> array.size);
        case STRUCTURE:
        case FUNCTION:
            {
                Type ret = new(struct Type_);
                ret -> kind = t -> kind;
                struct FieldList_ senti;
                for(FieldList src = t -> structure, des = &senti;; src = src -> next) {
                    if(!src) {
                        des -> next = NULL;
                        break;
                    }
                    des -> next = new(struct FieldList_);
                    des = des -> next;
                    des -> name = src -> name;
                    des -> type = typecpy(src -> type);
                }
                ret -> structure = senti.next;
                return ret;
            }
        default:
            panic();
    }
    panic();
    return NULL;
}
int typecmp(CType t1, CType t2) {
    if(t1 && t2) {
        if(t1 == t2) {
            return 0;
        }
        if(t1 -> kind != t2 -> kind) {
            return 1;
        }
        switch(t1 -> kind) {
            case BASIC:
                return t1 -> basic != t2 -> basic;
            case ARRAY:
                return typecmp(t1 -> array.elem, t2 -> array.elem);
            case STRUCTURE:
            case FUNCTION:
                return fieldcmp(t1 -> structure, t2 -> structure);
            default:
                panic();
                return 1;
        }
    } else {
        return (t1 != NULL) || (t2 != NULL);
    }
}

Type type_check(CType lhs, CType rhs, CType ret, int lineno, semantic_errors err, ...) {
    va_list ap;
    va_start(ap, err);

    if(lhs && rhs) {
        if(!typecmp(lhs, rhs)) {
            if(ret) return (Type)ret;
            else return (Type)lhs;
        }
        vsemantic_error(lineno, err, ap);
    }
    return NULL;
}

void static inline type_print_real(Type t, int indent) {
    switch(t -> kind) {
        case BASIC:
            if(t -> basic == T_INT) {
                printf("%*sINT", indent, "");
            } else if(t -> basic == T_FLOAT){
                printf("%*sFLOAT", indent, "");
            } else {
                panic();
            }
            break;
        case ARRAY:
            type_print_real(t -> array.elem, indent);
            printf("[%d]", t -> array.size);
            break;
        case STRUCTURE_DEF:
            t = t -> variable;
        case STRUCTURE:
            printf("struct {\n");
            for(FieldList cur = t -> structure; cur; cur = cur -> next) {
                type_print_real(cur -> type, indent + 4);
                printf(" %s",cur -> name);
                putchar('\n');
            }
            printf("}");
            break;
        case FUNCTION:
            type_print_real(t -> structure -> type, indent);
            printf(" func(");
            for(FieldList cur = t -> structure -> next; cur; cur = cur -> next) {
                type_print_real(cur -> type, 0);
                printf(" %s", cur -> name);
                if(cur -> next) {
                    printf(", ");
                }
            }
            printf(")");
            break;
        default:
            panic();
            break;
    }
}

void type_print(Type t) {
    type_print_real(t, 0);
    putchar('\n');
}

void type_clear(Type t);

void field_clear(FieldList f) {
    if(f) {
        type_clear(f -> type);
        field_clear(remove_access(f, next));
    }
}

void type_clear(Type t) {
    switch(t -> kind) {
        case ARRAY:
            type_clear(t -> array.elem);
            break;
        case STRUCTURE_DEF:
            if(!t -> is_dec)
                field_clear(remove_access(t -> variable, structure));
            break;
        case FUNCTION:
            field_clear(t -> structure);
            break;
        case STRUCTURE:
        case BASIC:
        case OFFSET_COMP:
        case OFFSET_BASIC:
            return;
        default:
            panic();
            return;
    }
    free(t);
}
