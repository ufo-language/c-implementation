#include <stdlib.h>
#include <stdio.h>

#include "d_array.h"
#include "d_binding.h"
#include "d_bool.h"
#include "d_closure.h"
#include "d_exn.h"
#include "d_hash.h"
#include "d_int.h"
#include "d_method.h"
#include "d_nothing.h"
#include "d_list.h"
#include "d_prim.h"
#include "d_queue.h"
#include "d_real.h"
#include "d_seq.h"
#include "d_set.h"
#include "d_streamin.h"
#include "d_streamout.h"
#include "d_string.h"
#include "d_stringbuffer.h"
#include "d_symbol.h"
#include "d_tuple.h"
#include "delegate.h"
#include "e_abstr.h"
#include "e_app.h"
#include "e_binop.h"
#include "e_do.h"
#include "e_ident.h"
#include "e_if.h"
#include "e_let.h"
#include "e_letin.h"
#include "e_letrec.h"
#include "e_quote.h"
#include "e_throw.h"
#include "gc.h"
#include "globals.h"
#include "object.h"
#include "trampoline.h"
#include "vmem.h"

/*------------------------------------------------------------------*/
bool objBoolValue(Object obj) {
  ObjType objType = objGetType(obj);
  switch (objType) {
    case D_Array:
      return arrayCount(obj) > 0;
    case D_Bool:
      return boolGet(obj);
    case D_Hash:
      return hashCount(obj) > 0;
    case D_Int:
      return intGet(obj) != 0;
    case D_Nothing:
      return false;
    case D_List:
      return obj.a != EMPTY_LIST.a;
    case D_Queue:
      return queueCount(obj) > 0;
    case D_Real:
      return realGet(obj) != 0;
    case D_Set:
      return setCount(obj) != 0;
    case D_String:
      return stringCount(obj) > 0;
    case D_StringBuffer:
      return stringBufferCount(obj) > 0;
    case D_Tuple:
      return tupleCount(obj) > 0;
    default:
      /* E_Ident, E_If, etc. */
      return true;
  }
}

/*------------------------------------------------------------------*/
Object objCopy(Object obj) {
  RawBlock blk = objToRawBlock(obj);
  Word nWords = memGetSize(blk) - OBJ_OVERHEAD;
  ObjType objType = objGetType(obj);
  Object objCopy = objAlloc(objType, nWords);
  for (int n=0; n<nWords; n++) {
    objSetData(objCopy, n, objGetData(obj, n));
  }
  return objCopy;
}

/*------------------------------------------------------------------*/
void objDisp(Object obj, FILE* stream) {
  switch (objGetType(obj)) {
    case D_String:
      stringDisp(obj, stream);
      break;
    default:
      objShow(obj, stream);
  }
}

/*------------------------------------------------------------------*/
bool objEquals(Object obj1, Object obj2, Thread* thd) {
  if (obj1.a == obj2.a) {
    return true;
  }
  ObjType obj1Type = objGetType(obj1);
  ObjType obj2Type = objGetType(obj2);
  if (obj1Type != obj2Type) {
    return false;
  }
  switch (obj1Type) {
    case D_Array:
      return arrayEquals(obj1, obj2, thd);
    case D_Binding:
      return bindingEquals(obj1, obj2, thd);
    case D_Bool:
      return boolEquals(obj1, obj2);
    case D_Exn:
      return exnEquals(obj1, obj2, thd);
    case D_Hash:
      return hashEquals(obj1, obj2, thd);
    case D_Int:
      return intEquals(obj1, obj2, thd);
    case D_List:
      return listEquals(obj1, obj2, thd);
    case D_Queue:
      return queueEquals(obj1, obj2, thd);
    case D_Real:
      return realEquals(obj1, obj2);
    case D_Seq:
      return seqEquals(obj1, obj2, thd);
    case D_Set:
      return setEquals(obj1, obj2, thd);
    case D_String:
      return stringEquals(obj1, obj2);
    case D_Symbol:
      return symbolEquals(obj1, obj2);
    case D_Tuple:
      return tupleEquals(obj1, obj2, thd);
    case E_Ident:
      return identEquals(obj1, obj2);
    default:
      printf("objEquals not implemented for type %s\n", ObjTypeNames[objGetType(obj1)]);
      printf("  returning default false\n");
  }
  return false;
}

/*------------------------------------------------------------------*/
Object objEval(Object obj, Thread* thd) {
  bool contin = true;
  Object threadRestoreEnv = nullObj;
  while (contin) {
    ObjType objType = objGetType(obj);
    switch (objType) {
      case D_Array:
        obj = arrayEval(obj, thd);
        break;
      case D_Binding:
        obj =  bindingEval(obj, thd);
        break;
      case D_Exn:
        obj = exnEval(obj, thd);
        break;
      case D_Hash:
        obj = hashEval(obj, thd);
        break;
      case D_List:
        obj = listEval(obj, thd);
        break;
      case D_Queue:
        obj = queueEval(obj, thd);
        break;
      case D_Set:
        obj = setEval(obj, thd);
        break;
      case D_Tuple:
        obj = tupleEval(obj, thd);
        break;
      case E_Abstr:
        obj = abstrEval(obj, thd);
        break;
      case E_App:
        obj = appEval(obj, thd);
        break;
      case E_Binop:
        obj = binopEval(obj, thd);
        break;
      case E_DoSeq:
        obj = doEval(obj, thd);
        break;
      case E_Ident:
        obj = identEval(obj, thd);
        break;
      case E_If:
        obj = ifEval(obj, thd);
        break;
      case E_Let:
        obj = letEval(obj, thd);
        break;
      case E_LetIn:
        obj = letInEval(obj, thd);
        break;
      case E_LetRec:
        obj = letRecEval(obj, thd);
        break;
      case E_Quote:
        obj = quoteEval(obj, thd);
        break;
      case E_Throw:
        obj = throwEval(obj, thd);
        break;
      default:
        /* use the object unevaluated */
        ;
    }  /* end switch */
    objType = objGetType(obj);
    if (objType == S_Trampoline) {
      contin = true;
      if (threadRestoreEnv.a == nullObj.a) {
        threadRestoreEnv = threadGetEnv(thd);
      }
      Object env = trampGetEnv(obj);
      threadSetEnv(thd, env);
      obj = trampGetExpr(obj);
    }
    else {
      contin = false;
    }
  }  /* end while */
  if (threadRestoreEnv.a != nullObj.a) {
    threadSetEnv(thd, threadRestoreEnv);
  }
  return obj;
}

/*------------------------------------------------------------------*/
void objFreeVars(Object obj, Object freeVarSet, Thread* thd) {
  switch (objGetType(obj)) {
    case D_Array:
      arrayFreeVars(obj, freeVarSet, thd);
      break;
    case D_Binding:
      bindingFreeVars(obj, freeVarSet, thd);
      break;
    case D_Hash:
      hashFreeVars(obj, freeVarSet, thd);
      break;
    case D_List:
      listFreeVars(obj, freeVarSet, thd);
      break;
    case D_Queue:
      queueFreeVars(obj, freeVarSet, thd);
      break;
    case D_Set:
      setFreeVars(obj, freeVarSet, thd);
      break;
    case D_Tuple:
      tupleFreeVars(obj, freeVarSet,thd);
      break;
    case E_Abstr:
      abstrFreeVars(obj, freeVarSet, thd);
      break;
    case E_App:
      appFreeVars(obj, freeVarSet, thd);
      break;
    case E_Binop:
      binopFreeVars(obj, freeVarSet, thd);
      break;
    case E_DoSeq:
      doFreeVars(obj, freeVarSet, thd);
      break;
    case E_Ident:
      setAddElem(freeVarSet, obj, thd);
      break;
    case E_If:
      ifFreeVars(obj, freeVarSet, thd);
      break;
    case E_Let:
      letFreeVars(obj, freeVarSet, thd);
      break;
    case E_LetIn:
      letInFreeVars(obj, freeVarSet, thd);
      break;
    case E_LetRec:
      letRecFreeVars(obj, freeVarSet, thd);
      break;
    case E_Quote:
      quoteFreeVars(obj, freeVarSet, thd);
      break;
    case E_Throw:
      throwFreeVars(obj, freeVarSet, thd);
      break;
    case S_Trampoline: {
        Object obj1 = {objGetData(obj, 0)};
        objFreeVars(obj1, freeVarSet, thd);
      }
      break;
    default:
      ;
  }
}

/*------------------------------------------------------------------*/
Word objHashCode(Object obj) {
  switch (objGetType(obj)) {
    /* TODO finish these cases */
    /*case D_Nothing:
      nothingHash(stream);
      break;*/
    /*case D_Array:
      arrayHash(obj, stream);
      break;*/
    /*case D_Binding:
      bindingHash(obj, stream);
      break;*/
    /*case D_Bool:
      boolHash(obj, stream);
      break;*/
    /*case D_Exn:
      exnHash(obj, stream);
      break;*/
    case D_Int:
      return intHash(obj);
    /*case D_List:
      listHash(obj, stream);
      break;*/
    /*case D_Real:
      realHash(obj, stream);
      break;*/
    case D_String:
      return stringHash(obj);
    case D_Symbol:
      return symbolHash(obj);
    case D_Tuple:
      return tupleHash(obj);
    case E_Ident:
      return identHash(obj);
    default:
      fprintf(stderr, "WARNING: object type %s not handled in objHashCode, returning object address\n",
        ObjTypeNames[objGetType(obj)]);
  }
  return obj.a;
}

/*------------------------------------------------------------------*/
bool objHasType(Object obj, Object typeSym) {
  if (symbolEquals(SYM_ANY, typeSym)) {
    return true;
  }
  Object objTypeSym = ObjTypeNameSyms[objGetType(obj)];
  return symbolEquals(objTypeSym, typeSym);
}

/*------------------------------------------------------------------*/
/*void objMark_generic(Object obj, Word start, Word count) {
  Word to = start + count;
  for (Word n=start; n<to; n++) {
    Object obj1 = {objGetData(obj, n)};
    objMark(obj1);
  }
}*/

/*------------------------------------------------------------------*/
void objMark(Object obj) {
  if (gcIsMarked(obj)) {
    return;
  }
  gcSetObjMarkedFlag(obj);
  switch (objGetType(obj)) {
    case D_Array:
      arrayMark(obj);
      break;
    case D_Binding:
      bindingMark(obj);
      break;
    case D_Closure:
      closureMark(obj);
      break;
    case D_Exn:
      exnMark(obj);
      break;
    case D_Hash:
      hashMark(obj);
      break;
    case D_List:
      listMark(obj);
      break;
    case D_Method:
      methodMark(obj);
      break;
    case D_Queue:
      queueMark(obj);
      break;
    case D_Seq:
      seqMark(obj);
      break;
    case D_Set:
      setMark(obj);
      break;
    case D_StreamIn:
      streamInMark(obj);
      break;
    case D_StreamOut:
      streamOutMark(obj);
      break;
    case D_StringBuffer:
      stringBufferMark(obj);
      break;
    case D_Tuple:
      tupleMark(obj);
      break;
    case E_Abstr:
      abstrMark(obj);
      break;
    case E_App:
      appMark(obj);
      break;
    case E_Binop:
      binopMark(obj);
      break;
    case E_DoSeq:
      doMark(obj);
      break;
    case E_If:
      ifMark(obj);
      break;
    case E_Let:
      letMark(obj);
      break;
    case E_LetIn:
      letInMark(obj);
      break;
    case E_LetRec:
      letRecMark(obj);
      break;
    case E_Quote:
      quoteMark(obj);
      break;
    case E_Throw:
      throwMark(obj);
      break;
    case S_Trampoline:
      trampMark(obj);
      break;
    case D_Null:
    case D_Bool:
    case D_Int:
    case D_Nothing:
    case D_Prim:
    case D_PrimMacro:
    case D_Real:
    case D_String:
    case D_Symbol:
    case E_Ident:
    case X_Count:
      ;  /* do nothing */
  }
}

/*------------------------------------------------------------------*/
Object objMatch(Object obj, Object other, Object bindingList, Thread* thd) {
  ObjType objType1 = objGetType(obj);
  if (objType1 == E_Ident) {
    return identMatch(obj, other, bindingList);
  }
  ObjType objType2 = objGetType(other);
  if (objType2 == E_Ident) {
    return identMatch(other, obj, bindingList);
  }
  if (objType1 != objType2) {
    return nullObj;
  }
  switch (objType1) {
    case D_Array:
      return arrayMatch(obj, other, bindingList, thd);
    case D_Binding:
      return bindingMatch(obj, other, bindingList, thd);
    case D_List:
      return listMatch(obj, other, bindingList, thd);
    case D_Tuple:
      return tupleMatch(obj, other, bindingList, thd);
    default:
      return objEquals(obj, other, thd) ? bindingList : nullObj;
  }
}

/*------------------------------------------------------------------*/
void objShow(Object obj, FILE* stream) {
  switch (objGetType(obj)) {
    case D_Nothing:
      nothingShow(stream);
      break;
    case D_Array:
      arrayShow(obj, stream);
      break;
    case D_Binding:
      bindingShow(obj, stream);
      break;
    case D_Bool:
      boolShow(obj, stream);
      break;
    case D_Closure:
      closureShow(obj, stream);
      break;
    case D_Exn:
      exnShow(obj, stream);
      break;
    case D_Hash:
      hashShow(obj, stream);
      break;
    case D_Int:
      intShow(obj, stream);
      break;
    case D_List:
      listShow(obj, stream);
      break;
    case D_Method:
      methodShow(obj, stream);
      break;
    case D_Prim:
    case D_PrimMacro:
      primShow(obj, stream);
      break;
    case D_Queue:
      queueShow(obj, stream);
      break;
    case D_Real:
      realShow(obj, stream);
      break;
    case D_Seq:
      seqShow(obj, stream);
      break;
    case D_Set:
      setShow(obj, stream);
      break;
    case D_String:
      stringShow(obj, stream);
      break;
    case D_StringBuffer:
      stringBufferShow(obj, stream);
      break;
    case D_Symbol:
      symbolShow(obj, stream);
      break;
    case D_Tuple:
      tupleShow(obj, stream);
      break;
    case E_Abstr:
      abstrShow(obj, stream);
      break;
    case E_App:
      appShow(obj, stream);
      break;
    case E_Binop:
      binopShow(obj, stream);
      break;
    case E_DoSeq:
      doShow(obj, stream);
      break;
    case E_Ident:
      identShow(obj, stream);
      break;
    case E_If:
      ifShow(obj, stream);
      break;
    case E_Let:
      letShow(obj, stream);
      break;
    case E_LetIn:
      letInShow(obj, stream);
      break;
    case E_LetRec:
      letRecShow(obj, stream);
      break;
    case E_Quote:
      quoteShow(obj, stream);
      break;
    case E_Throw:
      throwShow(obj, stream);
      break;
    case D_Null:
      fprintf(stream, "NULL-OBJECT");
      break;
    case S_Trampoline:
      trampShow(obj, stream);
      break;
    default:
      fprintf(stream, "SHOW:UNHANDLED-OBJECT(%d|%s)@%d", objGetType(obj), ObjTypeNames[objGetType(obj)], obj.a);
  }
}
