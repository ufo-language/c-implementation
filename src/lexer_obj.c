#include <string.h>

#include "d_array.h"
#include "d_bool.h"
#include "d_int.h"
#include "d_list.h"
#include "d_queue.h"
#include "d_real.h"
#include "d_string.h"
#include "d_symbol.h"
#include "e_ident.h"
#include "globals.h"
#include "lexer.h"
#include "lexer_obj.h"
#include "object.h"
#include "syntax.h"
#include "thread.h"

Object lex(Thread* thd, Object string) {
  LexerState lexerState;
  lexInit(&lexerState, syntax, string);
  Token token;
  Object tokenQ = queueNew();
  while (true) {
    bool res = lexToken(thd, &lexerState, &token);
    if (lexerState.error) {
      printf("lexer error\n");
      break;
    }
    Object tokenTypeSym = symbolNew(T_NAMES[token.type]);
    Object lexeme;
    switch(token.type) {
      case T_NONE:
        lexeme = NOTHING;
        break;
      case T_INT: {
          int i = atoi(token.lexeme);
          lexeme = intNew(i);
        }
        break;
      case T_REAL: {
          float f = atof(token.lexeme);
          lexeme = realNew(f);
        }
        break;
      case T_BOOL:
        lexeme = strcmp(token.lexeme, BOOL_TRUE_STRING) ? FALSE : TRUE;
        break;
      case T_OPER:
      case T_IDENT:
        lexeme = identNew(token.lexeme);
        break;
      case T_EOI:
        lexeme = NOTHING;
        break;
      case T_SYMBOL:
        lexeme = symbolNew(token.lexeme);
        break;
      case T_SPECIAL:
      case T_WORD:
      case T_RESERVED:
      case T_STRING:
      default:
        lexeme = stringNew(token.lexeme);
        ;
    }
    /* build a token object (array) from the lexer token information */
    Object tokenAry = arrayNew(2);
    arraySet_unsafe(tokenAry, 0, tokenTypeSym);
    arraySet_unsafe(tokenAry, 1, lexeme);
    queueEnq(tokenQ, tokenAry);
    if (!res) {
      break;
    }
  }
  return tokenQ;
}
