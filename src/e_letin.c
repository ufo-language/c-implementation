#include <stdio.h>

#include "d_binding.h"
#include "d_list.h"
#include "d_set.h"
#include "delegate.h"
#include "e_do.h"
#include "e_letin.h"
#include "eval.h"
#include "globals.h"
#include "object.h"
#include "thread.h"

/*------------------------------------------------------------------*/
Object letInEval(Object letIn, Thread* thd) {
  Object savedEnv = threadGetEnv(thd);
  Object bindings = {objGetData(letIn, LETIN_BINDINGS_OFS)};
  while (!listIsEmpty(bindings)) {
    Object binding = listGetFirst(bindings);
    Object var = bindingGetLhs(binding);
    Object valExpr = bindingGetRhs(binding);
    Object bindingVal = eval(valExpr, thd);
    threadEnvBind(thd, var, bindingVal);
    bindings = listGetRest(bindings);
  }
  Object body = {objGetData(letIn, LETIN_BODY_OFS)};
  Object res = eval(body, thd);
  threadSetEnv(thd, savedEnv);
  return res;
}

/*------------------------------------------------------------------*/
void letInFreeVars(Object letIn, Object freeVarSet, Thread* thd) {
  Object lhsVars = setNew();
  Object bindings = {objGetData(letIn, LETIN_BINDINGS_OFS)};
  while (!listIsEmpty(bindings)) {
    Object binding = listGetFirst(bindings);
    Object lhs = bindingGetLhs(binding);
    Object rhs = bindingGetRhs(binding);
    /* separate the vars on the left from the vars on the right */
    objFreeVars(lhs, lhsVars, thd);
    objFreeVars(rhs, freeVarSet, thd);
    bindings = listGetRest(bindings);
  }
  Object body = {objGetData(letIn, LETIN_BODY_OFS)};
  objFreeVars(body, freeVarSet, thd);
  /* remove each var in the lhs set from the rhs set */
  setRemoveSet(freeVarSet, lhsVars, thd);
}

/*------------------------------------------------------------------*/
void letInMark(Object letIn) {
  Object bindings = {objGetData(letIn, LETIN_BINDINGS_OFS)};
  Object body = {objGetData(letIn, LETIN_BODY_OFS)};
  objMark(bindings);
  objMark(body);
}

/*------------------------------------------------------------------*/
Object letInNew(Object bindings, Object body) {
  Object letIn = objAlloc(E_LetIn, LETIN_OBJ_SIZE);
  Object bodySeq = doNew(body);
  objSetData(letIn, LETIN_BINDINGS_OFS, bindings.a);
  objSetData(letIn, LETIN_BODY_OFS, bodySeq.a);
  return letIn;
}

/*------------------------------------------------------------------*/
void letInShow(Object letIn, FILE* stream) {
  Object bindings = {objGetData(letIn, LETIN_BINDINGS_OFS)};
  listShowWith(bindings, "let ", ", ", "", stream);
  fputs(" in ", stream);
  Object body = {objGetData(letIn, LETIN_BODY_OFS)};
  doShowWith("", body, " ", stream);
  fputs("end", stream);
}
