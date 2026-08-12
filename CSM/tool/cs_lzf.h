/*
*
* cs_lzf.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_lsf_h
#define __cs_lsf_h

#include "cs_array.h"

namespace tool
{
  unsigned int
  lzf_compress ( const void *const in_data, unsigned int in_len,
                 void *out_data, unsigned int out_len );

  unsigned int
  lzf_decompress ( const void *const in_data, unsigned int in_len,
                   void *out_data, unsigned int out_len );

  namespace lzf
  {
    void compress ( const void *const in_data, unsigned int in_len,
                    array<byte>& out);
    bool decompress ( const void *const in_data, unsigned int in_len,
                      array<byte>& out);

  }

};

#endif
