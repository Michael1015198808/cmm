#include "common.h"
#include "table.h"
#include "type.h"

static struct Type_ type_int_ = {
    .kind = BASIC,
    .basic = T_INT
}, type_float_ = {
    .kind = BASIC,
    .basic = T_FLOAT
};

Type type_int = &type_int_;
Type type_float = &type_float_;

Type to_array(Type base, int size) {
    Type ret = new(struct Type_);
    ret -> kind = ARRAY;
    ret -> array.elem = base;
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

Type to_func(Type ret_val, int cnt, ...) {
    Type ret = new(struct Type_);
    ret -> kind = FUNCTION;
    ret -> structure = new(struct FieldList_);
    ret -> structure -> name = "";
    ret -> structure -> type = ret_val;

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

Type get_type(node* cur) {
    if(IS("Specifier")) {
        cur = cur -> siblings[0];
        switch(cur -> cnt) {
            case 2:
                {
                    char buf[100];
                    sprintf(buf, "struct %s", cur -> siblings[1] -> siblings[0] -> val_str);
                    return table_lookup(buf);
                }
            case 5:
                break;
        }
    } else if(IS("TYPE")){
        Type ret = malloc(sizeof(struct Type_));
        ret -> kind = BASIC;
        ret -> basic = cur -> val_int;
        return ret;
    }
    return NULL;
}

int typecmp(Type, Type);
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
int typecmp(Type t1, Type t2) {
    if(t1 -> kind != t2 -> kind) {
        return 1;
    }
    switch(t1 -> kind) {
        case BASIC:
            return t1 -> basic != t2 -> basic;
            break;
        case ARRAY:
            return t1 -> array.size != t2 -> array.size ||
                typecmp(t1 -> array.elem, t2 -> array.elem);
            break;
        case STRUCTURE:
        case FUNCTION:
            return fieldcmp(t1 -> structure, t2 -> structure);
            break;
        case NOTYPE:
            Assert();
            break;
        default:
            Assert();
    }
}

void static inline type_print_real(Type t, int indent) {
    switch(t -> kind) {
        case BASIC:
            if(t -> basic == T_INT) {
                printf("%*sINT", indent, "");
            } else if(t -> basic == T_FLOAT){
                printf("%*sFLOAT", indent, "");
            } else {
                Assert();
            }
            break;
        case ARRAY:
            type_print_real(t -> array.elem, indent);
            printf("[%d]", t -> array.size);
            break;
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
            Assert();
            break;
    }
}
void type_print(Type t) {
    type_print_real(t, 0);
    putchar('\n');
}
