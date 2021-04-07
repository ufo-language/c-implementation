#ifndef D_REAL_H
#define D_REAL_H

#include <stdio.h>

#include "defines.h"
#include "object.h"

Object realNew(float i);
bool realEqual(Object real, Object other);
float realGet(Object real);
void realShow(Object real, FILE* stream);

#endif
