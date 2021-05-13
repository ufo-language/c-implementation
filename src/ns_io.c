#include <stdio.h>

#include "d_binding.h"
#include "d_hash.h"
#include "d_list.h"
#include "d_prim.h"
#include "delegate.h"
#include "e_ident.h"
#include "globals.h"
#include "namespace.h"
#include "object.h"

Object io_disp(Thread* thd, Object args);
Object io_displn(Thread* thd, Object args);
Object io_nl(Thread* thd, Object args);
Object io_show(Thread* thd, Object args);
Object io_showln(Thread* thd, Object args);

/*------------------------------------------------------------------*/
void io_defineAll(Object env) {
  char* nsName = "io";
  Object ns = hashNew();
  nsAddPrim(ns, "disp", io_disp);
  nsAddPrim(ns, "displn", io_displn);
  nsAddPrim(ns, "nl", io_nl);
  nsAddPrim(ns, "show", io_show);
  nsAddPrim(ns, "showln", io_showln);
  hashPut(env, identNew(nsName), ns);
}

/*------------------------------------------------------------------*/
static void dispCallback(Object data, Object elem) {
  (void)data;
  objDisp(elem, stdout);
}

/*------------------------------------------------------------------*/
static void showCallback(Object data, Object elem) {
  (void)data;
  objShow(elem, stdout);
}

/*------------------------------------------------------------------*/
Object io_disp(Thread* thd, Object args) {
  (void)thd;
  listEach(args, dispCallback, NOTHING);
  return NOTHING;
}

/*------------------------------------------------------------------*/
Object io_displn(Thread* thd, Object args) {
  (void)thd;
  listEach(args, dispCallback, NOTHING);
  printf("\n");
  return NOTHING;
}

/*------------------------------------------------------------------*/
Object io_nl(Thread* thd, Object args) {
  primCheckArgs(EMPTY_LIST, args, thd);
  printf("\n");
  return NOTHING;
}

/*------------------------------------------------------------------*/
Object io_show(Thread* thd, Object args) {
  (void)thd;
  listEach(args, showCallback, NOTHING);
  return NOTHING;
}

/*------------------------------------------------------------------*/
Object io_showln(Thread* thd, Object args) {
  (void)thd;
  listEach(args, showCallback, NOTHING);
  printf("\n");
  return NOTHING;
}
