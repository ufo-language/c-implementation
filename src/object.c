#include <stdio.h>

#include "defines.h"
#include "gc.h"
#include "globals.h"
#include "object.h"
#include "vmem.h"

/* There must be a 1-to-1 correspondence with the ObjType enumerations
   in object.h */
char* ObjTypeNames[] = {
  "NULL",
  /* object types */
  "Array",
  "Binding",
  "Boolean",
  "Closure",
  "Excption",
  "Hash",
  "Integer",
  "List",
  "Method",
  "Nothing",
  "Primitive",
  "PrimitiveMacro",
  "Queue",
  "Real",
  "Sequence",
  "Set",
  "StreamIn",
  "StreamOut",
  "String",
  "StringBuffer",
  "Symbol",
  "Tuple",
  /* expressions */
  "Abstraction",
  "Application",
  "Binop",
  "DoSeq",
  "Ident",
  "IfThen",
  "Let",
  "LetIn",
  "LetRec",
  "Quote",
  "Throw",
  /* special */
  "Trampoline"
};

/* Raw block memory:
   0: Sz, 1: Nxt, 2: Data/object

   Object fields start at block offset 2.
   Object offsets:
   0: Type, 1: Payload
*/

Object nullObj = {0};

/* Object functions ------------------------------------------------*/

Object objRawBlockToObj(RawBlock rawBlk) {
  Block blk = memRawBlockToBlock(rawBlk);
  Object obj = {blk.a};
  return obj;
}

Object objBlockToObj(Block blk) {
  Object obj = {blk.a};
  return obj;
}

Block objToBlock(Object obj) {
  Block block = {obj.a};
  return block;
}

RawBlock objToRawBlock(Object obj) {
  Block blk = {obj.a};
  RawBlock rawBlock = memBlockToRawBlock(blk);
  return rawBlock;
}

Object objAlloc(ObjType type, Word nWords) {
  Object obj = objAlloc_unsafe(type, nWords);
  if (!obj.a) {
    fprintf(stderr, "ERROR: Out of usable memory\n");
    exit(1);
  }
  return obj;
}

/* This function is unsafe because it returns the nullObj if there is
   not enough memory to satisfy the allocation request. */
Object objAlloc_unsafe(ObjType type, Word nWords) {
  nWords += OBJ_OVERHEAD;
  for (int n=0; n<2; n++) {
    Object obj = objBlockToObj(memAlloc(nWords));
    if (obj.a) {
      objSetType(obj, type);
      gcReg(obj);
      return obj;
    }
    if (n == 0) {
      gcMarkSweep();
    }
  }
  return nullObj;
}

void objFree(Object obj) {
  memFree(objToBlock(obj));
}

/* An Object's type is the first word of the object. */
ObjType objGetType(Object obj) {
  return vmemGet(obj.a);
}

/* An Object's type is the first word of the object. */
void objSetType(Object obj, ObjType type) {
  vmemSet(obj.a, type);
}

/* gets and sets values in an object's data area */
Word objGetData(Object obj, Word offset) {
  return vmemGet(obj.a + OBJ_OVERHEAD + offset);
}

void objSetData(Object obj, Word offset, Word value) {
  vmemSet(obj.a + OBJ_OVERHEAD + offset, value);
}

Word objIncData(Object obj, Word offset) {
  return vmemInc(obj.a + OBJ_OVERHEAD + offset);
}

Word objIncDataBy(Object obj, Word offset, Word w) {
  return vmemIncBy(obj.a + OBJ_OVERHEAD + offset, w);
}

Word objDecData(Object obj, Word offset) {
  return vmemDec(obj.a + OBJ_OVERHEAD + offset);
}

Word objDecDataBy(Object obj, Word offset, Word w) {
  return vmemDecBy(obj.a + OBJ_OVERHEAD + offset, w);
}
