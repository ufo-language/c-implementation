#ifndef E_LET_H
#define E_LET_H

#include "object.h"
#include "thread.h"

#define LET_BINDINGS_OFS 0

Object letNew(Object bindings);

Object letEval(Object let, Thread* thd);
void letFreeVars(Object let, Object freeVarSet);
void letShow(Object let, FILE* stream);

#endif
