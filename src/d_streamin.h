#ifndef D_STREAM_IN_H
#define D_STREAM_IN_H

#include "object.h"

Object streamInNew(Object source);

Object streamInRead(Object stream);

void streamInMark(Object stream);

#endif
