#ifndef E_LETIN_H
#define E_LETIN_H

#include "object.h"
#include "thread.h"

#define LETIN_BINDINGS_OFS 0
#define LETIN_BODY_OFS 1

Object letInNew(Object bindings, Object body);

Object letInEval(Object letIn, Thread* thd);
void letInFreeVars(Object let, Object freeVarSet);
void letInMark(Object letIn);
void letInShow(Object letIn, FILE* stream);

#endif
