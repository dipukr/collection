/*
*
* cs_base64.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "tool.h"

namespace tool
{
  string base64_encode ( const byte* data, int data_length );
  bool   base64_decode ( const char *data, int data_length, array<byte>& out );
};
