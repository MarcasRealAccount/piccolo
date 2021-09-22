
#ifndef PICCOLO_ENGINE_H
#define PICCOLO_ENGINE_H

#include "package.h"

#include <stdarg.h>
#include <stdbool.h>

struct piccolo_Engine {
    struct piccolo_Package package;

    struct piccolo_Package* currentPackage;
    struct piccolo_Bytecode* bytecode;
    uint8_t* prevIp;
    uint8_t* ip;
    struct piccolo_ValueArray stack;
    bool hadError;

    void (*printError)(const char* format, va_list);
};

void piccolo_initEngine(struct piccolo_Engine* engine, void (*printError)(const char* format, va_list));
void piccolo_freeEngine(struct piccolo_Engine* engine);

bool piccolo_executePackage(struct piccolo_Engine* engine, struct piccolo_Package* package);
bool piccolo_executeBytecode(struct piccolo_Engine* engine, struct piccolo_Bytecode* bytecode);

void piccolo_enginePrintError(struct piccolo_Engine* engine, const char* format, ...);

void piccolo_enginePushStack(struct piccolo_Engine* engine, piccolo_Value value);
piccolo_Value piccolo_enginePopStack(struct piccolo_Engine* engine);

void piccolo_runtimeError(struct piccolo_Engine* engine, const char* format, ...);

#endif