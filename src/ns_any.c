#include <stdio.h>

#include "d_binding.h"
#include "d_hash.h"
#include "d_int.h"
#include "d_list.h"
#include "d_prim.h"
#include "d_set.h"
#include "d_symbol.h"
#include "delegate.h"
#include "e_app.h"
#include "e_ident.h"
#include "eval.h"
#include "globals.h"
#include "namespace.h"
#include "object.h"

Object any_freeVars(Thread* thd, Object args);
Object any_hashCode(Thread* thd, Object args);
Object any_match(Thread* thd, Object args);
Object any_pry(Thread* thd, Object args);

extern Object NS_ANY;

static Object param_Any;
static Object param_AnyAny;

/*------------------------------------------------------------------*/
void any_defineAll(Object env, Thread* thd) {
  char* nsName = "any";
  param_Any = primBuildTypeList(1, D_Null);
  param_AnyAny = primBuildTypeList(2, D_Null, D_Null);
  Object ns = hashNew();
  nsAddPrim(ns, "freeVars", any_freeVars, thd);
  nsAddPrim(ns, "hashCode", any_hashCode, thd);
  nsAddPrim(ns, "match", any_match, thd);
  nsAddPrim(ns, "pry", any_pry, thd);
  hashPut(env, identNew(nsName), ns, thd);
  NS_ANY = ns;
}

/*------------------------------------------------------------------*/
Object any_freeVars(Thread* thd, Object args) {
  Object arg;
  Object* argAry[] = {&arg};
  primCheckArgs(param_Any, args, argAry, thd);
  Object freeVarSet = setNew();
  objFreeVars(arg, freeVarSet, thd);
  return freeVarSet;
}

/*------------------------------------------------------------------*/
Object any_hashCode(Thread* thd, Object args) {
  Object arg;
  Object* argAry[] = {&arg};
  primCheckArgs(param_Any, args, argAry, thd);
  Word hashCode = objHashCode(arg);
  return intNew(hashCode);
}

/*------------------------------------------------------------------*/
Object any_match(Thread* thd, Object args) {
  Object arg1, arg2;
  Object* argAry[] = {&arg1, &arg2};
  primCheckArgs(param_AnyAny, args, argAry, thd);
  return objMatch(arg1, arg1, EMPTY_LIST, thd);
}

/*------------------------------------------------------------------*/
Object any_pry(Thread* thd, Object args) {
  Object receiver, abstr;
  Object* argAry[] = {&receiver, &abstr};
  primCheckArgs(param_AnyAny, args, argAry, thd);
  Object app = appNew(abstr, listNew(receiver, EMPTY_LIST));
  eval(app, thd);
  return receiver;
}
