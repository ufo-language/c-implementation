#ifndef E_THROW_H
#define E_THROW_H

#include "object.h"
#include "thread.h"

#define THR_PAYLOAD_OFS 0
#define THR_OBJ_SIZE 1

extern Object throwPayload;

Object throwNew(Object expr);

Object throwEval(Object thrw, Thread* thd);
void throwFreeVars(Object thrw, Object freeVarSet, Thread* thd);
void throwMark(Object thrw);
void throwShow(Object thrw, FILE* stream);

#endif
