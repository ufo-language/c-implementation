#ifndef NAMESPACE_H
#define NAMESPACE_H

#include "d_prim.h"
#include "object.h"

void nsAddPrim(Object hash, char* name, PrimFunc func, Thread* thd);
Object nsDefineAll(Thread* thd);

#endif
