#ifndef D_BINDING_H
#define D_BINDING_H

#include <stdio.h>

#include "defines.h"
#include "object.h"
#include "thread.h"

#define BND_LHS_OFS 0
#define BND_RHS_OFS 1
#define BND_OBJ_SIZE 2

Object bindingNew(Object lhs, Object rhs);

void bindingDel(Object binding);
bool bindingEquals(Object binding, Object other, Thread* thd);
Object bindingEval(Object binding, Thread* thd);
void bindingFreeVars(Object binding, Object freeVarSet, Thread* thd);
Object bindingGetLhs(Object binding);
Object bindingGetRhs(Object binding);
void bindingMark(Object binding);
Object bindingMatch(Object binding, Object other, Object bindingList, Thread* thd);
void bindingSetRhs(Object binding, Object rhs);
void bindingShow(Object binding, FILE* stream);

#endif
