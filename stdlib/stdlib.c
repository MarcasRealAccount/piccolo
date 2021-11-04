
#include "stdlib.h"
#include "../embedding.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static piccolo_Value printNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    for(int i = 0; i < argc; i++) {
        piccolo_printValue(args[i]);
        printf(" ");
    }
    printf("\n");
    return PICCOLO_NIL_VAL();
}

static piccolo_Value inputNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    if(argc != 0) {
        piccolo_runtimeError(engine, "Wrong argument count.");
        return PICCOLO_NIL_VAL();
    }

    size_t lenMax = 64;
    size_t len = 0;
    char* line = malloc(lenMax);
    char* curr = line;
    for(;;) {
        if(line == NULL) {
            piccolo_runtimeError(engine, "Could not read input.");
            return PICCOLO_NIL_VAL();
        }
        int c = fgetc(stdin);
        if(c == EOF || c == '\n') {
            break;
        }
        *curr = c;
        curr++;
        len++;
        if(len >= lenMax) {
            lenMax *= 2;
            line = realloc(line, lenMax);
        }
    }
    *curr = '\0';
    return PICCOLO_OBJ_VAL(piccolo_takeString(engine, line));
}

void piccolo_addIOLib(struct piccolo_Engine* engine) {
    struct piccolo_Package* io = piccolo_createPackage(engine);
    io->packageName = "io";
    piccolo_defineGlobal(engine, io, "print", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, printNative)));
    piccolo_defineGlobal(engine, io, "input", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, inputNative)));
}

static piccolo_Value clockNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    double time = (double)clock() / (double)CLOCKS_PER_SEC;
    if(argc != 0) {
        piccolo_runtimeError(engine, "Wrong argument count.");
    }
    return PICCOLO_NUM_VAL(time);
}

static piccolo_Value sleepNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    if(argc != 1) {
        piccolo_runtimeError(engine, "Wrong argument count.");
    } else {
        if(!PICCOLO_IS_NUM(args[0])) {
            piccolo_runtimeError(engine, "Sleep time must be a number.");
        } else {
            double time = PICCOLO_AS_NUM(args[0]);
            clock_t startTime = clock();
            while(clock() - startTime < time * CLOCKS_PER_SEC) {}
        }
    }
    return PICCOLO_NIL_VAL();
}

void piccolo_addTimeLib(struct piccolo_Engine* engine) {
    struct piccolo_Package* time = piccolo_createPackage(engine);
    time->packageName = "time";
    piccolo_defineGlobal(engine, time, "clock", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, clockNative)));
    piccolo_defineGlobal(engine, time, "sleep", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, sleepNative)));
}

#include "../debug/disassembler.h"
static piccolo_Value disassembleFunctionNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    if(argc != 1) {
        piccolo_runtimeError(engine, "Wrong argument count.");
    } else {
        piccolo_Value val = args[0];
        if(!PICCOLO_IS_OBJ(val) || PICCOLO_AS_OBJ(val)->type != PICCOLO_OBJ_CLOSURE) {
            piccolo_runtimeError(engine, "Cannot dissasemble %s.", piccolo_getTypeName(val));
        } else {
            struct piccolo_ObjClosure* function = PICCOLO_AS_OBJ(val);
            piccolo_disassembleBytecode(&function->prototype->bytecode);
        }
    }
    return PICCOLO_NIL_VAL();
}

static int assertions = 0;
static int assertionsMet = 0;

static piccolo_Value assertNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    if(argc != 1) {
        piccolo_runtimeError(engine, "Wrong argument count.");
    } else {
        piccolo_Value val = args[0];
        if(!PICCOLO_IS_BOOL(val)) {
            piccolo_runtimeError(engine, "Expected assertion to be a boolean.");
        } else {
            assertions++;
            bool assertion = PICCOLO_AS_BOOL(val);
            if(assertion) {
                printf("\x1b[32m[OK]\x1b[0m ASSERTION MET\n");
                assertionsMet++;
            } else {
                printf("\x1b[31m[ERROR]\x1b[0m ASSERTION FAILED\n");
            }
        }
    }
    return PICCOLO_NIL_VAL();
}

static piccolo_Value printAssertionResultsNative(struct piccolo_Engine* engine, int argc, struct piccolo_Value* args) {
    if(argc != 0) {
        piccolo_runtimeError(engine, "Wrong argument count.");
    } else {
        if(assertionsMet == assertions) {
            printf("\x1b[32m%d / %d ASSERTIONS MET! ALL OK\x1b[0m\n", assertionsMet, assertions);
        } else {
            printf("\x1b[31m%d / %d ASSERTIONS MET.\x1b[0m\n", assertionsMet, assertions);
        }
    }
    return PICCOLO_NIL_VAL();
}

void piccolo_addDebugLib(struct piccolo_Engine* engine) {
    struct piccolo_Package* debug = piccolo_createPackage(engine);
    debug->packageName = "debug";
    piccolo_defineGlobal(engine, debug, "disassemble", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, disassembleFunctionNative)));
    piccolo_defineGlobal(engine, debug, "assert", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, assertNative)));
    piccolo_defineGlobal(engine, debug, "printAssertionResults", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, printAssertionResultsNative)));
}
