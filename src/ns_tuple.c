#include <stdio.h>

#include "d_tuple.h"
#include "d_binding.h"
#include "d_hash.h"
#include "d_int.h"
#include "d_list.h"
#include "d_prim.h"
#include "d_symbol.h"
#include "delegate.h"
#include "e_ident.h"
#include "globals.h"
#include "namespace.h"
#include "object.h"

Object tuple_count(Thread* thd, Object args);
Object tuple_get(Thread* thd, Object args);
Object tuple_set(Thread* thd, Object args);

static Object param_Tuple;
static Object param_TupleInt;

/*------------------------------------------------------------------*/
Object tuple_defineAll(Object env) {
  param_Tuple = primBuildTypeList(1, D_Tuple);
  param_TupleInt = primBuildTypeList(2, D_Tuple, D_Int);
  Object ns = hashNew();
  nsAddPrim(ns, "count", tuple_count);
  nsAddPrim(ns, "get", tuple_get);
  Object binding = bindingNew(identNew("tuple"), ns);
  return listNew(binding, env);
}

/*------------------------------------------------------------------*/
Object tuple_count(Thread* thd, Object args) {
  (void)thd;
  primCheckArgs(param_Tuple, args, thd);
  Object tuple = listGetFirst(args);
  Word nElems = tupleCount(tuple);
  return intNew(nElems);
}

/*------------------------------------------------------------------*/
Object tuple_get(Thread* thd, Object args) {
  (void)thd;
  primCheckArgs(param_TupleInt, args, thd);
  Object tuple = listGetFirst(args);
  Object indexInt = listGetSecond(args);
  Word index = intGet(indexInt);
  return tupleGet(tuple, index, thd);
}