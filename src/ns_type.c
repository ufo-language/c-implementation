#include <stdio.h>

#include "d_binding.h"
#include "d_hash.h"
#include "d_int.h"
#include "d_list.h"
#include "d_prim.h"
#include "d_set.h"
#include "d_symbol.h"
#include "delegate.h"
#include "e_ident.h"
#include "globals.h"
#include "namespace.h"
#include "object.h"

Object type_check(Thread* thd, Object args);
Object type_hasType(Thread* thd, Object args);
Object type_name(Thread* thd, Object args);

static Object param_Any;
static Object param_AnySymbol;

/*------------------------------------------------------------------*/
void type_defineAll(Object env, Thread* thd) {
  char* nsName = "type";
  param_Any = primBuildTypeList(1, D_Null);
  param_AnySymbol = primBuildTypeList(2, D_Null, D_Symbol);
  Object ns = hashNew();
  nsAddPrim(ns, "check", type_check, thd);
  nsAddPrim(ns, "hasType", type_hasType, thd);
  nsAddPrim(ns, "name", type_name, thd);
  hashPut(env, identNew(nsName), ns, thd);
}

/*------------------------------------------------------------------*/
Object type_check(Thread* thd, Object args) {
  Object arg, typeNameSym;
  Object* argAry[] = {&arg, &typeNameSym};
  primCheckArgs(param_AnySymbol, args, argAry, thd);
  ObjType objType = objGetType(arg);
  char* typeName = ObjTypeNames[objType];
  Object typeSym = symbolNew(typeName);
  if (!objEquals(typeNameSym, typeSym, thd)) {
    threadThrowException(thd, "TypeError", "object {}, expected type {}, found type {}",
      listNew(arg, listNew(typeNameSym, listNew(typeSym, EMPTY_LIST))));
  }
  return arg;
}

/*------------------------------------------------------------------*/
Object type_hasType(Thread* thd, Object args) {
  Object arg, typeNameSym;
  Object* argAry[] = {&arg, &typeNameSym};
  primCheckArgs(param_AnySymbol, args, argAry, thd);
  ObjType objType = objGetType(arg);
  char* typeName = ObjTypeNames[objType];
  Object typeSym = symbolNew(typeName);
  return objEquals(typeNameSym, typeSym, thd) ? TRUE : FALSE;
}

/*------------------------------------------------------------------*/
Object type_name(Thread* thd, Object args) {
  Object arg;
  Object* argAry[] = {&arg};
  primCheckArgs(param_Any, args, argAry, thd);
  ObjType objType = objGetType(arg);
  char* typeName = ObjTypeNames[objType];
  Object typeSym = symbolNew(typeName);
  return typeSym;
}
