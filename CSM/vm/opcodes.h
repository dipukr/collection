/*
*
* opcodes.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#ifndef __opcodes_h
#define __opcodes_h
// opcodes
#define OP_BRT        0x01  /* branch on true */
#define OP_BRF        0x02  /* branch on false */
#define OP_BR         0x03  /* branch unconditionally */
#define OP_NULL       0x04  /* load top of stack with nil */
#define OP_PUSH       0x05  /* push null onto stack */
#define OP_NOT        0x06  /* logical negate top of stack */
#define OP_NEG        0x07  /* negate top of stack */
#define OP_ADD        0x08  /* add top two stack entries */
#define OP_SUB        0x09  /* subtract top two stack entries */
#define OP_MUL        0x0A  /* multiply top two stack entries */
#define OP_DIV        0x0B  /* divide top two stack entries */
#define OP_REM        0x0C  /* remainder of top two stack entries */
#define OP_BAND       0x0D  /* bitwise and of top two stack entries */
#define OP_BOR        0x0E  /* bitwise or of top two stack entries */
#define OP_XOR        0x0F  /* bitwise xor of top two stack entries */
#define OP_BNOT       0x10  /* bitwise not of top two stack entries */
#define OP_SHL        0x11  /* shift left top two stack entries */
#define OP_SHR        0x12  /* shift right top two stack entries */
#define OP_LT         0x13  /* less than */
#define OP_LE         0x14  /* less than or equal to */
#define OP_EQ         0x15  /* equal to */
#define OP_NE         0x16  /* not equal to */
#define OP_GE         0x17  /* greater than or equal to */
#define OP_GT         0x18  /* greater than */
#define OP_INC        0x19  /* increment */
#define OP_DEC        0x1A  /* decrement */
#define OP_LIT        0x1B  /* load literal */
#define OP_RETURN     0x1C  /* return from interpreter */
#define OP_CALL       0x1D  /* call a function */
#define OP_REF        0x1E  /* load a variable value */
#define OP_SET        0x1F  /* set the value of a variable */
#define OP_VREF       0x20  /* load a vector element */
#define OP_VSET       0x21  /* set a vector element */
#define OP_MREF       0x22  /* load a member variable value */
#define OP_MSET       0x23  /* set a member variable */
#define OP_AREF       0x24  /* load an argument value */
#define OP_ASET       0x25  /* set an argument value */
#define OP_TREF       0x26  /* load a temporary variable value */
#define OP_TSET       0x27  /* set a temporary variable */
#define OP_TSPACE     0x28  /* allocate temporary variable space */
#define OP_SEND       0x29  /* send a message to an object */
#define OP_DUP2       0x2A  /* duplicate top two elements on the stack */
#define OP_NEW        0x2B  /* create a new object */
#define OP_LINE       0x2C  /* line number debug info */
#define OP_EH_PUSH    0x2D  /* push error handler block */
#define OP_EH_POP     0x2E  /* pop error handler block */
#define OP_THROW      0x2F  /* throw error */
#define OP_PMREF      0x30  /* load a member variable value by symbol */
#define OP_PMSET      0x31  /* set a member variable value by symbol */

#define OP_INSTANCEOF 0x32  /*  */
#define OP_UNDEFINED  0x33  /* undefined in JS */

#define OP_ARGUMENTS  0x34  /* argument count */
#define OP_ARGUMENT   0x35  /* argument n */

#define OP_COPY       0x36  /* SP ( 0 ) <- SP ( 1 ) */
#define OP_POP        0x37  /* POP */

#define OP_DUP        0x38  /* PUSH; SP ( 0 ) <- SP ( 1 ) */

#define OP_MAKEREF    0x39  /* OBJECT @ METHOD -> DT_OBJECT_METHOD */

#define OP_ENTER      0x40  /* enter critical section */
#define OP_LEAVE      0x41  /* leave critical section */

#define OP_TSTORE     0x42  /* TREGISTER <- SP ( 0 ) */
#define OP_TRESTORE   0x43  /* SP ( 0 ) <- TREGISTER; TREGISTER.CLEAR ()  */

#endif
