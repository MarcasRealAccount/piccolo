
#include "gc.h"

static void markValue(piccolo_Value value);

static void markObj(struct piccolo_Obj* obj) {
    if(obj == NULL)
        return;
    switch(obj->type) {
        case PICCOLO_OBJ_ARRAY: {
            struct piccolo_ObjArray* array = (struct piccolo_ObjArray*)obj;
            for(int i = 0; i < array->array.count; i++)
                markValue(array->array.values[i]);
            break;
        }
        case PICCOLO_OBJ_FUNC: {
            struct piccolo_ObjFunction* func = (struct piccolo_ObjFunction*)obj;
            for(int i = 0; i < func->bytecode.constants.count; i++)
                markValue(func->bytecode.constants.values[i]);
            break;
        }
        case PICCOLO_OBJ_UPVAL: {
            struct piccolo_ObjUpval* upval = (struct piccolo_ObjUpval*)obj;
            markValue(*upval->valPtr);
            break;
        }
        case PICCOLO_OBJ_CLOSURE: {
            struct piccolo_ObjClosure* closure = (struct piccolo_ObjClosure*)obj;
            for(int i = 0; i < closure->upvalCnt; i++)
                markObj(closure->upvals[i]);
            break;
        }
    }
    obj->marked = true;
}

static void markValue(piccolo_Value value) {
    if(PICCOLO_IS_OBJ(value)) {
        markObj(PICCOLO_AS_OBJ(value));
    }
}

static void markPackage(struct piccolo_Package* package) {
    for(int i = 0; i < package->bytecode.constants.count; i++)
        markValue(package->bytecode.constants.values[i]);
    for(int i = 0; i < package->globals.count; i++)
        markValue(package->globals.values[i]);
}

static void markRoots(struct piccolo_Engine* engine) {
    for(piccolo_Value* iter = engine->stack; iter != engine->stackTop; iter++)
        markValue(*iter);
    for(int i = 0; i < engine->currFrame; i++) {
        for(int j = 0; j < sizeof(engine->frames[i].varStack) / sizeof(piccolo_Value); j++)
            markValue(engine->frames[i].varStack[j]);
        markObj(engine->frames[i].closure);
    }
    markPackage(&engine->package);
}

void piccolo_collectGarbage(struct piccolo_Engine* engine) {
    struct piccolo_Obj* currObj = engine->objs;
    while(currObj != NULL) {
        currObj->marked = false;
        currObj = currObj->next;
    }
    markRoots(engine);

    struct piccolo_Obj* newObjs = NULL;
    currObj = engine->objs;
    while(currObj != NULL) {
        struct piccolo_Obj* curr = currObj;
        currObj = currObj->next;
        if(curr->marked) {
            curr->next = newObjs;
            newObjs = curr;
        } else {
            piccolo_freeObj(engine, curr);
        }
    }
    engine->objs = newObjs;

}