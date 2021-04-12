#include <stdio.h>

#include "test.h"

#include "../src/d_array.h"
#include "../src/d_binding.h"
#include "../src/d_int.h"
#include "../src/d_list.h"
#include "../src/delegate.h"
#include "../src/e_ident.h"
#include "../src/globals.h"
#include "../src/mem.h"
#include "../src/object.h"

static void test_arrayNewSize();
static void test_arrayGet();
static void test_arraySet();
static void test_arrayCount();
static void test_arrayEqual();
static void test_arrayMatch();

/* List the unit tests to run here ---------------------------------*/

static TestEntry testEntries[] = {
  {"test_arrayNewSize", test_arrayNewSize},
  {"test_arrayGet", test_arrayGet},
  {"test_arraySet", test_arraySet},
  {"test_arrayCount", test_arrayCount},
  {"test_arrayEqual", test_arrayEqual},
  {"test_arrayMatch", test_arrayMatch},
  {0, 0}
};

/* Before & after --------------------------------------------------*/

static void test_before() {
  memStart();
  globalsSetup();
}

static void test_after() {
  memStop();
}

/* Runs all the listed tests ---------------------------------------*/

void test_array() {
  runTests((char*)__func__, test_before, testEntries, test_after);
}

/* Unit tests ------------------------------------------------------*/

void test_arrayNewSize() {
  Object ary1 = arrayNew(3);
  ASSERT_NE(0, ary1.a);
  EXPECT_EQ(3, arrayCount(ary1));
}

void test_arrayGet() {
  Object ary1 = arrayNew(3);
  ASSERT_NE(0, ary1.a);
  EXPECT_EQ(NOTHING.a, arrayGet(ary1, 0).a);
  EXPECT_EQ(NOTHING.a, arrayGet(ary1, 1).a);
  EXPECT_EQ(NOTHING.a, arrayGet(ary1, 2).a);
}

void test_arraySet() {
  Object ary1 = arrayNew(3);
  ASSERT_NE(0, ary1.a);
  Object obj0 = objAlloc(D_Unknown, 0);
  Object obj1 = objAlloc(D_Unknown, 0);
  Object obj2 = objAlloc(D_Unknown, 0);
  arraySet(ary1, 0, obj0);
  arraySet(ary1, 1, obj1);
  arraySet(ary1, 2, obj2);
  EXPECT_EQ(obj0.a, arrayGet(ary1, 0).a);
  EXPECT_EQ(obj1.a, arrayGet(ary1, 1).a);
  EXPECT_EQ(obj2.a, arrayGet(ary1, 2).a);
}

void test_arrayCount() {
  Object ary0 = arrayNew(0);
  EXPECT_EQ(0, arrayCount(ary0));
  Object ary1 = arrayNew(1);
  EXPECT_EQ(1, arrayCount(ary1));
  Object ary2 = arrayNew(2);
  EXPECT_EQ(2, arrayCount(ary2));
}

void test_arrayEqual() {
  Object ary0a = arrayNew(0);
  Object ary0b = arrayNew(0);
  EXPECT_T(arrayEqual(ary0a, ary0b));

  Object ary1a = arrayNew(1);
  Object ary1b = arrayNew(1);
  EXPECT_T(arrayEqual(ary1a, ary1b));
  EXPECT_F(arrayEqual(ary0a, ary1a));
  EXPECT_F(arrayEqual(ary1a, ary0a));
  
  Object i100a = intNew(100);
  EXPECT_F(arrayEqual(ary1a, i100a));
  arraySet(ary1a, 0, i100a);
  EXPECT_F(arrayEqual(ary1a, ary1b));
  EXPECT_F(arrayEqual(ary1b, ary1a));
  Object i100b = intNew(100);
  arraySet(ary1b, 0, i100b);
  EXPECT_T(arrayEqual(ary1a, ary1b));
}

void test_arrayMatch() {
  Object bindingList = EMPTY_LIST;
  Object ary0a = arrayNew(0);
  Object ary0b = arrayNew(0);
  Object bindingList0 = objMatch(ary0a, ary0b, bindingList);
  EXPECT_EQ(bindingList.a, bindingList0.a);

  Object ary1a = arrayNew(1);
  Object ary1b = arrayNew(1);
  Object bindingList1 = objMatch(ary1a, ary1b, bindingList);
  EXPECT_EQ(bindingList.a, bindingList1.a);

  Object x = identNew("x");
  Object y = identNew("y");
  Object i100 = intNew(100);
  Object i200 = intNew(200);
  Object ary2a = arrayNew(2);
  arraySet(ary2a, 0, x);
  arraySet(ary2a, 1, i100);
  Object ary2b = arrayNew(2);
  arraySet(ary2b, 0, i200);
  arraySet(ary2b, 1, y);
  Object bindingList2 = objMatch(ary2a, ary2b, bindingList);
  EXPECT_NE(bindingList.a, bindingList2.a);
  Object res = listLocate(bindingList2, x);
  EXPECT_T(objEquals(bindingNew(x, i200), res));
  res = listLocate(bindingList2, y);
  EXPECT_T(objEquals(bindingNew(y, i100), res));

  Object ary3 = arrayNew(3);
  Object ary4 = arrayNew(4);
  Object bindingList34 = objMatch(ary3, ary4, bindingList);
  EXPECT_EQ(nullObj.a, bindingList34.a);
  bindingList34 = objMatch(ary4, ary3, bindingList);
  EXPECT_EQ(nullObj.a, bindingList34.a);
}
