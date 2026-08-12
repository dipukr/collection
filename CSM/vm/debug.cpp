/*
*
* debug.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
// til debug routines

#include "c-smile.h"
#include "vm.h"
#include "opcodes.h"
#include "scanner.h"
#include "streams.h"

namespace c_smile
{
  /* instruction output formats */
#define FMT_NONE	0
#define FMT_BYTE	1
#define FMT_WORD	2
#define FMT_LIT		3

#ifdef DECODE_TRACE
  typedef struct
  {
    int ot_code; char *ot_name; int ot_fmt;
  }
  OTDEF;
  OTDEF otab [] =
  {
    { OP_BRT,     "BRT",              FMT_WORD  },
    { OP_BRF,     "BRF",              FMT_WORD  },
    { OP_BR,      "BR",               FMT_WORD  },
    { OP_LIT,     "LIT",              FMT_LIT	  },
    { OP_REF,     "REF",              FMT_LIT	  },
    { OP_SET,     "SET",              FMT_LIT	  },
    { OP_AREF     "AREF",             FMT_BYTE  },
    { OP_ASET     "ASET",             FMT_BYTE  },
    { OP_TREF,    "TREF",             FMT_BYTE  },
    { OP_TSET     "TSET",             FMT_BYTE  },
    { OP_MREF     "MREF",             FMT_BYTE  },
    { OP_MSET     "MSET",             FMT_BYTE  },
    { OP_VREF     "VREF",             FMT_NONE  },
    { OP_VSET     "VSET",             FMT_NONE  },
    { OP_CALL     "CALL",             FMT_WORD  },
    { OP_RETURN,  "RETURN",           FMT_NONE  },
    { OP_SEND,    "SEND",             FMT_BYTE  },
    { OP_TSPACE,  "TSPACE",           FMT_BYTE  },
    { OP_NULL,    "NULL",             FMT_NONE  },
    { OP_PUSH,    "PUSH",             FMT_NONE  },
    { OP_NOT,     "NOT",              FMT_NONE  },
    { OP_NEG,     "NEG",              FMT_NONE  },
    { OP_ADD,     "ADD",              FMT_NONE  },
    { OP_SUB,     "SUB",              FMT_NONE  },
    { OP_MUL,     "MUL",              FMT_NONE  },
    { OP_DIV,     "DIV",              FMT_NONE  },
    { OP_REM,     "REM",              FMT_NONE  },
    { OP_SHL,     "SHL",              FMT_NONE  },
    { OP_SHR,     "SHR",              FMT_NONE  },
    { OP_BAND     "BAND",             FMT_NONE  },
    { OP_BOR,     "BOR",              FMT_NONE  },
    { OP_BNOT,    "BNOT",             FMT_NONE  },
    { OP_LT,      "LT",               FMT_NONE  },
    { OP_LE,      "LE",               FMT_NONE  },
    { OP_EQ,      "EQ",               FMT_NONE  },
    { OP_NE,      "NE",               FMT_NONE  },
    { OP_GE,      "GE",               FMT_NONE  },
    { OP_GT,      "GT",               FMT_NONE  },
    { OP_INC,     "INC",              FMT_NONE  },
    { OP_DEC,     "DEC",              FMT_NONE  },
    { OP_DUP2,    "DUP2",             FMT_NONE  },
    { OP_NEW,     "NEW",              FMT_BYTE  },
    { OP_LINE,    "LINE",             FMT_WORD  },
    { OP_EH_PUSH, "EH_PUSH",          FMT_WORD  },
    { OP_EH_POP,  "EH_POP",           FMT_WORD  },
    { OP_THROW,   "THROW",            FMT_NONE  },
    { OP_PMREF,   "PMREF",            FMT_NONE  },
    { OP_PMSET,   "PMSET",            FMT_NONE  },
    { OP_UNDEFINED, "UNDEFINED",      FMT_NONE  },
    { OP_ARGUMENTS, "ARGUMENT_COUNT", FMT_NONE  },
    { OP_ARGUMENT,  "ARGUMENT",       FMT_NONE  },
    { OP_COPY,      "COPY",           FMT_NONE  },
    { OP_POP,       "POP",            FMT_NONE  },
    { OP_DUP,       "DUP",            FMT_NONE  },
    { OP_MAKEREF,   "MAKEREF",        FMT_NONE  },
    {	OP_ENTER,     "ENTER",          FMT_NONE  },
    { OP_LEAVE,     "LEAVE",          FMT_NONE  },
    { OP_TSTORE,    "TSTORE",         FMT_NONE  },
    { OP_TRESTORE,  "TRESTORE",       FMT_NONE  }
  };


  // decode_procedure - decode the instructions in a code object
  void
    VM::decode_procedure ( CODE *code )
  {
    int len, lc, n;
    len = code->bytecode_size ();
    for ( lc = 0; lc < len; lc += n )
      n = decode_instruction ( code, lc );
  }

  // decode_instruction - decode a single bytecode instruction
  int
    VM::decode_instruction ( CODE *code, int lc )
  {
    char buf [ 1024 ];
    unsigned char *cp;
    OTDEF *op;
    int n = 1;

    /* get bytecode pointer for this instruction */
    cp = (unsigned char *) code->bytecode () + lc;

    /* show the address and opcode */

    CLASS *klass = code->klass ();
    assert ( klass );
    PACKAGE *package = klass->get_package ();
    assert ( package );

    serr->put ( (const char *) klass->full_name () );
    serr->put ( "::" );

    snprintf ( buf, 1024, "%s %04x %02x ", voc [ code->name () ], lc, *cp );
    serr->put ( buf );

    /* display the operands */
    for ( op = otab; op->ot_name; ++op )
      if ( *cp == op->ot_code  )
      {
        switch ( op->ot_fmt  )
        {
        case FMT_NONE:
          snprintf ( buf, 1024, "      %s\n", op->ot_name );
          serr->put ( buf );
          break;
        case FMT_BYTE:
          snprintf ( buf, 1024, "%02x    %s %02x\n", cp [ 1 ], op->ot_name,
                                                     cp [ 1 ] );
          serr->put ( buf );
          n += 1;
          break;
        case FMT_WORD:
          snprintf ( buf, 1024, "%02x %02x %s %02x%02x\n", cp [ 1 ], cp [ 2 ],
                                                           op->ot_name,
                                                           cp [ 2 ], cp [ 1 ] );
          serr->put ( buf );
          n += 2;
          break;
        case FMT_LIT:
          snprintf ( buf, 1024, "%02x    %s %02x ;", cp [ 1 ], op->ot_name,
                                                     cp [ 1 ] );
          serr->put ( buf );
          string s = ( *package->literals ) [ cp [ 1 ]];
          serr->put ( (const char *) s );
          serr->put ( "\n" );
          n += 2;
          break;
        }
        return ( n );
      }

    /* unknown opcode */
    snprintf ( buf, 1024, "      <unknown>\n" );
    serr->put ( buf );
    return 1;
  }

#endif

};
