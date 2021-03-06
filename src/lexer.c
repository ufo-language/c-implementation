#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "d_int.h"
#include "d_list.h"
#include "d_string.h"
#include "delegate.h"
#include "lexer.h"
#include "mem.h"
#include "object.h"
#include "syntax.h"
#include "thread.h"

/* There must be a 1:1 correspondence between this and the StateName enum */
char* S_NAMES[] = {
  "I", "Integer", "Point", "Real", "Word", "String", "Operator", "Symbol"
};

/* There must be a 1:1 correspondence between this and the Action enum */
char* A_NAMES[] = {
  "Keep", "Reuse", "Ignore", "DoubleDot", "Err", "ErrReal", "ErrString"
};

/* There must be a 1:1 correspondence between this and the TokenType enum */
char* T_NAMES[] = {
  "None", "Int", "Real", "Bool", "Word", "Ident", "Reserved",
  "String", "Operator", "Special", "Symbol", "EOI"
};

char cDisp(char c) {
  return c < 32 ? '~' : c;
}

Transition* findTransition(Transition** syntax, StateName stateName, char c) {
  Transition* state = syntax[stateName];
#define FIND_DEBUG 0
#if FIND_DEBUG
  printf("State: %s\n", S_NAMES[stateName]);
#endif
  for (int n=0;; n++) {
    Transition* transition = &state[n];
    char from = transition->from;
    char to = transition->to;
#if FIND_DEBUG
    printf("  comparing char '%c' (%d) to range '%c' - '%c'\n", cDisp(c), c, cDisp(from), cDisp(to));
#endif
    if (from == C_ANY) {
      return transition;
    }
    if (c >= from && c <= to) {
      return transition;
    }
  }
  fprintf(stderr, "lexer.findTransition should not get here");
  return NULL;
}

void lexInit(LexerState* lexerState, Transition** syntax, Object inputString) {
  lexerState->syntax = syntax;
  lexerState->inputString = inputString;
  lexerState->inputLen = stringCount(inputString);
  lexerState->pos = 0;
  lexerState->line = 1;
  lexerState->col = 0;
  lexerState->error = false;
}

bool isIn(char* str, char* strAry[]) {
  for (int n=0;; n++) {
    char* strElem = strAry[n];
    if (!strElem) {
      return false;
    }
    if (!strcmp(strElem, str)) {
      return true;
    }
  }
}

void lexError(Thread* thd, LexerState* lexerState, char* message) {
  Object exn = listNew(lexerState->inputString, intNew(lexerState->pos));
  threadThrowException(thd, "LexerError", message, exn);
}

bool lexToken(Thread* thd, LexerState* lexerState, Token* token) {
  StateName stateName = S_I;
  int lexemeIndex = 0;
  bool contin = true;
  while (contin && (lexerState->pos <= lexerState->inputLen)) {
    Object inputString = lexerState->inputString;
    char c = stringGetChar_unsafe(inputString, (Word)lexerState->pos);
    Transition* transition = findTransition(lexerState->syntax, stateName, c);
    stateName = transition->nextState;
    Action action = transition->action;
    TokenType tokenType = transition->tokenType;
    switch (action) {
      case A_KEEP:
        token->lexeme[lexemeIndex++] = c;
        break;
      case A_REUSE:
        lexerState->pos--;
        break;
      case A_IGNORE:
        break;
      case A_DOUBLEDOT:
        lexerState->pos -= 2;
        lexemeIndex--;
        break;
      case A_ERR:
        lexError(thd, lexerState, "lexer error");
      case A_ERR_REAL:
        lexError(thd, lexerState, "digits expected after decimal point");
      case A_ERR_STRING:
        lexError(thd, lexerState, "unterminated string");
    }
    if (tokenType != T_NONE) {
      token->lexeme[lexemeIndex] = '\0';
      contin = false;
      if (tokenType == T_WORD) {
        if (isIn(token->lexeme, RESERVED_WORDS)) {
          tokenType = T_RESERVED;
        }
        else if(isIn(token->lexeme, BOOL_WORDS)) {
          tokenType = T_BOOL;
        }
        else {
          tokenType = T_IDENT;
        }
      }
      token->type = tokenType;
      contin = false;
    }
    lexerState->pos++;
  }
  return token->type != T_EOI;
}
